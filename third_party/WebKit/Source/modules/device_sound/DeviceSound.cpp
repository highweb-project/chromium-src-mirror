// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "modules/device_sound/DeviceSound.h"

#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"
#include "DeviceSoundStatus.h"
#include "DeviceSoundScriptCallback.h"
#include "modules/device_sound/DeviceVolume.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"

#include "platform/mojo/MojoHelper.h"
#include "public/platform/ServiceRegistry.h"

namespace blink {

DeviceSound::DeviceSound(LocalFrame& frame)
	: mClient(DeviceApiPermissionController::from(frame)->client())
{
	mOrigin = frame.document()->url().strippedForUseAsReferrer();
	mClient->SetOrigin(mOrigin.latin1().data());
	d_functionData.clear();
}

DeviceSound::~DeviceSound()
{
	if (soundManager.is_bound()) {
		soundManager.reset();
	}
	d_functionData.clear();
	mCallbackList.clear();
}


void DeviceSound::outputDeviceType(DeviceSoundScriptCallback* callback) {
	functionData* data = new functionData(function::FUNC_OUTPUT_DEVICE_TYPE);
	//data->scriptCallback = callback;
	mCallbackList.append(callback);
	d_functionData.append(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceSound::outputDeviceTypeInternal() {
	if (!soundManager.is_bound()) {
		Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&soundManager));
	}
	soundManager->outputDeviceType(
		createBaseCallback(bind<device::blink::DeviceSound_ResultCodePtr>(&DeviceSound::mojoResultCallback, this)));
}

void DeviceSound::deviceVolume(DeviceSoundScriptCallback* callback) {
	functionData* data = new functionData(function::FUNC_DEVICE_VOLUME);
	//data->scriptCallback = callback;
	mCallbackList.append(callback);
	d_functionData.append(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceSound::deviceVolumeInternal() {
	if (!soundManager.is_bound()) {
		Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&soundManager));
	}
	soundManager->deviceVolume(
		createBaseCallback(bind<device::blink::DeviceSound_ResultCodePtr>(&DeviceSound::mojoResultCallback, this)));
}

void DeviceSound::resultCodeCallback(DeviceSoundStatus* status) {
	if (status != nullptr) {
		notifyCallback(status, mCallbackList.at(0).get());
	} else {
		notifyError(ErrorCodeList::FAILURE, mCallbackList.at(0).get());
	}
}

void DeviceSound::mojoResultCallback(device::blink::DeviceSound_ResultCodePtr result) {
	DCHECK(result.get());
	DeviceSoundStatus* status = nullptr;
	if (result.get() != nullptr) {
		status = DeviceSoundStatus::create();
		status->setResultCode(result->resultCode);
		switch(result->functionCode) {
			case function::FUNC_OUTPUT_DEVICE_TYPE: {
				status->setOutputType(result->outputType);
				break;
			}
			case function::FUNC_DEVICE_VOLUME: {
				DeviceVolume volume = DeviceVolume();
				if (result->volume.get() != nullptr) {
					volume.setMediaVolume(result->volume->MediaVolume);
					volume.setNotificationVolume(result->volume->NotificationVolume);
					volume.setAlarmVolume(result->volume->AlarmVolume);
					volume.setBellVolume(result->volume->BellVolume);
					volume.setCallVolume(result->volume->CallVolume);
					volume.setSystemVolume(result->volume->SystemVolume);
					volume.setDTMFVolume(result->volume->DTMFVolume);
				}
				status->setVolume(volume);
				break;
			}
		}
	}
	resultCodeCallback(status);
}

void DeviceSound::notifyCallback(DeviceSoundStatus* status, DeviceSoundScriptCallback* callback) {
	if (callback != NULL) {
		callback->handleEvent(status);
		callback = nullptr;
	}
	if (d_functionData.size() > 0)
		d_functionData.removeFirst();

	if(mCallbackList.size() > 0)
		mCallbackList.remove(0);
	continueFunction();
}

void DeviceSound::notifyError(int errorCode, DeviceSoundScriptCallback* callback) {
	DeviceSoundStatus* status = DeviceSoundStatus::create();
	status->setResultCode(errorCode);
	notifyCallback(status, callback);
}

void DeviceSound::continueFunction() {
	if (!ViewPermissionState && d_functionData.size() > 0) {
		requestPermission();
		return;
	}
	if (d_functionData.size() > 0) {
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_OUTPUT_DEVICE_TYPE : {
				outputDeviceTypeInternal();
				break;
			}
			case function::FUNC_DEVICE_VOLUME : {
				deviceVolumeInternal();
				break;
			}
			default: {
				break;
			}
		}
	}
}

void DeviceSound::requestPermission() {
	if(mClient) {
		mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
				PermissionAPIType::SYSTEM_INFORMATION,
				PermissionOptType::VIEW,
				base::Bind(&DeviceSound::onPermissionChecked, base::Unretained(this))));
	}
}

void DeviceSound::onPermissionChecked(PermissionResult result)
{
	if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK) {
		ViewPermissionState = true;
	}
	if (ViewPermissionState) {
		continueFunction();
	} else {
		notifyError(ErrorCodeList::NOT_ENABLED_PERMISSION, mCallbackList.at(0).get());
	}
}

DEFINE_TRACE(DeviceSound)
{
	//visitor->trace(d_functionData);
	visitor->trace(mCallbackList);
}

} // namespace blink
