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
#include "public/platform/WebString.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
#include "public/platform/modules/device_messaging/WebDeviceMessageFindOptions.h"
#include "public/platform/modules/device_messaging/WebDeviceMessageObject.h"
#include "public/platform/modules/device_messaging/WebDeviceMessagingParameter.h"
#include "public/platform/modules/device_messaging/WebDeviceMessagingResultListener.h"
#include "modules/device_api/DeviceApiPermissionController.h"
#include "modules/device_messaging/MessageObject.h"
#include "modules/device_messaging/MessagingErrorCallback.h"
#include "modules/device_messaging/MessagingEvent.h"
#include "modules/device_messaging/MessagingSuccessCallback.h"
#include "modules/EventTargetModulesNames.h"

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

	Platform::current()->sendMessage(convertToBlinkMessage(message.get()));
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

	WebDeviceMessagingParameter* parameter = new WebDeviceMessagingParameter(this);
	parameter->mRequestId = requestId;
	MessageFindOptions findOptions = mMessageFindOptionsMap.get(requestId);
	parameter->mFindOptions = new WebDeviceMessageFindOptions(
			getMessageFindTarget(findOptions.target()),
			findOptions.maxItem(),
			WebString(findOptions.condition()));

	Platform::current()->findMessage(parameter);
}

void Messaging::OnMessageFindResult(int requestId, unsigned error, std::vector<WebDeviceMessageObject*> results)
{
	DLOG(INFO) << "Contact::OnContactFindResult, result size=" << results.size();

	MessagingSuccessCallback* successCB = mMessagingSuccessCBMap.get(requestId);
	MessagingErrorCallback* errorCB = mMessagingErrorCBMap.get(requestId);

	if(error != Messaging::SUCCESS) {
		errorCB->handleResult((unsigned)error);
		return;
	}
	else {
		HeapVector<Member<MessageObject>> resultToReturn;
		MessageObject* tmpObj = nullptr;
		size_t resultSize = results.size();
		for(size_t i=0; i<resultSize; i++) {
			tmpObj = convertToScriptMessage(results[i]);
			resultToReturn.append(tmpObj);
		}

		successCB->handleResult(resultToReturn);
	}

	mMessageFindOptionsMap.remove(requestId);
	mMessagingSuccessCBMap.remove(requestId);
	mMessagingErrorCBMap.remove(requestId);
}

void Messaging::OnMessageReceived(int observerId, unsigned error, std::vector<WebDeviceMessageObject*> results)
{
	DLOG(INFO) << "Messaging::OnMessageReceived";

	if(error == 40) {
		MessagingEvent* event = MessagingEvent::create(EventTypeNames::messagereceived);
		event->setID(WTF::String(results[0]->mId.utf8().c_str()));
		event->setType(WTF::String(results[0]->mType.utf8().c_str()));
		event->setTo(WTF::String(results[0]->mTo.utf8().c_str()));
		event->setFrom(WTF::String(results[0]->mFrom.utf8().c_str()));
		event->setTitle(WTF::String(results[0]->mTitle.utf8().c_str()));
		event->setBody(WTF::String(results[0]->mBody.utf8().c_str()));
		event->setDate(WTF::String(results[0]->mDate.utf8().c_str()));

		dispatchEvent(event);
	}
}

WebDeviceMessageObject* Messaging::convertToBlinkMessage(MessageObject* scriptMessage)
{
	WebDeviceMessageObject* blinkMessage = new WebDeviceMessageObject();
	if(scriptMessage->hasId())
		blinkMessage->mId = WebString(scriptMessage->id());
	if(scriptMessage->hasType())
		blinkMessage->mType = WebString(scriptMessage->type());
	if(scriptMessage->hasTo())
		blinkMessage->mTo = WebString(scriptMessage->to());
	if(scriptMessage->hasFrom())
		blinkMessage->mFrom = WebString(scriptMessage->from());
	if(scriptMessage->hasTitle())
		blinkMessage->mTitle = WebString(scriptMessage->title());
	if(scriptMessage->hasBody())
		blinkMessage->mBody = WebString(scriptMessage->body());
	if(scriptMessage->hasDate())
		blinkMessage->mDate = WebString(scriptMessage->date());

	return blinkMessage;
}

MessageObject* Messaging::convertToScriptMessage(WebDeviceMessageObject* blinkMessage)
{
	MessageObject* scriptMessage = MessageObject::create();

	scriptMessage->setId(WTF::String(blinkMessage->mId.utf8().c_str()));
	scriptMessage->setType(WTF::String(blinkMessage->mType.utf8().c_str()));
	scriptMessage->setTo(WTF::String(blinkMessage->mTo.utf8().c_str()));
	scriptMessage->setFrom(WTF::String(blinkMessage->mFrom.utf8().c_str()));
	scriptMessage->setTitle(WTF::String(blinkMessage->mTitle.utf8().c_str()));
	scriptMessage->setBody(WTF::String(blinkMessage->mBody.utf8().c_str()));
	scriptMessage->setDate(WTF::String(blinkMessage->mDate.utf8().c_str()));

	return scriptMessage;
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

		WebDeviceMessagingParameter* parameter = new WebDeviceMessagingParameter(this);
		parameter->mRequestId = mObserverID;

		Platform::current()->removeMessagingListener(parameter);
	}
}

void Messaging::stop()
{
	DLOG(INFO) << "Messaging::stop";

	if(mIsListening) {
		mIsListening = false;

		WebDeviceMessagingParameter* parameter = new WebDeviceMessagingParameter(this);
		parameter->mRequestId = mObserverID;

		Platform::current()->removeMessagingListener(parameter);
	}
}

EventListener* Messaging::onmessagereceived()
{
	DLOG(INFO) << "Messaging::onmessagereceived";

	if(!mIsListening) {
		mIsListening = true;

		mObserverID = base::RandInt(10000,99999);
		WebDeviceMessagingParameter* parameter = new WebDeviceMessagingParameter(this);
		parameter->mRequestId = mObserverID;

		Platform::current()->addMessagingListener(parameter);
	}

	return this->getAttributeEventListener(EventTypeNames::messagereceived);
}

void Messaging::setOnmessagereceived(EventListener* listener)
{
	DLOG(INFO) << "Messaging::setOnmessagereceived";

	if(!mIsListening) {
		mIsListening = true;

		mObserverID = base::RandInt(10000,99999);
		WebDeviceMessagingParameter* parameter = new WebDeviceMessagingParameter(this);
		parameter->mRequestId = mObserverID;

		Platform::current()->addMessagingListener(parameter);
	}
	else {
		if(listener != nullptr) {
			DLOG(INFO) << "null event listener";
			mIsListening = false;

			WebDeviceMessagingParameter* parameter = new WebDeviceMessagingParameter(this);
			parameter->mRequestId = mObserverID;

			Platform::current()->removeMessagingListener(parameter);
		}
	}

	this->setAttributeEventListener(EventTypeNames::messagereceived, listener);
}

DEFINE_TRACE(Messaging) {
	//GarbageCollectedFinalized<Messaging>::trace(visitor);
	//ContextLifecycleObserver::trace(visitor);
	visitor->trace(mMessagingSuccessCBMap);
	visitor->trace(mMessagingErrorCBMap);
    EventTargetWithInlineData::trace(visitor);
    ActiveDOMObject::trace(visitor);   
}

}
