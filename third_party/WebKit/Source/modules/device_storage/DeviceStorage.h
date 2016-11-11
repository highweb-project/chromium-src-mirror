// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceStorage_h
#define DeviceStorage_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/Deque.h"
#include "wtf/text/WTFString.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

#include "device/storage/devicestorage_manager.mojom-blink.h"
#include "device/storage/devicestorage_ResultCode.mojom-blink.h"

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
		SUCCESS = 0,
		FAILURE = -1,
		NOT_ENABLED_PERMISSION = -2,
		NOT_SUPPORT_API = 9999,
	};

	enum function {
		FUNC_GET_DEVICE_STORAGE = 1,
	};

	enum device_storage_type {
		DEVICE_UNKNOWN = 1,
		DEVICE_HARDDISK,
		DEVICE_FLOPPYDISK,
		DEVICE_OPTICAL,
		//android storage type
		DEVICE_INTERNAL,
		DEVICE_EXTERNAL,
		DEVICE_SDCARD,
		DEVICE_USB,
	};

	struct functionData {
		int functionCode = -1;
		//DeviceStorageScriptCallback* scriptCallback = nullptr;
		String str1;
		functionData(int code) {
			functionCode = code;
		}
	};

	static DeviceStorage* create(LocalFrame& frame) {
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
	void mojoResultCallback(device::blink::DeviceStorage_ResultCodePtr result);

	Deque<functionData*> d_functionData;
	HeapVector<Member<DeviceStorageScriptCallback>> mCallbackList;

	WTF::String mOrigin;
	WebDeviceApiPermissionCheckClient* mClient;

	bool ViewPermissionState = false;

	device::blink::DeviceStorageManagerPtr storageManager;
};

} // namespace blink

#endif // DeviceStorage_h
