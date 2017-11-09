// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "platform/wtf/build_config.h"
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
#include "services/device/public/interfaces/constants.mojom-blink.h"
#include "services/service_manager/public/cpp/connector.h"

namespace blink {

DeviceStorage::DeviceStorage(LocalFrame& frame)
	: mClient(DeviceApiPermissionController::From(frame)->client())
{
	mOrigin = frame.GetDocument()->Url().StrippedForUseAsReferrer();
	mClient->SetOrigin(mOrigin.Latin1().data());
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
	mCallbackList.push_back(callback);
	d_functionData.push_back(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceStorage::getDeviceStorageInternal() {
	if (!storageManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&storageManager));
	}
	storageManager->getDeviceStorage(
		ConvertToBaseCallback(WTF::Bind(&DeviceStorage::mojoResultCallback, WrapPersistent(this))));
}

void DeviceStorage::resultCodeCallback(DeviceStorageStatus* status) {
	if (status != nullptr) {
		notifyCallback(status, mCallbackList.at(0).Get());
	} else {
		notifyError(ErrorCodeList::kFailure, mCallbackList.at(0).Get());
	}
}

void DeviceStorage::mojoResultCallback(device::mojom::blink::DeviceStorage_ResultCodePtr result) {
	DCHECK(result.get());
	DeviceStorageStatus* status = nullptr;
	if (result.get() != nullptr) {
		status = DeviceStorageStatus::Create();
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
						status->storageList().push_back(info);
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
		d_functionData.pop_front();
	if(mCallbackList.size() > 0)
		mCallbackList.erase(0);
	continueFunction();
}

void DeviceStorage::notifyError(int errorCode, DeviceStorageScriptCallback* callback) {
	DeviceStorageStatus* status = DeviceStorageStatus::Create();
	status->setResultCode(errorCode);
	notifyCallback(status, callback);
}

void DeviceStorage::continueFunction() {
	if (!ViewPermissionState && d_functionData.size() > 0) {
		requestPermission();
		return;
	}
	if (d_functionData.size() > 0) {
		switch(d_functionData.front()->functionCode) {
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
		notifyError(ErrorCodeList::kNotEnabledPermission, mCallbackList.at(0).Get());
	}
}

DEFINE_TRACE(DeviceStorage)
{
	//visitor->trace(d_functionData);
	visitor->Trace(mCallbackList);
}

} // namespace blink
