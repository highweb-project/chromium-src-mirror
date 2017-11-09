#include "device_api_applauncher_client.h"

#include "base/logging.h"
#include "third_party/WebKit/public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
#include "content/public/common/device_api_applauncher_request.h"

namespace content {

DeviceApiApplauncherClient::DeviceApiApplauncherClient(RenderFrame* render_frame)
	: RenderFrameObserver(render_frame)
{
}

DeviceApiApplauncherClient::~DeviceApiApplauncherClient()
{
}

bool DeviceApiApplauncherClient::OnMessageReceived(const IPC::Message& message)
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(DeviceApiApplauncherClient, message)
    IPC_MESSAGE_HANDLER(DeviceApiApplauncherMsg_RequestResult,
    					OnRequestResult)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void DeviceApiApplauncherClient::DidFinishLoad()
{

	DLOG(INFO) << "DeviceApiApplauncherClient::DidFinishLoad";
}

void DeviceApiApplauncherClient::requestFunction(WebDeviceApiApplauncherRequest* request) {
	LOG(INFO) << "DeviceApiApplauncherClient::InternalGetAppList " << request << ", " << request->functionCode;
	requestQueue.push_back(request);
	DeviceApiApplauncherRequestMessage message;
	message.functionCode = request->functionCode;
	message.appId = request->appId;
	Send(new DeviceApiApplauncherMsg_RequestFunction(routing_id(), message));
}

void DeviceApiApplauncherClient::OnRequestResult(DeviceApiApplauncherResultMessage message) {
	LOG(INFO) << "DeviceAPiApplauncherCLient::OnRequestResult " << message.resultCode << ", " << message.functionCode;
	if (requestQueue.empty()) {
		LOG(INFO) << "requestQueue is empty!!";
	}
	WebDeviceApiApplauncherRequest* request = requestQueue.front();
	requestQueue.pop_front();

	WebDeviceApiApplauncherRequestResult result;
	result.resultCode = message.resultCode;
	result.functionCode = message.functionCode;
	if (message.applist.size() > 0) {
		for(DeviceApiApplauncherApplicationInfo info : message.applist) {
			WebAppLauncher_ApplicationInfo appInfo;
			appInfo.name = info.name;
			appInfo.id = info.id;
			appInfo.url = info.url;
			appInfo.version = info.version;
			appInfo.iconUrl = info.iconUrl;
			result.applist.push_back(appInfo);
		}
	}
	
	request->callback_.Run(result);
}

void DeviceApiApplauncherClient::OnDestruct() {
	requestQueue.clear();
}

} // namespace content
