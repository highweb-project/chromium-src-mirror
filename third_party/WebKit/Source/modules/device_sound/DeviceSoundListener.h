// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceSoundListener_h
#define DeviceSoundListener_h
#include "wtf/text/WTFString.h"
#include "public/platform/modules/device_systeminformation/WebDeviceSoundListener.h"

namespace blink {

class DeviceSound;
class DeviceSoundStatus;

class DeviceSoundListener final : public GarbageCollectedFinalized<DeviceSoundListener>, public WebDeviceSoundListener {
public:
	static DeviceSoundListener* instance(DeviceSound*);
	~DeviceSoundListener();

	void resultDeviceSound(const WebDeviceSoundStatus&) override;

	DeviceSoundStatus* lastData();

	DECLARE_VIRTUAL_TRACE();

private:
	DeviceSoundListener(DeviceSound*);
	Member<DeviceSound> mDeviceSound;
	Member<DeviceSoundStatus> mSoundStatus;
};

} // namespace blink

#endif // DeviceSoundListener_h