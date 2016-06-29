// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * WebDeviceStorageStatus.h
 *
 *  Created on: 2015. 12. 23.
 *      Author: pjs3232
 */


#ifndef WebDeviceStorageStatus_h
#define WebDeviceStorageStatus_h

#include "public/platform/WebPrivatePtr.h"

namespace blink {

struct StorageInfo {
	long type;
	long long capacity;
	long long availableCapacity;
	bool isRemovable;
};

class WebDeviceStorageStatus {
public:
	enum status {
		// Exception code
		SUCCESS = 0,
		FAILURE = -1,
		NOT_ENABLED_PERMISSION = -2,
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

	WebDeviceStorageStatus()
	{
	}

	unsigned functionCode = 0;

	int resultCode = -1;
	int listSize = 0;
	
	StorageInfo* mStorageList;
};

} // namespace blink

#endif // WebDeviceStorageStatus_h
