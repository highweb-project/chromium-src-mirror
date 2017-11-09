// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DEVICE_API_PERMISSION_DEVICE_API_APPLAUNCHER_CLIENT_H_
#define CONTENT_RENDERER_DEVICE_API_PERMISSION_DEVICE_API_APPLAUNCHER_CLIENT_H_

#include <map>
#include <deque>
#include <utility>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_thread.h"

#include "content/common/device_api/device_api_applauncher_messages.h"
#include "third_party/WebKit/public/platform/modules/device_api/WebDeviceApiApplauncherClient.h"

namespace content {

struct DeviceApiApplauncherRequest;

class CONTENT_EXPORT DeviceApiApplauncherClient :
	public RenderFrameObserver,
	NON_EXPORTED_BASE(public blink::WebDeviceApiApplauncherClient) {
public:
	explicit DeviceApiApplauncherClient(RenderFrame* render_frame);
	~DeviceApiApplauncherClient() override;

	void requestFunction(WebDeviceApiApplauncherRequest* request) override;
	void OnRequestResult(DeviceApiApplauncherResultMessage message);

  void OnDestruct() override;

private:
	bool OnMessageReceived(const IPC::Message& message) override;
	void DidFinishLoad() override;

	std::deque<WebDeviceApiApplauncherRequest*> requestQueue;
};

} // namespace content
#endif  // CONTENT_RENDERER_DEVICE_API_PERMISSION_DEVICE_API_APPLAUNCHER_CLIENT_H_ */
