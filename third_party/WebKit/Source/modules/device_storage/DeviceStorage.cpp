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
#include "DeviceStorageStatus.h"
#include "DeviceStorageScriptCallback.h"
#include "modules/device_storage/DeviceStorage.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"

#include "platform/mojo/MojoHelper.h"
#include "public/platform/InterfaceProvider.h"

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
	if (storageManager.is_bound()) {
		storageManager.reset();
	}
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

void DeviceStorage::getDeviceStorageInternal() {
	if (!storageManager.is_bound()) {
		Platform::current()->interfaceProvider()->getInterface(mojo::GetProxy(&storageManager));
	}
	storageManager->getDeviceStorage(
		convertToBaseCallback(WTF::bind(&DeviceStorage::mojoResultCallback, wrapPersistent(this))));
}

void DeviceStorage::resultCodeCallback(DeviceStorageStatus* status) {
	if (status != nullptr) {
		notifyCallback(status, mCallbackList.at(0).get());
	} else {
		notifyError(ErrorCodeList::kFailure, mCallbackList.at(0).get());
	}
}

void DeviceStorage::mojoResultCallback(device::blink::DeviceStorage_ResultCodePtr result) {
	DCHECK(result.get());
	DeviceStorageStatus* status = nullptr;
	if (result.get() != nullptr) {
		status = DeviceStorageStatus::create();
		status->setResultCode(result->resultCode);
		switch(result->functionCode) {
			case function::FUNC_GET_DEVICE_STORAGE: {
				if (result->storageList.has_value() > 0) {
					size_t size = result->storageList.value().size();
					for(size_t i = 0; i < size; i++) {
						DeviceStorageInfo info = DeviceStorageInfo();
						info.setAvailableCapacity(result->storageList.value()[i].get()->availableCapacity);
						info.setCapacity(result->storageList.value()[i].get()->capacity);
						info.setIsRemovable(result->storageList.value()[i].get()->isRemovable);
						info.setType(result->storageList.value()[i].get()->type);
						status->storageList().append(info);
					}
				}
				break;
			}
		}
	}
	resultCodeCallback(status);
}

void DeviceStorage::notifyCallback(DeviceStorageStatus* status, DeviceStorageScriptCallback* callback) {
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

void DeviceStorage::notifyError(int errorCode, DeviceStorageScriptCallback* callback) {
	DeviceStorageStatus* status = DeviceStorageStatus::create();
	status->setResultCode(errorCode);
	notifyCallback(status, callback);
}

void DeviceStorage::continueFunction() {
	if (!ViewPermissionState && d_functionData.size() > 0) {
		requestPermission();
		return;
	}
	if (d_functionData.size() > 0) {
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_GET_DEVICE_STORAGE : {
				getDeviceStorageInternal();
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
		notifyError(ErrorCodeList::kNotEnabledPermission, mCallbackList.at(0).get());
	}
}

DEFINE_TRACE(DeviceStorage)
{
	//visitor->trace(d_functionData);
	visitor->trace(mCallbackList);
}

} // namespace blink
