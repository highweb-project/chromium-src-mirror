// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceGalleryListener_h
#define DeviceGalleryListener_h
#include "wtf/text/WTFString.h"
#include "public/platform/modules/device_gallery/WebDeviceGalleryListener.h"

namespace blink {

class DeviceGallery;
class DeviceGalleryStatus;
class GalleryMediaObject;

class DeviceGalleryListener final : public GarbageCollectedFinalized<DeviceGalleryListener>, public WebDeviceGalleryListener {
public:
	static DeviceGalleryListener* instance(DeviceGallery*);
	~DeviceGalleryListener();

	void resultDeviceGallery(const WebDeviceGalleryStatus&) override;
	void copyMediaObjectWebToBlink(WebDeviceGalleryMediaObject* source, GalleryMediaObject* dest);

	DeviceGalleryStatus* lastData();

	DECLARE_VIRTUAL_TRACE();

private:
	DeviceGalleryListener(DeviceGallery*);
	Member<DeviceGallery> mDeviceGallery;
	Member<DeviceGalleryStatus> mGalleryStatus;
};

} // namespace blink

#endif // DeviceGalleryListener_h