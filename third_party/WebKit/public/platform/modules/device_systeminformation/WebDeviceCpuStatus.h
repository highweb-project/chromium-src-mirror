// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * WebDeviceCpuStatus.h
 *
 *  Created on: 2016. 01. 04.
 *      Author: pjs3232
 */


#ifndef WebDeviceCpuStatus_h
#define WebDeviceCpuStatus_h

#include "public/platform/WebPrivatePtr.h"

namespace blink {

class WebDeviceCpuStatus {
public:
	enum status {
		// Exception code
		SUCCESS = 0,
		FAILURE = -1,
		NOT_ENABLED_PERMISSION = -2,
	};

	enum function {
		FUNC_GET_CPU_LOAD = 0,
	};

	WebDeviceCpuStatus()
	{
	}

	unsigned functionCode = 0;

	int resultCode = -1;

	float load = 0;
};

} // namespace blink

#endif // WebDeviceCpuStatus_h
