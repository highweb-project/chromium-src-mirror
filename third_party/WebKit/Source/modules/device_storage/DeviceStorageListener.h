// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceStorageListener_h
#define DeviceStorageListener_h
#include "wtf/text/WTFString.h"
#include "public/platform/modules/device_systeminformation/WebDeviceStorageListener.h"

namespace blink {

class DeviceStorage;
class DeviceStorageStatus;

class DeviceStorageListener final : public GarbageCollectedFinalized<DeviceStorageListener>, public WebDeviceStorageListener {
public:
	static DeviceStorageListener* instance(DeviceStorage*);
	~DeviceStorageListener();

	void resultDeviceStorage(const WebDeviceStorageStatus&) override;

	DeviceStorageStatus* lastData();

	DECLARE_VIRTUAL_TRACE();

private:
	DeviceStorageListener(DeviceStorage*);
	Member<DeviceStorage> mDeviceStorage;
	Member<DeviceStorageStatus> mDeviceStorageStatus;
};

} // namespace blink

#endif // DeviceStorageListener_h