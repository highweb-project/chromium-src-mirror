/*
 * Messaging.cpp
 *
 *  Created on: 2015. 12. 21.
 *      Author: azureskybox
 */
#include "platform/wtf/build_config.h"
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
#include "services/device/public/interfaces/constants.mojom-blink.h"
#include "services/service_manager/public/cpp/connector.h"

namespace blink {

Messaging::Messaging(LocalFrame& frame)
	: SuspendableObject((ExecutionContext*)frame.GetDocument()),
	 	mIsListening(false)
{
	mClient = DeviceApiPermissionController::From(frame)->client();
	WTF::String origin = frame.GetDocument()->Url().StrippedForUseAsReferrer();
	mClient->SetOrigin(origin.Latin1().data());

	this->SuspendIfNeeded();
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
			MessagingErrorCallback* errorCallback = mMessagingErrorCBMap.at(holder->mRequestID);
			if(errorCallback != nullptr) {
				errorCallback->handleResult((unsigned)Messaging::kPermissionDeniedError);
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
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&messageManager));
	}
	device::mojom::blink::MessageObjectPtr messageObject(device::mojom::blink::MessageObject::New());
	convertBlinkMessageToMojo(message.Get(), messageObject.get());
	messageManager->SendMessage(std::move(messageObject));
}

void Messaging::findMessage(MessageFindOptions findOptions, MessagingSuccessCallback* successCallback, MessagingErrorCallback* errorCallback)
{
	DLOG(INFO) << "Messaging::findMessage";

	bool isPermitted = mPermissionMap[(int)PermissionType::MODIFY] || mPermissionMap[(int)PermissionType::VIEW];
	DLOG(INFO) << ">> find operation permitted=" << isPermitted;

	int requestId = base::RandInt(10000,99999);
	mMessageFindOptionsMap.Set(requestId, findOptions);
	mMessagingSuccessCBMap.Set(requestId, successCallback);
	mMessagingErrorCBMap.Set(requestId, errorCallback);

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
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&messageManager));
	}
	MessageFindOptions findOptions = mMessageFindOptionsMap.at(requestId);
	messageManager->FindMessage(requestId, getMessageFindTarget(findOptions.target()), findOptions.maxItem(), findOptions.condition(),
		ConvertToBaseCallback(WTF::Bind(&Messaging::findMessageResult, WrapPersistent(this))));
}

void Messaging::findMessageResult(
	uint32_t requestID, uint32_t error, WTF::Optional<WTF::Vector<device::mojom::blink::MessageObjectPtr>> results) {
	DLOG(INFO) << "Messaging::findMessageResult, result =" << results.has_value();

	MessagingSuccessCallback* successCB = mMessagingSuccessCBMap.at(requestID);
	MessagingErrorCallback* errorCB = mMessagingErrorCBMap.at(requestID);

	if(error != Messaging::kSuccess || !results.has_value()) {
		errorCB->handleResult((unsigned)error);
		return;
	}
	else {
		HeapVector<Member<MessageObject>> resultToReturn;
		MessageObject* tmpObj = nullptr;
		size_t resultSize = results.value().size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertMojoToScriptMessage(results.value()[i].get());
			resultToReturn.push_back(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mMessageFindOptionsMap.erase(requestID);
	mMessagingSuccessCBMap.erase(requestID);
	mMessagingErrorCBMap.erase(requestID);
}

void Messaging::messageReceivedCallback(uint32_t observerId, device::mojom::blink::MessageObjectPtr message) {
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

		DispatchEvent(event);

		messageManager->AddMessagingListener(mObserverID,
			ConvertToBaseCallback(WTF::Bind(&Messaging::messageReceivedCallback, WrapPersistent(this))));
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

const AtomicString& Messaging::InterfaceName() const
{
    return EventTargetNames::PermissionStatus;
}

ExecutionContext* Messaging::GetExecutionContext() const
{
    return ContextLifecycleObserver::GetExecutionContext();
}

bool Messaging::HasPendingActivity() const
{
	DLOG(INFO) << "Messaging::hasPendingActivity, pending activity=" << mIsListening;

	return mIsListening;
}

void Messaging::Resume()
{
}

void Messaging::Suspend()
{
	if(mIsListening) {
		mIsListening = false;

		if (!messageManager.is_bound()) {
			Platform::Current()->GetConnector()->BindInterface(
				device::mojom::blink::kServiceName, mojo::MakeRequest(&messageManager));
		}
		messageManager->RemoveMessagingListener(mObserverID);
	}
}

void Messaging::ContextDestroyed(ExecutionContext* context)
{
	DLOG(INFO) << "Messaging::stop";

	if(mIsListening) {
		mIsListening = false;

		if (!messageManager.is_bound()) {
			Platform::Current()->GetConnector()->BindInterface(
				device::mojom::blink::kServiceName, mojo::MakeRequest(&messageManager));
		}
		messageManager->RemoveMessagingListener(mObserverID);
	}
}

EventListener* Messaging::onmessagereceived()
{
	DLOG(INFO) << "Messaging::onmessagereceived " << mIsListening;
#if defined(OS_ANDROID)
	if(!mIsListening) {
		mIsListening = true;
		DLOG(INFO) << "Messaging::onmessagereceived set mIsListening " << mIsListening;

		mObserverID = base::RandInt(10000,99999);
		if (!messageManager.is_bound()) {
			Platform::Current()->GetConnector()->BindInterface(
				device::mojom::blink::kServiceName, mojo::MakeRequest(&messageManager));
		}
		messageManager->AddMessagingListener(mObserverID,
			ConvertToBaseCallback(WTF::Bind(&Messaging::messageReceivedCallback, WrapPersistent(this))));
	}

	return this->GetAttributeEventListener(EventTypeNames::messagereceived);
#else
	return nullptr;
#endif
}

void Messaging::setOnmessagereceived(EventListener* listener)
{
	DLOG(INFO) << "Messaging::setOnmessagereceived " << mIsListening << ", " << listener;

#if defined(OS_ANDROID)
	if(!mIsListening) {
		mIsListening = true;

		mObserverID = base::RandInt(10000,99999);
		if (!messageManager.is_bound()) {
			Platform::Current()->GetConnector()->BindInterface(
				device::mojom::blink::kServiceName, mojo::MakeRequest(&messageManager));
		}
		messageManager->AddMessagingListener(mObserverID,
			ConvertToBaseCallback(WTF::Bind(&Messaging::messageReceivedCallback, WrapPersistent(this))));
	}
	else {
		if(listener == nullptr) {
			DLOG(INFO) << "null event listener";
			mIsListening = false;

			if (!messageManager.is_bound()) {
				Platform::Current()->GetConnector()->BindInterface(
					device::mojom::blink::kServiceName, mojo::MakeRequest(&messageManager));
			}
			messageManager->RemoveMessagingListener(mObserverID);
		}
	}

	this->SetAttributeEventListener(EventTypeNames::messagereceived, listener);
#endif
}

void Messaging::convertBlinkMessageToMojo(MessageObject* scriptMessage, device::mojom::blink::MessageObject* mojoObject) {
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

MessageObject* Messaging::convertMojoToScriptMessage(device::mojom::blink::MessageObject* mojoObject) {
	MessageObject* scriptMessage = MessageObject::Create();

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
	visitor->Trace(mMessagingSuccessCBMap);
	visitor->Trace(mMessagingErrorCBMap);
  EventTargetWithInlineData::Trace(visitor);
  SuspendableObject::Trace(visitor);
}

}
