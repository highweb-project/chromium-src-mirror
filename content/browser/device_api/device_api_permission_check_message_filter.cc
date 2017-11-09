/*
 * device_api_permission_check_message_filter.cc
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#include "content/browser/device_api/device_api_permission_check_message_filter.h"

#include "base/logging.h"
#include "content/browser/frame_host/render_frame_host_delegate.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/common/device_api/device_api_permission_check_messages.h"
#include "content/public/browser/browser_thread.h"

namespace content {

DeviceApiPermissionCheckMessageFilter::DeviceApiPermissionCheckMessageFilter(int render_process_id)
    : BrowserMessageFilter(DeviceApiPermissionCheckMsgStart),
	  render_process_id_(render_process_id)
{
}

DeviceApiPermissionCheckMessageFilter::~DeviceApiPermissionCheckMessageFilter()
{
}

bool DeviceApiPermissionCheckMessageFilter::OnMessageReceived(const IPC::Message& message)
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(DeviceApiPermissionCheckMessageFilter, message)
	  IPC_MESSAGE_HANDLER(DeviceApiPermissionCheckMsg_CheckPermission, OnPermissionCheckRequested);
	  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void DeviceApiPermissionCheckMessageFilter::OnPermissionCheckRequested(int frame_id, int api_type, int operation_type)
{
	DLOG(INFO) << "DeviceApiPermissionCheckMessageFilter::OnPermissionCheckRequested";

	render_frame_id_ = frame_id;

	  BrowserThread::PostTask(
	      BrowserThread::UI, FROM_HERE,
	      base::Bind(&DeviceApiPermissionCheckMessageFilter::RequestOnUI, this, render_process_id_, frame_id, api_type, operation_type));
}

void DeviceApiPermissionCheckMessageFilter::RequestOnUI(int render_process_id, int render_frame_id, int api_type, int operation_type)
{
	DLOG(INFO) << "DeviceApiPermissionCheckMessageFilter::RequestOnUI";

	RenderFrameHostImpl* host = RenderFrameHostImpl::FromID(render_process_id, render_frame_id);

	if(host) {
		RenderFrameHostDelegate* render_delegate = host->delegate();

		if(render_delegate) {
			DeviceApiPermissionRequest request(base::Bind(&DeviceApiPermissionCheckMessageFilter::OnPermissionRequestResult, base::Unretained(this)));
			request.api_type_ = (ApiType)api_type;
			request.operation_type_ = (OperationType)operation_type;

			render_delegate->RequestDeviceApiPermission(request);
		}
	}
	else {
		DLOG(INFO) << "host is null!";
	}
}

void DeviceApiPermissionCheckMessageFilter::OnPermissionRequestResult(DeviceApiPermissionRequestResult result) {
	DLOG(INFO) << "DeviceApiPermissionCheckMessageFilter::OnPermissionRequestResult";

	BrowserThread::PostTask(
		      BrowserThread::IO, FROM_HERE,
		      base::Bind(&DeviceApiPermissionCheckMessageFilter::SendOnIO, this, (int)result));
}

void DeviceApiPermissionCheckMessageFilter::SendOnIO(int result)
{
	DLOG(INFO) << "DeviceApiPermissionCheckMessageFilter::SendOnIO";

	Send(new DeviceApiPermissionCheckMsg_CheckPermissionResult(render_frame_id_, result));
}

}


