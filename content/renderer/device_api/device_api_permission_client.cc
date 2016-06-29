/*
 * device_api_permission_checker.cc
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */
#include "device_api_permission_client.h"

#include "base/logging.h"
#include "content/common/device_api/device_api_permission_check_messages.h"
#include "third_party/WebKit/public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"

namespace content {

DeviceApiPermissionClient::DeviceApiPermissionClient(RenderFrame* render_frame)
	: RenderFrameObserver(render_frame)
{

}

DeviceApiPermissionClient::~DeviceApiPermissionClient()
{
}

bool DeviceApiPermissionClient::OnMessageReceived(const IPC::Message& message)
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(DeviceApiPermissionClient, message)
    IPC_MESSAGE_HANDLER(DeviceApiPermissionCheckMsg_CheckPermissionResult,
    					OnPermissionChecked)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void DeviceApiPermissionClient::DidFinishLoad()
{
	DLOG(INFO) << "DeviceApiPermissionClient::DidFinishLoad";

	result_map_.clear();
}

void DeviceApiPermissionClient::OnPermissionChecked(int result)
{
	DLOG(INFO) << "DeviceApiPermissionClient::OnPermissionChecked, result=" << result;

	HandlePermissionResponse((blink::PermissionResult)result, true);
}

void DeviceApiPermissionClient::CheckPermission(blink::WebDeviceApiPermissionCheckRequest* request)
{
	DLOG(INFO) << "DeviceApiPermissionClient::CheckPermission";

	request->request_id = origin_;
	request->request_id = request->request_id.append(1, 48+request->api);
	request->request_id = request->request_id.append(1, 48+request->operation);
	request_queue_.push_back(request);

	DLOG(INFO) << ">> request id=" << request->request_id;

	if(request_queue_.size() == 1) {
		HandlePermissionRequest(request);
	}
}

void DeviceApiPermissionClient::HandlePermissionRequest(blink::WebDeviceApiPermissionCheckRequest* request)
{
	std::map<std::string, bool>::iterator it = result_map_.find(request->request_id);

	if (it == result_map_.end()) {
		Send(new DeviceApiPermissionCheckMsg_CheckPermission(
				routing_id(), (int)request->api, (int)request->operation));
	}
	else {
		HandlePermissionResponse(it->second ? blink::PermissionResult::RESULT_OK : blink::PermissionResult::RESULT_DENIED, false);
	}
}

void DeviceApiPermissionClient::HandlePermissionResponse(blink::PermissionResult result, bool fromUI)
{
	DLOG(INFO) << "DeviceApiPermissionClient::HandlePermissionResponse";

	blink::WebDeviceApiPermissionCheckRequest* request = request_queue_.front();

	DLOG(INFO) << ">> request id=" << request->request_id;

	if(fromUI) {
		result_map_[request->request_id] = result==blink::PermissionResult::RESULT_OK?true:false;
	}
	request->callback.Run((blink::PermissionResult)result);

	request->callback.Reset();
	request_queue_.pop_front();

	if(request_queue_.size() >= 1) {
		HandlePermissionRequest(request_queue_.front());
	}
}



} // namespace content



