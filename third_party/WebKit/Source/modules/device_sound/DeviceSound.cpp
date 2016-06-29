// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "modules/device_sound/DeviceSound.h"

// #include "base/basictypes.h"
#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"
#include "public/platform/WebString.h"
#include "DeviceSoundListener.h"
#include "DeviceSoundStatus.h"
#include "DeviceSoundScriptCallback.h"
#include "modules/device_sound/DeviceVolume.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
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

void DeviceSound::resultCodeCallback() {
	DeviceSoundStatus* status = nullptr;
	if (mCallback != nullptr) {
		status = mCallback->lastData();
		mCallback = nullptr;
	}
	if (status != nullptr) {
		notifyCallback(status, mCallbackList.at(0).get());
	} else {
		notifyError(ErrorCodeList::FAILURE, mCallbackList.at(0).get());
	}
}

void DeviceSound::notifyCallback(DeviceSoundStatus* status, DeviceSoundScriptCallback* callback) {
	if (callback != NULL) {
		callback->handleEvent(status);
		callback = nullptr;
	}
	Platform::current()->resetDeviceSoundDispatcher();
	if (d_functionData.size() > 0)
		d_functionData.removeFirst();
	continueFunction();
}

void DeviceSound::notifyError(int errorCode, DeviceSoundScriptCallback* callback) {
	DeviceSoundStatus* status = DeviceSoundStatus::create();
	status->setResultCode(errorCode);
	notifyCallback(status, callback);
}

void DeviceSound::continueFunction() {
	if (!ViewPermissionState) {
		requestPermission();
		return;
	}
	if (d_functionData.size() > 0) {
		if (mCallback == NULL) {
			mCallback = DeviceSoundListener::instance(this);
		}
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_OUTPUT_DEVICE_TYPE : {
				Platform::current()->outputDeviceType(mCallback);
				break;
			}
			case function::FUNC_DEVICE_VOLUME : {
				Platform::current()->deviceVolume(mCallback);
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
	visitor->trace(mCallback);
	//visitor->trace(d_functionData);
	visitor->trace(mCallbackList);
}

} // namespace blink
