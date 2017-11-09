#include "content/browser/device_api/device_api_applauncher_message_filter.h"

#include "base/logging.h"
#include "content/browser/frame_host/render_frame_host_delegate.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/public/browser/browser_thread.h"

namespace content {

DeviceApiApplauncherMessageFilter::DeviceApiApplauncherMessageFilter(int render_process_id)
    : BrowserMessageFilter(DeviceApiApplauncherMsgStart),
    render_process_id_(render_process_id)
{
}

DeviceApiApplauncherMessageFilter::~DeviceApiApplauncherMessageFilter()
{
}

bool DeviceApiApplauncherMessageFilter::OnMessageReceived(const IPC::Message& message)
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(DeviceApiApplauncherMessageFilter, message)
    IPC_MESSAGE_HANDLER(DeviceApiApplauncherMsg_RequestFunction, OnReqeustFunction);
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void DeviceApiApplauncherMessageFilter::OnReqeustFunction(int frame_id, const DeviceApiApplauncherRequestMessage& message) {
  render_frame_id_ = frame_id;
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&DeviceApiApplauncherMessageFilter::OnReqeustFunctionOnUI, this, render_process_id_, message));
}

void DeviceApiApplauncherMessageFilter::OnReqeustFunctionOnUI(int frame_id, const DeviceApiApplauncherRequestMessage& message) {
   RenderFrameHostImpl* host = RenderFrameHostImpl::FromID(render_process_id_, render_frame_id_);
  if (host) {
    RenderFrameHostDelegate* render_delegate = host->delegate();

		if(render_delegate) {
      DeviceApiApplauncherRequest request(
        base::Bind(&DeviceApiApplauncherMessageFilter::OnRequestResult, base::Unretained(this)));
      request.functionCode = message.functionCode;
      request.appId = message.appId;
      render_delegate->RequestApplauncherRequestFunction(request);
		}
  }
}

void DeviceApiApplauncherMessageFilter::OnRequestResult(const DeviceApiApplauncherRequestResult result) {
  BrowserThread::PostTask(
		      BrowserThread::IO, FROM_HERE,
		      base::Bind(&DeviceApiApplauncherMessageFilter::OnRequestResultOnIO, this, result));
}

void DeviceApiApplauncherMessageFilter::OnRequestResultOnIO(const DeviceApiApplauncherRequestResult result) {
  DeviceApiApplauncherResultMessage message;
  message.resultCode = result.resultCode;
  message.functionCode = result.functionCode;
  for(AppLauncher_ApplicationInfo info : result.applist) {
    DeviceApiApplauncherApplicationInfo appInfo;
    appInfo.name = info.name;
    appInfo.id = info.id;
    appInfo.url = info.url;
    appInfo.version = info.version;
    appInfo.iconUrl = info.iconUrl;
    message.applist.push_back(appInfo);
  }
  Send(new DeviceApiApplauncherMsg_RequestResult(render_frame_id_, message));
}

}


