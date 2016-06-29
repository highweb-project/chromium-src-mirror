// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "modules/device_gallery/DeviceGallery.h"

// #include "base/basictypes.h"
#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"
#include "public/platform/WebString.h"
#include "DeviceGalleryListener.h"
#include "modules/device_gallery/GalleryMediaObject.h"
#include "GallerySuccessCallback.h"
#include "GalleryErrorCallback.h"
#include "DeviceGalleryStatus.h"
#include "public/platform/modules/device_gallery/WebDeviceGalleryStruct.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
namespace blink {

DeviceGallery::DeviceGallery(LocalFrame& frame)
	: mClient(DeviceApiPermissionController::from(frame)->client())
{
	mOrigin = frame.document()->url().strippedForUseAsReferrer();
	mClient->SetOrigin(mOrigin.latin1().data());
	d_functionData.clear();
}

DeviceGallery::~DeviceGallery()
{
}

void DeviceGallery::findMedia(GalleryFindOptions findOptions, GallerySuccessCallback* successCallback, GalleryErrorCallback* errorCallback){
	functionData* data = new functionData(function::FUNC_FIND_MEDIA);
	//data->successCallback = successCallback;
	//data->errorCallback = errorCallback;
	mSuccessCallbackList.append(successCallback);
	mErrorCallbackList.append(errorCallback);
	data->option = findOptions;
	d_functionData.append(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceGallery::getMedia(GalleryMediaObject* media, GallerySuccessCallback* successCallback, GalleryErrorCallback* errorCallback) {
	functionData* data = new functionData(function::FUNC_GET_MEDIA);
	//data->successCallback = successCallback;
	//data->errorCallback = errorCallback;
	mSuccessCallbackList.append(successCallback);
	mErrorCallbackList.append(errorCallback);
	data->object = media;
	d_functionData.append(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceGallery::deleteMedia(GalleryMediaObject* media, GalleryErrorCallback* errorCallback) {
	functionData* data = new functionData(function::FUNC_DELETE_MEDIA);
	//data->errorCallback = errorCallback;
	mErrorCallbackList.append(errorCallback);
	data->object = media;
	d_functionData.append(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceGallery::resultCodeCallback() {
	DeviceGalleryStatus* status = nullptr;
	if (mCallback != nullptr) {
		status = mCallback->lastData();
		mCallback = nullptr;
	}

	if (status != nullptr) {
		if (status->getResultCode() == GalleryError::SUCCESS && d_functionData.first()->functionCode != function::FUNC_DELETE_MEDIA) {
			notifyCallback(status, mSuccessCallbackList.at(0).get());
		} else {
			notifyError(status->getResultCode(), mErrorCallbackList.at(0).get());
		}
	} else {
		notifyError(GalleryError::UNKNOWN_ERROR, mErrorCallbackList.at(0).get());
	}
}

void DeviceGallery::notifyCallback(DeviceGalleryStatus* status, GallerySuccessCallback* callback) {
	if (callback != NULL) {
		callback->handleEvent(status->mediaObjectList());
		callback = nullptr;
	}
	Platform::current()->resetDeviceGalleryDispatcher();
	if (status->getFunctionCode() == function::FUNC_DELETE_MEDIA) {
		if(mErrorCallbackList.size() > 0)
			mErrorCallbackList.remove(0);
	} else {
		if(mSuccessCallbackList.size() > 0)
			mSuccessCallbackList.remove(0);
		if(mErrorCallbackList.size() > 0)
			mErrorCallbackList.remove(0);
	}
	if (d_functionData.size() > 0) {
		d_functionData.removeFirst();
	}
	continueFunction();
}

void DeviceGallery::notifyError(int errorCode, GalleryErrorCallback* callback) {
	if (callback != NULL) {
		callback->handleEvent(errorCode);
		callback = nullptr;
	}
	Platform::current()->resetDeviceGalleryDispatcher();
	if (d_functionData.size() > 0) {
		d_functionData.removeFirst();
	}
	if(mSuccessCallbackList.size() > 0)
		mSuccessCallbackList.remove(0);
	if(mErrorCallbackList.size() > 0)
		mErrorCallbackList.remove(0);
	continueFunction();
}

void DeviceGallery::continueFunction() {
	if (d_functionData.size() > 0) {
		if (mCallback == NULL) {
			mCallback = DeviceGalleryListener::instance(this);
		}
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_FIND_MEDIA : {
				if (!FindPermissionState) {
					requestPermission(PermissionOptType::FIND);
					return;
				}
				WebDeviceGalleryFindOptions* webFindOptions = new WebDeviceGalleryFindOptions();
				copyFindOptions(d_functionData.first()->option, webFindOptions);
				Platform::current()->findMedia(webFindOptions, mCallback);
				break;
			}
			case function::FUNC_GET_MEDIA : {
				if (!ViewPermissionState) {
					requestPermission(PermissionOptType::VIEW);
					return;
				}
				WebDeviceGalleryMediaObject* webMediaObject = new WebDeviceGalleryMediaObject();
				copyMediaObjectNotBlob(d_functionData.first()->object, webMediaObject);
				Platform::current()->getMedia(webMediaObject, mCallback);
				break;
			}
			case function::FUNC_DELETE_MEDIA : {
				if (!DeletePermissionState) {
					requestPermission(PermissionOptType::DELETE);
					return;
				}
				WebDeviceGalleryMediaObject* webMediaObject = new WebDeviceGalleryMediaObject();
				copyMediaObjectNotBlob(d_functionData.first()->object, webMediaObject);
				Platform::current()->deleteMedia(webMediaObject, mCallback);
				break;
			}
			default: {
				break;
			}
		}
	}
}

void DeviceGallery::copyFindOptions(GalleryFindOptions& source, WebDeviceGalleryFindOptions* dest) {
	dest->mOperation = source.operation();
	if (source.hasMaxItem()) {
		dest->mMaxItem = source.maxItem();
	}
	else {
		dest->mMaxItem = 0;
	}
	if (source.hasFindObject()) {
		copyMediaObjectNotBlob(source.findObject(), &dest->mObject);
	}
}

void DeviceGallery::copyMediaObjectNotBlob(GalleryMediaObject* source, WebDeviceGalleryMediaObject* dest) {
	if (source == nullptr || dest == nullptr) {
		return;
	}
	dest->mType = WebString(source->type());
	dest->mDescription = WebString(source->description());
	dest->mId = WebString(source->id());
	dest->mTitle = WebString(source->title());
	dest->mFileName = WebString(source->fileName());
	dest->mFileSize = source->fileSize();
	dest->mCreatedDate = source->createdDate();
	if (source->content() != nullptr) {
		dest->mContent = new WebDeviceGalleryMediaContent();
		dest->mContent->mUri = source->content()->uri();
	}
}

void DeviceGallery::requestPermission(PermissionOptType type) {
	switch(type) {
		case PermissionOptType::FIND : {
			if(mClient) {
				mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
				PermissionAPIType::GALLERY,
				PermissionOptType::FIND,
				base::Bind(&DeviceGallery::onPermissionChecked, base::Unretained(this))));
			}
			break;
		}
		case PermissionOptType::VIEW : {
			if(mClient) {
				mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
				PermissionAPIType::GALLERY,
				PermissionOptType::VIEW,
				base::Bind(&DeviceGallery::onPermissionChecked, base::Unretained(this))));
			}
			break;
		}
		case PermissionOptType::DELETE : {
			if(mClient) {
				mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
				PermissionAPIType::GALLERY,
				PermissionOptType::DELETE,
				base::Bind(&DeviceGallery::onPermissionChecked, base::Unretained(this))));
			}
			break;
		}
		default : {
			break;
		}
	}
}

void DeviceGallery::onPermissionChecked(PermissionResult result)
{
	if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK &&
		d_functionData.first()->functionCode == function::FUNC_FIND_MEDIA) {
		FindPermissionState = true;
		permissionCheck(PermissionOptType::FIND);
	} else if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK &&
		d_functionData.first()->functionCode == function::FUNC_GET_MEDIA) {
		ViewPermissionState = true;
		permissionCheck(PermissionOptType::VIEW);
	} else if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK &&
		d_functionData.first()->functionCode == function::FUNC_DELETE_MEDIA) {
		DeletePermissionState = true;
		permissionCheck(PermissionOptType::DELETE);
	}
}

void DeviceGallery::permissionCheck(PermissionOptType type) {
	switch(type) {
		case PermissionOptType::FIND: {
			if (FindPermissionState) {
				continueFunction();
			} else {
				notifyError(GalleryError::NOT_ENABLED_PERMISSION, mErrorCallbackList.at(0).get());
			}
			break;
		}
		case PermissionOptType::VIEW: {
			if (ViewPermissionState) {
				continueFunction();
			} else {
				notifyError(GalleryError::NOT_ENABLED_PERMISSION, mErrorCallbackList.at(0).get());
			}
			break;
		}
		case PermissionOptType::DELETE: {
			if (DeletePermissionState) {
				continueFunction();
			} else {
				notifyError(GalleryError::NOT_ENABLED_PERMISSION, mErrorCallbackList.at(0).get());
			}
			break;
		}
		default: {
			notifyError(GalleryError::NOT_ENABLED_PERMISSION, mErrorCallbackList.at(0).get());
			break;
		}
	}
}

DEFINE_TRACE(DeviceGallery)
{
	visitor->trace(mCallback);
	visitor->trace(d_functionData);
	visitor->trace(mSuccessCallbackList);
	visitor->trace(mErrorCallbackList);
}

} // namespace blink
