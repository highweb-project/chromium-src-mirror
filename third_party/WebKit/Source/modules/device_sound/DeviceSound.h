// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceSound_h
#define DeviceSound_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/Deque.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"
#include "device/sound/devicesound_manager.mojom-blink.h"
#include "device/sound/devicesound_resultData.mojom-blink.h"

namespace blink {

class LocalFrame;
class DeviceSoundStatus;
class DeviceSoundScriptCallback;
class DeviceVolume;

class DeviceSound
	: public GarbageCollectedFinalized<DeviceSound>
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
		FUNC_DEVICE_VOLUME = 1,
		FUNC_OUTPUT_DEVICE_TYPE = 2,
	};

	enum device_type {
		DEVICE_SPEAKER_PHONE = 1,
		DEVICE_WIRED_HEADSET,
		DEVICE_BLUETOOTH,
		DEVICE_DEFAULT,
	};

	struct functionData {
		int functionCode = -1;
		//DeviceSoundScriptCallback* scriptCallback = nullptr;
		functionData(int code) {
			functionCode = code;
		}
	};

	static DeviceSound* create(LocalFrame& frame) {
		DeviceSound* devicesound = new DeviceSound(frame);
		return devicesound;
	}
	virtual ~DeviceSound();

	void outputDeviceType(DeviceSoundScriptCallback* callback);
	void deviceVolume(DeviceSoundScriptCallback* callback);

	void resultCodeCallback(DeviceSoundStatus* status);
	void notifyCallback(DeviceSoundStatus*, DeviceSoundScriptCallback*);
	void notifyError(int, DeviceSoundScriptCallback*);
	void continueFunction();

	void requestPermission();
	void onPermissionChecked(PermissionResult result);

	DECLARE_TRACE();

private:
	DeviceSound(LocalFrame& frame);

	void outputDeviceTypeInternal();
	void deviceVolumeInternal();

	void mojoResultCallback(device::blink::DeviceSound_ResultCodePtr result);

	Deque<functionData*> d_functionData;
	HeapVector<Member<DeviceSoundScriptCallback>> mCallbackList;

	WTF::String mOrigin;
	WebDeviceApiPermissionCheckClient* mClient;

	bool ViewPermissionState = false;

	device::blink::DeviceSoundManagerPtr soundManager;
};

} // namespace blink

#endif // DeviceSound_h
