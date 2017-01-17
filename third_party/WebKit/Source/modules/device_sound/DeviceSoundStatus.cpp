// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "DeviceSoundStatus.h"

namespace blink {

	void DeviceSoundStatus::setResultCode(int code) {
		mResultCode = code;
	}
	void DeviceSoundStatus::setVolume(DeviceVolume& value) {
		mVolume = value;
	}

	void DeviceSoundStatus::setOutputType(int type) {
		mOutputType = type;
	}
	
	int DeviceSoundStatus::resultCode() {
		return mResultCode;
	}

	int DeviceSoundStatus::outputType() {
		return mOutputType;
	}

	void DeviceSoundStatus::volume(DeviceVolume& value) {
		value = mVolume;
	}


	DeviceSoundStatus::~DeviceSoundStatus() {
	}

	DeviceSoundStatus::DeviceSoundStatus() {
	}
} // namespace blink