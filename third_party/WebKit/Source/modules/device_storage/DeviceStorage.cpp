// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "modules/device_storage/DeviceStorage.h"

// #include "base/basictypes.h"
#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"
#include "public/platform/WebString.h"
#include "DeviceStorageListener.h"
#include "DeviceStorageStatus.h"
#include "DeviceStorageScriptCallback.h"
#include "modules/device_storage/DeviceStorage.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
namespace blink {

DeviceStorage::DeviceStorage(LocalFrame& frame)
	: mClient(DeviceApiPermissionController::from(frame)->client())
{
	mOrigin = frame.document()->url().strippedForUseAsReferrer();
	mClient->SetOrigin(mOrigin.latin1().data());
	d_functionData.clear();
}

DeviceStorage::~DeviceStorage()
{
}

void DeviceStorage::getDeviceStorage(DeviceStorageScriptCallback* callback) {
	functionData* data = new functionData(function::FUNC_GET_DEVICE_STORAGE);
	//data->scriptCallback = callback;
	mCallbackList.append(callback);
	d_functionData.append(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceStorage::resultCodeCallback() {
	DeviceStorageStatus* status = nullptr;
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

void DeviceStorage::notifyCallback(DeviceStorageStatus* status, DeviceStorageScriptCallback* callback) {
	if (callback != NULL) {
		callback->handleEvent(status);
		callback = nullptr;
	}
	Platform::current()->resetDeviceStorageDispatcher();
	if (d_functionData.size() > 0)
		d_functionData.removeFirst();
	if(mCallbackList.size() > 0)
		mCallbackList.remove(0);
	continueFunction();
}

void DeviceStorage::notifyError(int errorCode, DeviceStorageScriptCallback* callback) {
	DeviceStorageStatus* status = DeviceStorageStatus::create();
	status->setResultCode(errorCode);
	notifyCallback(status, callback);
}

void DeviceStorage::continueFunction() {
	if (!ViewPermissionState) {
		requestPermission();
		return;
	}
	if (d_functionData.size() > 0) {
		if (mCallback == NULL) {
			mCallback = DeviceStorageListener::instance(this);
		}
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_GET_DEVICE_STORAGE : {
				Platform::current()->getDeviceStorage(mCallback);
				break;
			}
			default: {
				break;
			}
		}
	}
}

void DeviceStorage::requestPermission() {
	if(mClient) {
		mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
				PermissionAPIType::SYSTEM_INFORMATION,
				PermissionOptType::VIEW,
				base::Bind(&DeviceStorage::onPermissionChecked, base::Unretained(this))));
	}
}

void DeviceStorage::onPermissionChecked(PermissionResult result)
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

DEFINE_TRACE(DeviceStorage)
{
	visitor->trace(mCallback);
	//visitor->trace(d_functionData);
	visitor->trace(mCallbackList);
}

} // namespace blink
