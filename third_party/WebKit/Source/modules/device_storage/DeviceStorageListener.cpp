// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "DeviceStorageListener.h"
#include "DeviceStorage.h"
#include "DeviceStorageStatus.h"

#include "public/platform/modules/device_systeminformation/WebDeviceStorageStatus.h"
#include "public/platform/Platform.h"


namespace blink {

DeviceStorageListener* DeviceStorageListener::instance(DeviceStorage* storage)
{
	DeviceStorageListener* callback = new DeviceStorageListener(storage);
	return callback;
}

DeviceStorageListener::DeviceStorageListener(DeviceStorage* storage)
{
	mDeviceStorage = storage;
}

DeviceStorageListener::~DeviceStorageListener()
{
	mDeviceStorage = NULL;
	mDeviceStorageStatus = NULL;
}

DEFINE_TRACE(DeviceStorageListener)
{
	visitor->trace(mDeviceStorage);
	visitor->trace(mDeviceStorageStatus);
}

void DeviceStorageListener::resultDeviceStorage(const WebDeviceStorageStatus& storageStatus) {
	mDeviceStorageStatus = DeviceStorageStatus::create();
	mDeviceStorageStatus.get()->setResultCode(storageStatus.resultCode);
	switch(storageStatus.functionCode) {
		case WebDeviceStorageStatus::function::FUNC_GET_DEVICE_STORAGE: {
			if (storageStatus.mStorageList != nullptr) {
				unsigned size = storageStatus.listSize;
				for(unsigned i = 0; i < size; i++) {
					DeviceStorageInfo info = DeviceStorageInfo();
					info.setAvailableCapacity(storageStatus.mStorageList[i].availableCapacity);
					info.setCapacity(storageStatus.mStorageList[i].capacity);
					info.setIsRemovable(storageStatus.mStorageList[i].isRemovable);
					info.setType(storageStatus.mStorageList[i].type);
					mDeviceStorageStatus.get()->storageList().append(info);
				}
			}
			break;
		}
	}
	mDeviceStorage->resultCodeCallback();
}

DeviceStorageStatus* DeviceStorageListener::lastData()
{
	if (mDeviceStorageStatus != nullptr)
		return mDeviceStorageStatus.get();
	else
		return nullptr;
}

} // namespace blink