/*
 * device_api_permission_check_message_filter.h
 *
 *  Created on: 2015. 11. 30.
 *      Author: azureskybox
 */

#ifndef CONTENT_BROWSER_DEVICE_API_PERMISSION_CHECK_MESSAGE_FILTER_H_
#define CONTENT_BROWSER_DEVICE_API_PERMISSION_CHECK_MESSAGE_FILTER_H_

// #include "base/basictypes.h"
#include "base/callback.h"
#include "base/memory/weak_ptr.h"

#include "content/public/browser/browser_message_filter.h"
#include "content/public/common/device_api_permission_request.h"
#include <utility>

namespace content {

class DeviceApiPermissionCheckMessageFilter : public BrowserMessageFilter {
 public:
	DeviceApiPermissionCheckMessageFilter(int render_process_id);

  // BrowserMessageFilter implementation.
  bool OnMessageReceived(const IPC::Message& message) override;

  void OnPermissionCheckRequested(int frame_id, int api_type, int operation_type);
  void RequestOnUI(int render_process_id, int render_frame_id, int api_type, int operation_type);

  void OnPermissionRequestResult(DeviceApiPermissionRequestResult result);
  void SendOnIO(int result);

 private:
  ~DeviceApiPermissionCheckMessageFilter() override;

  int render_process_id_;
  int render_frame_id_;

  DISALLOW_COPY_AND_ASSIGN(DeviceApiPermissionCheckMessageFilter);
};

}  // namespace content

#endif /* CONTENT_BROWSER_DEVICE_API_PERMISSION_CHECK_MESSAGE_FILTER_H_ */
