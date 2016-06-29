// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * WebDeviceSoundStatus.h
 *
 *  Created on: 2015. 12. 17.
 *      Author: pjs3232
 */


#ifndef WebDeviceSoundStatus_h
#define WebDeviceSoundStatus_h

#include "public/platform/WebPrivatePtr.h"

namespace blink {

struct WebDeviceVolume {
	float mediaVolume;
	float notificationVolume;
	float alarmVolume;
	float bellVolume;
	float callVolume;
	float systemVolume;
	float DTMFVolume;
};

class WebDeviceSoundStatus {
public:
	enum status {
		// Exception code
		SUCCESS = 0,
		FAILURE = -1,
		NOT_ENABLED_PERMISSION = -2,
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

	WebDeviceSoundStatus()
	{
	}

	unsigned functionCode = 0;

	int resultCode = -1;

	int outputType = 0;
	WebDeviceVolume volume;
};

} // namespace blink

#endif // WebDeviceSoundStatus_h
