// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DEVICEGALLERY_DISPATCHER_H_
#define CONTENT_RENDERER_DEVICEGALLERY_DISPATCHER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "device/gallery/devicegallery_ResultCode.mojom.h"
#include "device/gallery/devicegallery_manager.mojom.h"

#include "third_party/WebKit/public/platform/modules/device_gallery/WebDeviceGalleryListener.h"
#include "third_party/WebKit/public/platform/modules/device_gallery/WebDeviceGalleryStatus.h"
#include "third_party/WebKit/public/platform/modules/device_gallery/WebDeviceGalleryStruct.h"

namespace blink {
	struct WebDeviceGalleryFindOptions;
	class WebDeviceGalleryListener;
	struct WebDeviceGalleryMediaContent;
	struct WebDeviceGalleryMediaObject;
	class WebDeviceGalleryStatus;
}

namespace content {

class CONTENT_EXPORT DeviceGalleryDispatcher {
	public:
		explicit DeviceGalleryDispatcher();
		~DeviceGalleryDispatcher();
		void findMedia(blink::WebDeviceGalleryFindOptions* findOptions, blink::WebDeviceGalleryListener* mCallback);
		void getMedia(blink::WebDeviceGalleryMediaObject* media, blink::WebDeviceGalleryListener* mCallback);
		void deleteMedia(blink::WebDeviceGalleryMediaObject* media, blink::WebDeviceGalleryListener* mCallback);
		void resultCodeCallback(device::DeviceGallery_ResultCodePtr resultCodePtr);

		void copyFindOptionsWebToMojo(blink::WebDeviceGalleryFindOptions* source, device::MojoDeviceGalleryFindOptions* dest);

		void copyMediaObjectWebToMojo(blink::WebDeviceGalleryMediaObject* source, device::MojoDeviceGalleryMediaObject* dest);
		void copyMediaObjectMojoToWeb(device::MojoDeviceGalleryMediaObject* source, blink::WebDeviceGalleryMediaObject* dest);
	private:
		device::DeviceGalleryManagerPtr manager_;
		blink::WebDeviceGalleryListener* listener_ = nullptr;
		blink::WebDeviceGalleryStatus webDeviceGallerystatus;

		DISALLOW_COPY_AND_ASSIGN(DeviceGalleryDispatcher);
};

}  // namespace content

#endif  // CONTENT_RENDERER_DEVICEGALLERY_DISPATCHER_H_
