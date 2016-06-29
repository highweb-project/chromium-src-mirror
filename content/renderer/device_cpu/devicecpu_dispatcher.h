// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DEVICECPU_DISPATCHER_H_
#define CONTENT_RENDERER_DEVICECPU_DISPATCHER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "device/cpu/devicecpu_ResultCode.mojom.h"
#include "device/cpu/devicecpu_manager.mojom.h"

#include "third_party/WebKit/public/platform/modules/device_systeminformation/WebDeviceCpuListener.h"
#include "third_party/WebKit/public/platform/modules/device_systeminformation/WebDeviceCpuStatus.h"

namespace blink {
class WebDeviceCpuStatus;
class WebDeviceCpuListener;
}

namespace content {

class CONTENT_EXPORT DeviceCpuDispatcher {
	public:
		explicit DeviceCpuDispatcher();
		~DeviceCpuDispatcher();
		void getDeviceCpuLoad(blink::WebDeviceCpuListener* mCallback);

		void getCpuLoad();
		void resultCodeCallback(device::DeviceCpu_ResultCodePtr resultCodePtr);

	private:
		device::DeviceCpuManagerPtr manager_;
		blink::WebDeviceCpuListener* listener_ = nullptr;
		blink::WebDeviceCpuStatus webDeviceCpustatus;

		bool runGetCpuLoad = false;

		DISALLOW_COPY_AND_ASSIGN(DeviceCpuDispatcher);
};

}  // namespace content

#endif  // CONTENT_RENDERER_DEVICECPU_DISPATCHER_H_
