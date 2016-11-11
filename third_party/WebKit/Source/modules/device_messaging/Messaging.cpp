/*
 * Messaging.cpp
 *
 *  Created on: 2015. 12. 21.
 *      Author: azureskybox
 */
#include "wtf/build_config.h"
#include "modules/device_messaging/Messaging.h"

#include "base/bind.h"
#include "base/rand_util.h"
#include "core/dom/Document.h"
#include "core/events/Event.h"
#include "public/platform/Platform.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
#include "modules/device_api/DeviceApiPermissionController.h"
#include "modules/device_messaging/MessageObject.h"
#include "modules/device_messaging/MessagingErrorCallback.h"
#include "modules/device_messaging/MessagingEvent.h"
#include "modules/device_messaging/MessagingSuccessCallback.h"
#include "modules/EventTargetModulesNames.h"

#include "platform/mojo/MojoHelper.h"
#include "public/platform/ServiceRegistry.h"

namespace blink {

Messaging::Messaging(LocalFrame& frame)
	: ActiveScriptWrappable(this), ActiveDOMObject(frame.domWindow()->getExecutionContext())
	, mIsListening(false)
{
	mClient = DeviceApiPermissionController::from(frame)->client();
	WTF::String origin = frame.document()->url().strippedForUseAsReferrer();
	mClient->SetOrigin(origin.latin1().data());

	this->suspendIfNeeded();
}

Messaging::~Messaging()
{
	if (messageManager.is_bound()) {
		messageManager.reset();
	}
}

void Messaging::onPermissionChecked(PermissionResult result)
{
	DLOG(INFO) << "Messaging::onPermissionChecked, result=" << result;

	if(mRequestQueue.size()) {
		MessagingRequestHolder* holder = mRequestQueue.front();

		if(result != PermissionResult::RESULT_OK) {
			MessagingErrorCallback* errorCallback = mMessagingErrorCBMap.get(holder->mRequestID);
			if(errorCallback != nullptr) {
				errorCallback->handleResult((unsigned)Messaging::PERMISSION_DENIED_ERROR);
			}

			delete holder;
			mRequestQueue.pop_front();

			return;
		}
		else {
			if(holder->mOperation == MessagingRequestHolder::MessagingOperation::FIND) {
				mPermissionMap[(int)PermissionType::VIEW] = false;
			}
		}

		if(holder->mOperation == MessagingRequestHolder::MessagingOperation::FIND) {
			findMessageInternal(holder->mRequestID);
		}

		delete holder;
		mRequestQueue.pop_front();
	}
}

void Messaging::sendMessage(Member<MessageObject> message)
{
	DLOG(INFO) << "Messaging::sendMessage";

	if (!messageManager.is_bound()) {
		Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&messageManager));
	}
	device::blink::MessageObjectPtr messageObject(device::blink::MessageObject::New());
	convertBlinkMessageToMojo(message.get(), messageObject.get());
	messageManager->SendMessage(std::move(messageObject));
}

void Messaging::findMessage(MessageFindOptions findOptions, MessagingSuccessCallback* successCallback, MessagingErrorCallback* errorCallback)
{
	DLOG(INFO) << "Messaging::findMessage";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY] || mPermissionMap[(int)PermissionType::VIEW];
	DLOG(INFO) << ">> find operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);
	mMessageFindOptionsMap.set(requestId, findOptions);
	mMessagingSuccessCBMap.set(requestId, successCallback);
	mMessagingErrorCBMap.set(requestId, errorCallback);

	if(!isPermitted) {
		MessagingRequestHolder* holder = new MessagingRequestHolder(
				MessagingRequestHolder::MessagingOperation::FIND,
				requestId
		);
		mRequestQueue.push_back(holder);

		if(mClient) {
			mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
					PermissionAPIType::MESSAGING,
					PermissionOptType::VIEW,
					base::Bind(&Messaging::onPermissionChecked, base::Unretained(this))));
		}

		return;
	}

	findMessageInternal(requestId);
}

void Messaging::findMessageInternal(int requestId)
{
	DLOG(INFO) << "Messaging::findMessageInternal";

	if (!messageManager.is_bound()) {
		Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&messageManager));
	}
	MessageFindOptions findOptions = mMessageFindOptionsMap.get(requestId);
	messageManager->FindMessage(requestId, getMessageFindTarget(findOptions.target()), findOptions.maxItem(), findOptions.condition(),
		createBaseCallback(bind<uint32_t, uint32_t, mojo::WTFArray<device::blink::MessageObjectPtr>>(&Messaging::findMessageResult, this)));
}

void Messaging::findMessageResult(uint32_t requestID, uint32_t error, mojo::WTFArray<device::blink::MessageObjectPtr> results) {
	DLOG(INFO) << "Messaging::findMessageResult, result size=" << results.size();

	MessagingSuccessCallback* successCB = mMessagingSuccessCBMap.get(requestID);
	MessagingErrorCallback* errorCB = mMessagingErrorCBMap.get(requestID);

	if(error != Messaging::SUCCESS) {
		errorCB->handleResult((unsigned)error);
		return;
	}
	else {
		HeapVector<Member<MessageObject>> resultToReturn;
		MessageObject* tmpObj = nullptr;
		size_t resultSize = results.size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertMojoToScriptMessage(results[i].get());
			resultToReturn.append(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mMessageFindOptionsMap.remove(requestID);
	mMessagingSuccessCBMap.remove(requestID);
	mMessagingErrorCBMap.remove(requestID);
}

void Messaging::messageReceivedCallback(uint32_t observerId, device::blink::MessageObjectPtr message) {
	DLOG(INFO) << "Messaging::messageReceivedCallback";

	if (message.get() != nullptr && mIsListening) {
		MessagingEvent* event = MessagingEvent::create(EventTypeNames::messagereceived);
		event->setID(message->id);
		event->setType(message->type);
		event->setTo(message->to);
		event->setFrom(message->from);
		event->setTitle(message->title);
		event->setBody(message->body);
		event->setDate(message->date);

		dispatchEvent(event);

		messageManager->AddMessagingListener(mObserverID,
			createBaseCallback(bind<uint32_t, device::blink::MessageObjectPtr>(&Messaging::messageReceivedCallback, this)));
	}
}

unsigned Messaging::getMessageFindTarget(WTF::String targetName)
{
	if(targetName == "from")
		return 0;
	else if(targetName == "body")
		return 1;
	else
		return 99;
}

const AtomicString& Messaging::interfaceName() const
{
    return EventTargetNames::PermissionStatus;
}

ExecutionContext* Messaging::getExecutionContext() const
{
    return ContextLifecycleObserver::getExecutionContext();
}

bool Messaging::hasPendingActivity() const
{
	DLOG(INFO) << "Messaging::hasPendingActivity, pending activity=" << mIsListening;

    return mIsListening;
}

void Messaging::resume()
{
}

void Messaging::suspend()
{
	if(mIsListening) {
		mIsListening = false;

		if (!messageManager.is_bound()) {
			Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&messageManager));
		}
		messageManager->RemoveMessagingListener(mObserverID);
	}
}

void Messaging::stop()
{
	DLOG(INFO) << "Messaging::stop";

	if(mIsListening) {
		mIsListening = false;

		if (!messageManager.is_bound()) {
			Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&messageManager));
		}
		messageManager->RemoveMessagingListener(mObserverID);
	}
}

EventListener* Messaging::onmessagereceived()
{
	DLOG(INFO) << "Messaging::onmessagereceived " << mIsListening;

	if(!mIsListening) {
		mIsListening = true;
		DLOG(INFO) << "Messaging::onmessagereceived set mIsListening " << mIsListening;

		mObserverID = base::RandInt(10000,99999);
		if (!messageManager.is_bound()) {
			Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&messageManager));
		}
		messageManager->AddMessagingListener(mObserverID,
			createBaseCallback(bind<uint32_t, device::blink::MessageObjectPtr>(&Messaging::messageReceivedCallback, this)));
	}

	return this->getAttributeEventListener(EventTypeNames::messagereceived);
}

void Messaging::setOnmessagereceived(EventListener* listener)
{
	DLOG(INFO) << "Messaging::setOnmessagereceived " << mIsListening;

	if(!mIsListening) {
		mIsListening = true;

		mObserverID = base::RandInt(10000,99999);
		if (!messageManager.is_bound()) {
			Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&messageManager));
		}
		messageManager->AddMessagingListener(mObserverID,
			createBaseCallback(bind<uint32_t, device::blink::MessageObjectPtr>(&Messaging::messageReceivedCallback, this)));
	}
	else {
		if(listener != nullptr) {
			DLOG(INFO) << "null event listener";
			mIsListening = false;

			if (!messageManager.is_bound()) {
				Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&messageManager));
			}
			messageManager->RemoveMessagingListener(mObserverID);
		}
	}

	this->setAttributeEventListener(EventTypeNames::messagereceived, listener);
}

void Messaging::convertBlinkMessageToMojo(MessageObject* scriptMessage, device::blink::MessageObject* mojoObject) {
	if (scriptMessage->hasId()) {
		mojoObject->id = scriptMessage->id();
	} else {
		mojoObject->id = WTF::String("");
	}
	if(scriptMessage->hasType()) {
		mojoObject->type = scriptMessage->type();
	} else {
		mojoObject->type = WTF::String("");
	}
	if(scriptMessage->hasTo()) {
		mojoObject->to = scriptMessage->to();
	} else {
		mojoObject->to = WTF::String("");
	}
	if(scriptMessage->hasFrom()) {
		mojoObject->from = scriptMessage->from();
	} else {
		mojoObject->from = WTF::String("");
	}
	if(scriptMessage->hasTitle()) {
		mojoObject->title = scriptMessage->title();
	} else {
		mojoObject->title = WTF::String("");
	}
	if(scriptMessage->hasBody()) {
		mojoObject->body = scriptMessage->body();
	} else {
		mojoObject->body = WTF::String("");
	}
	if(scriptMessage->hasDate()) {
		mojoObject->date = scriptMessage->date();
	} else {
		mojoObject->date = WTF::String("");
	}
}

MessageObject* Messaging::convertMojoToScriptMessage(device::blink::MessageObject* mojoObject) {
	MessageObject* scriptMessage = MessageObject::create();

	scriptMessage->setId(mojoObject->id);
	scriptMessage->setType(mojoObject->type);
	scriptMessage->setTo(mojoObject->to);
	scriptMessage->setFrom(mojoObject->from);
	scriptMessage->setTitle(mojoObject->title);
	scriptMessage->setBody(mojoObject->body);
	scriptMessage->setDate(mojoObject->date);

	return scriptMessage;
}

DEFINE_TRACE(Messaging) {
	visitor->trace(mMessagingSuccessCBMap);
	visitor->trace(mMessagingErrorCBMap);
  EventTargetWithInlineData::trace(visitor);
  ActiveDOMObject::trace(visitor);
}

}
