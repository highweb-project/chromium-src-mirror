// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DEVICESOUND_DISPATCHER_H_
#define CONTENT_RENDERER_DEVICESOUND_DISPATCHER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "device/sound/devicesound_resultData.mojom.h"
#include "device/sound/devicesound_manager.mojom.h"

#include "third_party/WebKit/public/platform/modules/device_systeminformation/WebDeviceSoundListener.h"
#include "third_party/WebKit/public/platform/modules/device_systeminformation/WebDeviceSoundStatus.h"

namespace blink {
class WebDeviceSoundStatus;
class WebDeviceSoundListener;
}

namespace content {

class CONTENT_EXPORT DeviceSoundDispatcher {
	public:
		explicit DeviceSoundDispatcher();
		~DeviceSoundDispatcher();
		void outputDeviceType(blink::WebDeviceSoundListener* mCallback);
		void deviceVolume(blink::WebDeviceSoundListener* mCallback);
		void resultCodeCallback(device::DeviceSound_ResultCodePtr resultCodePtr);
	private:
		device::DeviceSoundManagerPtr manager_;
		blink::WebDeviceSoundListener* listener_;
		blink::WebDeviceSoundStatus webDeviceSoundstatus;

		DISALLOW_COPY_AND_ASSIGN(DeviceSoundDispatcher);
};

}  // namespace content

#endif  // CONTENT_RENDERER_DEVICESOUND_DISPATCHER_H_
