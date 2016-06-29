// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/device_gallery/devicegallery_dispatcher.h"

#include "content/public/common/service_registry.h"
#include "content/public/renderer/render_thread.h"

namespace content {

DeviceGalleryDispatcher::DeviceGalleryDispatcher() {
	if (ServiceRegistry* registry = RenderThread::Get()->GetServiceRegistry()) {
		registry->ConnectToRemoteService(mojo::GetProxy(&manager_));
	}
}

DeviceGalleryDispatcher::~DeviceGalleryDispatcher() {
	manager_.reset();
	listener_ = nullptr;
	if (webDeviceGallerystatus.mMediaObjects != nullptr) {
		delete[] webDeviceGallerystatus.mMediaObjects;
		webDeviceGallerystatus.mMediaObjects = nullptr;
	}
}

void DeviceGalleryDispatcher::findMedia(blink::WebDeviceGalleryFindOptions* findOptions, blink::WebDeviceGalleryListener* mCallback) {
	listener_ = mCallback;
	device::MojoDeviceGalleryFindOptionsPtr options(device::MojoDeviceGalleryFindOptions::New());
	if (options != nullptr) {
		copyFindOptionsWebToMojo(findOptions, options.get());
	}
	manager_->findMedia(std::move(options), base::Bind(&DeviceGalleryDispatcher::resultCodeCallback, base::Unretained(this)));
}

void DeviceGalleryDispatcher::getMedia(blink::WebDeviceGalleryMediaObject* media, blink::WebDeviceGalleryListener* mCallback) {
	listener_ = mCallback;
	device::MojoDeviceGalleryMediaObjectPtr object(device::MojoDeviceGalleryMediaObject::New());
	if (media != nullptr) {
		copyMediaObjectWebToMojo(media, object.get());
	}
	manager_->getMedia(std::move(object), base::Bind(&DeviceGalleryDispatcher::resultCodeCallback, base::Unretained(this)));
}

void DeviceGalleryDispatcher::deleteMedia(blink::WebDeviceGalleryMediaObject* media, blink::WebDeviceGalleryListener* mCallback) {
	listener_ = mCallback;
	device::MojoDeviceGalleryMediaObjectPtr object(device::MojoDeviceGalleryMediaObject::New());
	if (media != nullptr) {
		copyMediaObjectWebToMojo(media, object.get());
	}
	manager_->deleteMedia(std::move(object), base::Bind(&DeviceGalleryDispatcher::resultCodeCallback, base::Unretained(this)));
}

void DeviceGalleryDispatcher::resultCodeCallback(device::DeviceGallery_ResultCodePtr resultCodePtr) {
	DCHECK(resultCodePtr);
	if (resultCodePtr != nullptr) {
		webDeviceGallerystatus.resultCode = resultCodePtr->resultCode;
		webDeviceGallerystatus.functionCode = resultCodePtr->functionCode;
		if (webDeviceGallerystatus.resultCode == blink::WebDeviceGalleryStatus::GalleryError::SUCCESS) {
			switch(resultCodePtr->functionCode) {
				case blink::WebDeviceGalleryStatus::function::FUNC_FIND_MEDIA: {
					webDeviceGallerystatus.mediaListSize = resultCodePtr->mediaListSize;
					webDeviceGallerystatus.mMediaObjects = new blink::WebDeviceGalleryMediaObject[resultCodePtr->mediaListSize]();
					for(int i = 0; i < resultCodePtr->mediaListSize; i++) {
						copyMediaObjectMojoToWeb(resultCodePtr->mediaList[i].get(), &webDeviceGallerystatus.mMediaObjects[i]);
					}
					break;
				}
				case blink::WebDeviceGalleryStatus::function::FUNC_GET_MEDIA: {
					webDeviceGallerystatus.mediaListSize = resultCodePtr->mediaListSize;
					webDeviceGallerystatus.mMediaObjects = new blink::WebDeviceGalleryMediaObject[resultCodePtr->mediaListSize]();
					for(int i = 0; i < resultCodePtr->mediaListSize; i++) {
						copyMediaObjectMojoToWeb(resultCodePtr->mediaList[i].get(), &webDeviceGallerystatus.mMediaObjects[i]);
					}
					break;
				}
				default: {
					break;
				}
			}
		}
	}
	if (listener_ != nullptr) {
		listener_->resultDeviceGallery(webDeviceGallerystatus);
	}
}

void DeviceGalleryDispatcher::copyFindOptionsWebToMojo(blink::WebDeviceGalleryFindOptions* source, device::MojoDeviceGalleryFindOptions* dest) {
	dest->mOperation = mojo::String(source->mOperation.utf8());
	dest->mMaxItem = source->mMaxItem;
	if (dest->mObject == nullptr) {
		dest->mObject = device::MojoDeviceGalleryMediaObject::New();
		copyMediaObjectWebToMojo(&source->mObject, dest->mObject.get());
	}
}

void DeviceGalleryDispatcher::copyMediaObjectWebToMojo(blink::WebDeviceGalleryMediaObject* source, device::MojoDeviceGalleryMediaObject* dest) {
	dest->mType = mojo::String(source->mType.utf8());
	dest->mDescription = mojo::String(source->mDescription.utf8());
	dest->mId = mojo::String(source->mId.utf8());
	dest->mTitle = mojo::String(source->mTitle.utf8());
	dest->mFileName = mojo::String(source->mFileName.utf8());
	dest->mFileSize = source->mFileSize;
	dest->mCreatedDate = source->mCreatedDate;
	dest->mContent = device::MojoDeviceGalleryMediaContent::New();
	if (source->mContent != nullptr && !source->mContent->mUri.isEmpty()) {
		dest->mContent.get()->mUri = mojo::String(source->mContent->mUri.utf8());
	} else {
		dest->mContent.get()->mUri = mojo::String("");
	}
	dest->mContent.get()->mBlob = mojo::Array<uint8_t>::New(0);
}

void DeviceGalleryDispatcher::copyMediaObjectMojoToWeb(device::MojoDeviceGalleryMediaObject* source, blink::WebDeviceGalleryMediaObject* dest) {
	dest->mType = blink::WebString::fromUTF8(source->mType.get());
	dest->mDescription = blink::WebString::fromUTF8(source->mDescription.get());
	dest->mId = blink::WebString::fromUTF8(source->mId.get());
	dest->mTitle = blink::WebString::fromUTF8(source->mTitle.get());
	dest->mFileName = blink::WebString::fromUTF8(source->mFileName.get());
	dest->mFileSize = source->mFileSize;
	dest->mCreatedDate = source->mCreatedDate;
	dest->mContent = new blink::WebDeviceGalleryMediaContent();
	dest->mContent->mUri = blink::WebString::fromUTF8(source->mContent->mUri.get());
	if (source->mContent->mBlobSize > 0) {
		dest->mContent->mBlob = new uint8_t[source->mContent->mBlobSize]();
		dest->mContent->mBlobSize = source->mContent->mBlobSize;
		for(int i = 0; i < source->mContent->mBlobSize; i++) {
			dest->mContent->mBlob[i] = (uint8_t)source->mContent->mBlob[i];
		}
	}
}

}  // namespace content
