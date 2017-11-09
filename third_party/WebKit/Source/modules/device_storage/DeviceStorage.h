// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceStorage_h
#define DeviceStorage_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Deque.h"
#include "platform/wtf/text/WTFString.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

#include "services/device/public/interfaces/devicestorage_manager.mojom-blink.h"
#include "services/device/public/interfaces/devicestorage_ResultCode.mojom-blink.h"

namespace blink {

class LocalFrame;
class DeviceStorageScriptCallback;
class DeviceStorageStatus;

class DeviceStorage
	: public GarbageCollectedFinalized<DeviceStorage>
	, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:

	enum ErrorCodeList{
		// Exception code
		kSuccess = 0,
		kFailure = -1,
		kNotEnabledPermission = -2,
		kNotSupportApi = 9999,
	};

	enum function {
		FUNC_GET_DEVICE_STORAGE = 1,
	};

	enum device_storage_type {
		kDeviceUnknown = 1,
		kDeviceHarddisk,
		kDeviceFloppydisk,
		kDeviceOptical,
		//android storage type
		kDeviceInternal,
		kDeviceExternal,
		kDeviceSdcard,
		kDeviceUsb,
	};

	struct functionData {
		int functionCode = -1;
		//DeviceStorageScriptCallback* scriptCallback = nullptr;
		String str1;
		functionData(int code) {
			functionCode = code;
		}
	};

	static DeviceStorage* Create(LocalFrame& frame) {
		DeviceStorage* devicestorage = new DeviceStorage(frame);
		return devicestorage;
	}
	virtual ~DeviceStorage();

	void getDeviceStorage(DeviceStorageScriptCallback* callback);

	void resultCodeCallback(DeviceStorageStatus* status);
	void notifyCallback(DeviceStorageStatus*, DeviceStorageScriptCallback*);
	void notifyError(int, DeviceStorageScriptCallback*);
	void continueFunction();

	void requestPermission();
	void onPermissionChecked(PermissionResult result);

	DECLARE_TRACE();

private:
	DeviceStorage(LocalFrame& frame);

	void getDeviceStorageInternal();
	void mojoResultCallback(device::mojom::blink::DeviceStorage_ResultCodePtr result);

	Deque<functionData*> d_functionData;
	HeapVector<Member<DeviceStorageScriptCallback>> mCallbackList;

	WTF::String mOrigin;
	WebDeviceApiPermissionCheckClient* mClient;

	bool ViewPermissionState = false;

	device::mojom::blink::DeviceStorageManagerPtr storageManager;
};

} // namespace blink

#endif // DeviceStorage_h
