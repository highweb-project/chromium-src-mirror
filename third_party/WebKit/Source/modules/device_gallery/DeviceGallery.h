// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceGallery_h
#define DeviceGallery_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "modules/device_gallery/GalleryFindOptions.h"
#include "modules/device_gallery/GalleryMediaObject.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

#include "services/device/public/interfaces/devicegallery_manager.mojom-blink.h"
#include "services/device/public/interfaces/devicegallery_ResultCode.mojom-blink.h"

namespace blink {

class LocalFrame;
class GallerySuccessCallback;
class GalleryErrorCallback;
class GalleryMediaObject;
class DeviceGalleryStatus;

class DeviceGallery
	: public GarbageCollectedFinalized<DeviceGallery>
	, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:

	enum GalleryError{
		// Exception code
		kSuccess = -1,
		kNotEnabledPermission = -2,
		kUnknownError = 0,
		kInvalidArgumentError = 1,
		kTimeoutError = 3,
		kPendingOperationError = 4,
		kIoError = 5,
		kNotSupportedError = 6,
		kMediaSizeExceeded = 20,
		kNotSupportApi = 9999,
	};

	enum function {
		FUNC_FIND_MEDIA = 1,
		FUNC_GET_MEDIA,
		FUNC_DELETE_MEDIA,
	};

	struct functionData : public GarbageCollectedFinalized<functionData>{
		int functionCode = -1;
		GalleryFindOptions option;
		Member<GalleryMediaObject> object = nullptr;
		functionData(int code) {
			functionCode = code;
		}
		DEFINE_INLINE_TRACE() {
			visitor->Trace(object);
			visitor->Trace(option);
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

	void notifyCallback(DeviceGalleryStatus*, GallerySuccessCallback*);
	void notifyError(int, GalleryErrorCallback*);
	void continueFunction();

	void requestPermission(PermissionOptType type);
	void onPermissionChecked(PermissionResult result);
	void permissionCheck(PermissionOptType type);

	DECLARE_TRACE();

private:
	DeviceGallery(LocalFrame& frame);

	void convertScriptFindOptionToMojo(device::mojom::blink::MojoDeviceGalleryFindOptions* mojoFindOption, GalleryFindOptions& option);
	void convertScriptMediaObjectToMojo(device::mojom::blink::MojoDeviceGalleryMediaObject* mojoMediaObject, GalleryMediaObject* object);
	void convertMojoToScriptObject(GalleryMediaObject* object, device::mojom::blink::MojoDeviceGalleryMediaObject* mojoObject);

	void findMediaInternal();
	void getMediaInternal();
	void deleteMediaInternal();
	void mojoResultCallback(device::mojom::blink::DeviceGallery_ResultCodePtr result);

	device::mojom::blink::DeviceGalleryManagerPtr galleryManager;

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
