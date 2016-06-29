// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "DeviceGalleryListener.h"
#include "DeviceGallery.h"
#include "DeviceGalleryStatus.h"
#include "GalleryMediaObject.h"

#include "core/fileapi/Blob.h"
#include "core/dom/DOMArrayBuffer.h"
#include "bindings/core/v8/ExceptionState.h"
//#include "bindings/core/v8/UnionTypesCore.h"
#include "core/fileapi/BlobPropertyBag.h"

#include "public/platform/modules/device_gallery/WebDeviceGalleryStatus.h"
#include "public/platform/Platform.h"

namespace blink {

DeviceGalleryListener* DeviceGalleryListener::instance(DeviceGallery* gallery)
{
	DeviceGalleryListener* callback = new DeviceGalleryListener(gallery);
	return callback;
}

DeviceGalleryListener::DeviceGalleryListener(DeviceGallery* gallery)
{
	mDeviceGallery = gallery;
}

DeviceGalleryListener::~DeviceGalleryListener()
{
	mDeviceGallery = NULL;
	mGalleryStatus = NULL;
}

DEFINE_TRACE(DeviceGalleryListener)
{
	visitor->trace(mDeviceGallery);
	visitor->trace(mGalleryStatus);
}

void DeviceGalleryListener::resultDeviceGallery(const WebDeviceGalleryStatus& galleryStatus) {
	mGalleryStatus = DeviceGalleryStatus::create();
	mGalleryStatus.get()->setResultCode(galleryStatus.resultCode);
	switch(galleryStatus.functionCode) {
		case WebDeviceGalleryStatus::function::FUNC_FIND_MEDIA: {
			for(unsigned i = 0; i < galleryStatus.mediaListSize; i++) {
				GalleryMediaObject* object = GalleryMediaObject::create();
				copyMediaObjectWebToBlink(&galleryStatus.mMediaObjects[i], object);
				mGalleryStatus.get()->mediaObjectList().append(object);
			}
			break;
		}
		case WebDeviceGalleryStatus::function::FUNC_GET_MEDIA: {
			for(unsigned i = 0; i < galleryStatus.mediaListSize; i++) {
				GalleryMediaObject* object = GalleryMediaObject::create();
				copyMediaObjectWebToBlink(&galleryStatus.mMediaObjects[i], object);
				mGalleryStatus.get()->mediaObjectList().append(object);
			}
			break;
		}
		default: {
			break;
		}
	}
	mDeviceGallery->resultCodeCallback();
}

void DeviceGalleryListener::copyMediaObjectWebToBlink(WebDeviceGalleryMediaObject* source, GalleryMediaObject* dest) {
	dest->setType(WTF::String(source->mType));
	dest->setDescription(WTF::String(source->mDescription));
	dest->setId(WTF::String(source->mId));
	dest->setTitle(WTF::String(source->mTitle));
	dest->setFileName(WTF::String(source->mFileName));
	dest->setFileSize(source->mFileSize);
	dest->setCreatedDate(source->mCreatedDate);
	MediaContent* mediaContent = dest->content();
	if (mediaContent == nullptr) {
		mediaContent = MediaContent::create();
		dest->setContent(mediaContent);
	}
	mediaContent->setUri(WTF::String(source->mContent->mUri));
	if (source->mContent->mBlobSize > 0) {
		blink::DOMArrayBuffer* array = DOMArrayBuffer::create(source->mContent->mBlob, source->mContent->mBlobSize * sizeof(uint8_t));
		NonThrowableExceptionState es;
		HeapVector<ArrayBufferOrArrayBufferViewOrBlobOrUSVString> vector;
		vector.append(ArrayBufferOrArrayBufferViewOrBlobOrUSVString::fromArrayBuffer(array));
		BlobPropertyBag bpb;
		bpb.setType(dest->type());
		Blob* blob = Blob::create(nullptr, vector, bpb, es);
		mediaContent->setBlob(blob);
	}
}


DeviceGalleryStatus* DeviceGalleryListener::lastData()
{
	if (mGalleryStatus != nullptr)
		return mGalleryStatus.get();
	else
		return nullptr;
}

} // namespace blink