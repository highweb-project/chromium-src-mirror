// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "platform/wtf/build_config.h"
#include "DeviceStorageStatus.h"

namespace blink {

	void DeviceStorageStatus::setResultCode(int code) {
		mResultCode = code;
	}
	
	int DeviceStorageStatus::resultCode() {
		return mResultCode;
	}

	DeviceStorageStatus::~DeviceStorageStatus() {
		mStorageList.clear();
	}

	DeviceStorageStatus::DeviceStorageStatus() {
	}

	HeapVector<DeviceStorageInfo>& DeviceStorageStatus::storageList() {
		return mStorageList;
	}
} // namespace blink