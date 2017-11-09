// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "platform/wtf/build_config.h"
#include "DeviceGalleryStatus.h"

namespace blink {

	DeviceGalleryStatus::DeviceGalleryStatus() {
		data.clear();
		mFunction = 0;
		mResultCode = 0;
	}

	DeviceGalleryStatus::~DeviceGalleryStatus() {
		data.clear();
	}

	void DeviceGalleryStatus::setFunctionCode(int function) {
		mFunction = function;
	}

	void DeviceGalleryStatus::setResultCode(int resultCode) {
		mResultCode = resultCode;
	}

	HeapVector<Member<GalleryMediaObject>>& DeviceGalleryStatus::mediaObjectList() {
		return data;
	}

} // namespace blink