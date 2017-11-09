// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceSound_h
#define DeviceSound_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Deque.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"
#include "services/device/public/interfaces/devicesound_manager.mojom-blink.h"
#include "services/device/public/interfaces/devicesound_resultData.mojom-blink.h"

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
		kSuccess = 0,
		kFailure = -1,
		kNotEnabledPermission = -2,
		kNotSupportApi = 9999,
	};

	enum function {
		FUNC_DEVICE_VOLUME = 1,
		FUNC_OUTPUT_DEVICE_TYPE = 2,
	};

	enum device_type {
		kDeviceSpeakerPhone = 1,
		kDeviceWiredHeadset,
		kDeviceBluetooth,
		kDeviceDefault,
	};

	struct functionData {
		int functionCode = -1;
		//DeviceSoundScriptCallback* scriptCallback = nullptr;
		functionData(int code) {
			functionCode = code;
		}
	};

	static DeviceSound* Create(LocalFrame& frame) {
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

	void mojoResultCallback(device::mojom::blink::DeviceSound_ResultCodePtr result);

	Deque<functionData*> d_functionData;
	HeapVector<Member<DeviceSoundScriptCallback>> mCallbackList;

	WTF::String mOrigin;
	WebDeviceApiPermissionCheckClient* mClient;

	bool ViewPermissionState = false;

	device::mojom::blink::DeviceSoundManagerPtr soundManager;
};

} // namespace blink

#endif // DeviceSound_h
