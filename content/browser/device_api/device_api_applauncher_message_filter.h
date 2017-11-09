// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_API_APPLAUNCHER_MESSAGE_FILTER_H_
#define CONTENT_BROWSER_DEVICE_API_APPLAUNCHER_MESSAGE_FILTER_H_

#include "base/callback.h"
#include "base/memory/weak_ptr.h"

#include "content/public/browser/browser_message_filter.h"
#include "content/public/common/device_api_applauncher_request.h"
#include "content/common/device_api/device_api_applauncher_messages.h"
#include <utility>

namespace content {

class DeviceApiApplauncherMessageFilter : public BrowserMessageFilter {
 public:
	DeviceApiApplauncherMessageFilter(int render_process_id);

  bool OnMessageReceived(const IPC::Message& message) override;

  void OnReqeustFunction(int frame_id, const DeviceApiApplauncherRequestMessage& message);
  void OnReqeustFunctionOnUI(int frame_id, const DeviceApiApplauncherRequestMessage& message);

  void OnRequestResult(const DeviceApiApplauncherRequestResult result);
  void OnRequestResultOnIO(const DeviceApiApplauncherRequestResult result);

 private:
  ~DeviceApiApplauncherMessageFilter() override;

  int render_process_id_;
  int render_frame_id_;

  DISALLOW_COPY_AND_ASSIGN(DeviceApiApplauncherMessageFilter);
};

}  // namespace content

#endif /* CONTENT_BROWSER_DEVICE_API_APPLAUNCHER_MESSAGE_FILTER_H_ */
