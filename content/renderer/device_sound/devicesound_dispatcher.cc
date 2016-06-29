// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/device_sound/devicesound_dispatcher.h"

#include "content/public/common/service_registry.h"
#include "content/public/renderer/render_thread.h"

namespace content {

DeviceSoundDispatcher::DeviceSoundDispatcher() {
	if (ServiceRegistry* registry = RenderThread::Get()->GetServiceRegistry()) {
		registry->ConnectToRemoteService(mojo::GetProxy(&manager_));
	}
}

DeviceSoundDispatcher::~DeviceSoundDispatcher() {
	manager_.reset();
	listener_ = nullptr;
}

void DeviceSoundDispatcher::outputDeviceType(blink::WebDeviceSoundListener* mCallback) {
	listener_ = mCallback;
	manager_->outputDeviceType(base::Bind(&DeviceSoundDispatcher::resultCodeCallback, base::Unretained(this)));
}

void DeviceSoundDispatcher::deviceVolume(blink::WebDeviceSoundListener* mCallback) {
	listener_ = mCallback;
	manager_->deviceVolume(base::Bind(&DeviceSoundDispatcher::resultCodeCallback, base::Unretained(this)));
}

void DeviceSoundDispatcher::resultCodeCallback(device::DeviceSound_ResultCodePtr resultCodePtr) {
	DCHECK(resultCodePtr);
	if (resultCodePtr != nullptr) {
		webDeviceSoundstatus.functionCode = resultCodePtr->functionCode;
		webDeviceSoundstatus.resultCode = resultCodePtr->resultCode;
		switch(resultCodePtr->functionCode) {
			case blink::WebDeviceSoundStatus::function::FUNC_OUTPUT_DEVICE_TYPE: {
				webDeviceSoundstatus.outputType = resultCodePtr->outputType;
				break;
			}
			case blink::WebDeviceSoundStatus::function::FUNC_DEVICE_VOLUME: {
				webDeviceSoundstatus.volume.mediaVolume = resultCodePtr->volume->MediaVolume;
				webDeviceSoundstatus.volume.notificationVolume = resultCodePtr->volume->NotificationVolume;
				webDeviceSoundstatus.volume.alarmVolume = resultCodePtr->volume->AlarmVolume;
				webDeviceSoundstatus.volume.bellVolume = resultCodePtr->volume->BellVolume;
				webDeviceSoundstatus.volume.callVolume = resultCodePtr->volume->CallVolume;
				webDeviceSoundstatus.volume.systemVolume = resultCodePtr->volume->SystemVolume;
				webDeviceSoundstatus.volume.DTMFVolume = resultCodePtr->volume->DTMFVolume;

				break;
			}
		}
	}
	if (listener_ != nullptr) {
		listener_->resultDeviceSound(webDeviceSoundstatus);
	}
}

}  // namespace content
