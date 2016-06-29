// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceGallery_h
#define DeviceGallery_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "wtf/PassOwnPtr.h"
#include "DeviceGalleryListener.h"
#include "wtf/Deque.h"
#include "modules/device_gallery/GalleryFindOptions.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

namespace blink {

class LocalFrame;
class DeviceGalleryListener;
class GallerySuccessCallback;
class GalleryErrorCallback;
class GalleryMediaObject;
class DeviceGalleryStatus;
struct WebDeviceGalleryMediaObject;
struct WebDeviceGalleryFindOptions;

class DeviceGallery
	: public GarbageCollectedFinalized<DeviceGallery>
	, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
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

	struct functionData : public GarbageCollectedFinalized<functionData>{
		int functionCode = -1;
		//GallerySuccessCallback* successCallback = nullptr;
		//GalleryErrorCallback* errorCallback = nullptr;
		GalleryFindOptions option;
		Member<GalleryMediaObject> object = nullptr;
		functionData(int code) {
			functionCode = code;
		}
		DEFINE_INLINE_TRACE() {
			visitor->trace(object);
			visitor->trace(option);
		}
	};

	static DeviceGallery* create(LocalFrame& frame) {
		DeviceGallery* devicegallery = new DeviceGallery(frame);
		return devicegallery;
	}
	virtual ~DeviceGallery();

	void findMedia(GalleryFindOptions findOptions, GallerySuccessCallback* successCallback, GalleryErrorCallback* errorCallback);
	void getMedia(GalleryMediaObject* media, GallerySuccessCallback* successCallback, GalleryErrorCallback* errorCallback);
	void deleteMedia(GalleryMediaObject* media, GalleryErrorCallback* errorCallback);

	void resultCodeCallback();
	void notifyCallback(DeviceGalleryStatus*, GallerySuccessCallback*);
	void notifyError(int, GalleryErrorCallback*);
	void continueFunction();

	void copyFindOptions(GalleryFindOptions& source, WebDeviceGalleryFindOptions* dest);
	void copyMediaObjectNotBlob(GalleryMediaObject* source, WebDeviceGalleryMediaObject* dest);

	void requestPermission(PermissionOptType type);
	void onPermissionChecked(PermissionResult result);
	void permissionCheck(PermissionOptType type);

	DECLARE_TRACE();

private:
	DeviceGallery(LocalFrame& frame);

	Member<DeviceGalleryListener> mCallback = nullptr;

	HeapDeque<Member<functionData>> d_functionData;
	HeapVector<Member<GallerySuccessCallback>> mSuccessCallbackList;
	HeapVector<Member<GalleryErrorCallback>> mErrorCallbackList;

	WTF::String mOrigin;
	WebDeviceApiPermissionCheckClient* mClient;

	bool FindPermissionState = false;
	bool ViewPermissionState = false;
	bool DeletePermissionState = false;
};

} // namespace blink

#endif // DeviceGallery_h
