// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/device_storage/devicestorage_dispatcher.h"

#include "content/public/common/service_registry.h"
#include "content/public/renderer/render_thread.h"

namespace content {

DeviceStorageDispatcher::DeviceStorageDispatcher() {
	if (ServiceRegistry* registry = RenderThread::Get()->GetServiceRegistry()) {
		registry->ConnectToRemoteService(mojo::GetProxy(&manager_));
	}
}

DeviceStorageDispatcher::~DeviceStorageDispatcher() {
	manager_.reset();
	listener_ = nullptr;
	if (webDeviceStoragestatus.mStorageList != nullptr) {
		delete[] webDeviceStoragestatus.mStorageList;
		webDeviceStoragestatus.mStorageList = nullptr;
	}
}

void DeviceStorageDispatcher::getDeviceStorage(blink::WebDeviceStorageListener* mCallback) {
	listener_ = mCallback;
	manager_->getDeviceStorage(base::Bind(&DeviceStorageDispatcher::resultCodeCallback, base::Unretained(this)));
}

void DeviceStorageDispatcher::resultCodeCallback(device::DeviceStorage_ResultCodePtr resultCodePtr) {
	DCHECK(resultCodePtr);
	if (resultCodePtr != nullptr) {
		webDeviceStoragestatus.functionCode = resultCodePtr->functionCode;
		webDeviceStoragestatus.resultCode = resultCodePtr->resultCode;
		if (resultCodePtr->resultCode == blink::WebDeviceStorageStatus::status::SUCCESS) {
			switch(resultCodePtr->functionCode) {
				case blink::WebDeviceStorageStatus::function::FUNC_GET_DEVICE_STORAGE: {
					unsigned listSize = resultCodePtr->storageList.size();
					webDeviceStoragestatus.mStorageList = new blink::StorageInfo[listSize]();
					webDeviceStoragestatus.listSize = listSize;
					for(unsigned i = 0; i < listSize; i++) {
						webDeviceStoragestatus.mStorageList[i].type = resultCodePtr->storageList[i]->type;
						webDeviceStoragestatus.mStorageList[i].capacity = resultCodePtr->storageList[i]->capacity;
						webDeviceStoragestatus.mStorageList[i].availableCapacity = resultCodePtr->storageList[i]->availableCapacity;
						webDeviceStoragestatus.mStorageList[i].isRemovable = resultCodePtr->storageList[i]->isRemovable;
					}
					break;
				}
			}
		}
	}
	if (listener_ != nullptr) {
			listener_->resultDeviceStorage(webDeviceStoragestatus);
	}
}

}  // namespace content
