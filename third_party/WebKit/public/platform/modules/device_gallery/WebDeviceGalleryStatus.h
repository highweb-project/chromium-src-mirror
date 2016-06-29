// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * WebDeviceGalleryStatus.h
 *
 *  Created on: 2016. 01. 25.
 *      Author: pjs3232
 */


#ifndef WebDeviceGalleryStatus_h
#define WebDeviceGalleryStatus_h

#include "public/platform/WebPrivatePtr.h"
#include "WebDeviceGalleryStruct.h"

namespace blink {

class WebDeviceGalleryStatus {
public:
	enum GalleryError{
		// Exception code
		SUCCESS = -1,
		NOT_ENABLED_PERMISSION = -2,
		UNKNOWN_ERROR = 0,
		INVALID_ARGUMENT_ERROR = 1,
		TIMEOUT_ERROR = 3,
		PENDING_OPERATION_ERROR = 4,
		IO_ERROR = 5,
		NOT_SUPPORTED_ERROR = 6,
		MEDIA_SIZE_EXCEEDED = 20,
	};

	enum function {
		FUNC_FIND_MEDIA = 1,
		FUNC_GET_MEDIA,
		FUNC_DELETE_MEDIA,
	};

	WebDeviceGalleryStatus()
	{
		mMediaObjects = nullptr;
	}
	~WebDeviceGalleryStatus() {
		if (mMediaObjects != nullptr) {
			delete[] mMediaObjects;
			mMediaObjects = nullptr;
		}
	}

	unsigned functionCode = 0;
	int resultCode = -1;
	unsigned mediaListSize = 0;

	WebDeviceGalleryMediaObject* mMediaObjects;
};

} // namespace blink

#endif // WebDeviceGalleryStatus_h
