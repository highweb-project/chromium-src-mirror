// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "platform/wtf/build_config.h"
#include "DeviceCpuStatus.h"

namespace blink {

	void DeviceCpuStatus::setResultCode(int code) {
		mResultCode = code;
	}

	void DeviceCpuStatus::setLoad(double load) {
		mLoad = load;
	}
	
	int DeviceCpuStatus::resultCode() {
		return mResultCode;
	}

	double DeviceCpuStatus::load() {
		return mLoad;
	}

	void DeviceCpuStatus::setFunctionCode(int function) {
		mFunctionCode = function;
	}
	int DeviceCpuStatus::getFunctionCode() {
		return mFunctionCode;
	}

	DeviceCpuStatus::~DeviceCpuStatus() {
	}

	DeviceCpuStatus::DeviceCpuStatus() {
	}
} // namespace blink