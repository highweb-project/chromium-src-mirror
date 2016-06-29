/*
 * device_api_contact_manager.cc
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#include "../device_messaging/device_api_messaging_manager.h"

#include "base/logging.h"
#include "base/rand_util.h"
#include "content/public/common/service_registry.h"
#include "content/public/renderer/render_thread.h"
#include "third_party/WebKit/public/platform/modules/device_messaging/WebDeviceMessageFindOptions.h"
#include "third_party/WebKit/public/platform/modules/device_messaging/WebDeviceMessageObject.h"
#include "third_party/WebKit/public/platform/modules/device_messaging/WebDeviceMessagingParameter.h"
#include "third_party/WebKit/public/platform/modules/device_messaging/WebDeviceMessagingResultListener.h"

namespace content {

DeviceApiMessagingManager::DeviceApiMessagingManager() {
  if (ServiceRegistry* registry = RenderThread::Get()->GetServiceRegistry()) {
    registry->ConnectToRemoteService(mojo::GetProxy(&monitor_));
  }
}

DeviceApiMessagingManager::~DeviceApiMessagingManager() {
}

void DeviceApiMessagingManager::FindMessage(blink::WebDeviceMessagingParameter* parameter)
{
	DLOG(INFO) << "DeviceApiMessagingManager::FindMessage";

	int request_id = base::RandInt(1000,9999);
	find_request_map_[request_id] = parameter;

	monitor_->FindMessage(
			request_id,
			parameter->mFindOptions->mTarget,
			parameter->mFindOptions->mMaxItem,
			parameter->mFindOptions->mCondition.utf8(),
			base::Bind(&DeviceApiMessagingManager::DidFindMessage, base::Unretained(this)));
}

void DeviceApiMessagingManager::DidFindMessage(int requestID, unsigned error, mojo::Array<device::MessageObjectPtr> results)
{
	DLOG(INFO) << "DeviceApiMessagingManager::DidFindMessage";

	FindRequestMap::iterator it = find_request_map_.find(requestID);
	if (it == find_request_map_.end()) {
		return;
	}
	blink::WebDeviceMessagingParameter* current_request = it->second;

	std::vector<blink::WebDeviceMessageObject*> result_objs;
	result_objs.clear();

	if(error == 40) {
		size_t results_size = results.size();
		for(size_t i=0; i<results_size; i++)
			result_objs.push_back(ConvertToBlinkMessage(results[i].get()));
	}
	current_request->mListener->OnMessageFindResult(current_request->mRequestId, error, result_objs);
}

void DeviceApiMessagingManager::SendMessage(std::unique_ptr<blink::WebDeviceMessageObject> message)
{
	device::MessageObjectPtr parmeterPtr(device::MessageObject::New());
	ConvertToMojoMessage(parmeterPtr.get(), message.get());

	monitor_->SendMessage(std::move(parmeterPtr));
}

void DeviceApiMessagingManager::AddListener(blink::WebDeviceMessagingParameter* parameter)
{
	DLOG(INFO) << "DeviceApiMessagingManager::AddObserver";

	observer_map_[parameter->mRequestId] = parameter;

	monitor_->AddMessagingListener(
			parameter->mRequestId,
			base::Bind(&DeviceApiMessagingManager::DidMessageReceived, base::Unretained(this)));
}

void DeviceApiMessagingManager::DidMessageReceived(int observerID, device::MessageObjectPtr result)
{
	DLOG(INFO) << "DeviceApiMessagingManager::DidMessageReceived";

	ObserverMap::iterator it = observer_map_.find(observerID);
	if (it == observer_map_.end()) {
		return;
	}

	std::vector<blink::WebDeviceMessageObject*> result_objs;
	result_objs.push_back(ConvertToBlinkMessage(result.get()));

	blink::WebDeviceMessagingParameter* current_request = it->second;
	current_request->mListener->OnMessageReceived(observerID, 40, result_objs);

	monitor_->AddMessagingListener(
			observerID,
			base::Bind(&DeviceApiMessagingManager::DidMessageReceived, base::Unretained(this)));
}

void DeviceApiMessagingManager::RemoveListener(blink::WebDeviceMessagingParameter* parameter)
{
	DLOG(INFO) << "DeviceApiMessagingManager::RemoveObserver";

	monitor_->RemoveMessagingListener(
			parameter->mRequestId);
}

blink::WebDeviceMessageObject* DeviceApiMessagingManager::ConvertToBlinkMessage(device::MessageObject* mojo_message)
{
	blink::WebDeviceMessageObject* blinkMessage = new blink::WebDeviceMessageObject(
			blink::WebString::fromUTF8(mojo_message->id.get()),
			blink::WebString::fromUTF8(mojo_message->type.get()),
			blink::WebString::fromUTF8(mojo_message->to.get()),
			blink::WebString::fromUTF8(mojo_message->from.get()),
			blink::WebString::fromUTF8(mojo_message->title.get()),
			blink::WebString::fromUTF8(mojo_message->body.get()),
			blink::WebString::fromUTF8(mojo_message->date.get()));

	return blinkMessage;
}

void DeviceApiMessagingManager::ConvertToMojoMessage(device::MessageObject* mojo_message, blink::WebDeviceMessageObject* blink_message)
{
	mojo_message->id = blink_message->mId.utf8();
	mojo_message->type = blink_message->mType.utf8();
	mojo_message->to = blink_message->mTo.utf8();
	mojo_message->from = blink_message->mFrom.utf8();
	mojo_message->title = blink_message->mTitle.utf8();
	mojo_message->body = blink_message->mBody.utf8();
	mojo_message->date = blink_message->mDate.utf8();
}

}  // namespace content
