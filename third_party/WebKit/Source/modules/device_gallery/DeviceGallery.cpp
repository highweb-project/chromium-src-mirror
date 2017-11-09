// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "platform/wtf/build_config.h"
#include "modules/device_gallery/DeviceGallery.h"

#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"
#include "modules/device_gallery/GalleryMediaObject.h"
#include "GallerySuccessCallback.h"
#include "GalleryErrorCallback.h"
#include "DeviceGalleryStatus.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"

#include "platform/mojo/MojoHelper.h"
#include "services/device/public/interfaces/constants.mojom-blink.h"
#include "services/service_manager/public/cpp/connector.h"
#include "core/fileapi/BlobPropertyBag.h"
#include "core/fileapi/Blob.h"
#include "core/typed_arrays/DOMArrayBuffer.h"
#include "bindings/core/v8/ExceptionState.h"

namespace blink {

DeviceGallery::DeviceGallery(LocalFrame& frame)
	: mClient(DeviceApiPermissionController::From(frame)->client())
{
	mOrigin = frame.GetDocument()->Url().StrippedForUseAsReferrer();
	mClient->SetOrigin(mOrigin.Latin1().data());
	d_functionData.clear();
}

DeviceGallery::~DeviceGallery()
{
	if (galleryManager.is_bound()) {
		galleryManager.reset();
	}
}

void DeviceGallery::findMedia(GalleryFindOptions findOptions, GallerySuccessCallback* successCallback, GalleryErrorCallback* errorCallback){
	functionData* data = new functionData(function::FUNC_FIND_MEDIA);
	mSuccessCallbackList.push_back(successCallback);
	mErrorCallbackList.push_back(errorCallback);
	data->option = findOptions;
	d_functionData.push_back(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceGallery::findMediaInternal() {
	if (!galleryManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&galleryManager));
	}

	device::mojom::blink::MojoDeviceGalleryFindOptionsPtr options(device::mojom::blink::MojoDeviceGalleryFindOptions::New());
	convertScriptFindOptionToMojo(options.get(), d_functionData.front()->option);

	galleryManager->findMedia(std::move(options),
		ConvertToBaseCallback(WTF::Bind(&DeviceGallery::mojoResultCallback, WrapPersistent(this))));
}

void DeviceGallery::getMedia(GalleryMediaObject* media, GallerySuccessCallback* successCallback, GalleryErrorCallback* errorCallback) {
	functionData* data = new functionData(function::FUNC_GET_MEDIA);
	mSuccessCallbackList.push_back(successCallback);
	mErrorCallbackList.push_back(errorCallback);
	data->object = media;
	d_functionData.push_back(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceGallery::getMediaInternal() {
	if (!galleryManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&galleryManager));
	}

	device::mojom::blink::MojoDeviceGalleryMediaObjectPtr mojoObject(device::mojom::blink::MojoDeviceGalleryMediaObject::New());
	convertScriptMediaObjectToMojo(mojoObject.get(), d_functionData.front()->object);

	galleryManager->getMedia(std::move(mojoObject),
	ConvertToBaseCallback(WTF::Bind(&DeviceGallery::mojoResultCallback, WrapPersistent(this))));
}

void DeviceGallery::deleteMedia(GalleryMediaObject* media, GalleryErrorCallback* errorCallback) {
	functionData* data = new functionData(function::FUNC_DELETE_MEDIA);
	mErrorCallbackList.push_back(errorCallback);
	data->object = media;
	d_functionData.push_back(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceGallery::deleteMediaInternal() {
	if (!galleryManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
			device::mojom::blink::kServiceName, mojo::MakeRequest(&galleryManager));
	}

	device::mojom::blink::MojoDeviceGalleryMediaObjectPtr mojoObject(device::mojom::blink::MojoDeviceGalleryMediaObject::New());
	convertScriptMediaObjectToMojo(mojoObject.get(), d_functionData.front()->object);

	galleryManager->deleteMedia(std::move(mojoObject),
	ConvertToBaseCallback(WTF::Bind(&DeviceGallery::mojoResultCallback, WrapPersistent(this))));
}

void DeviceGallery::notifyCallback(DeviceGalleryStatus* status, GallerySuccessCallback* callback) {
	if (callback != NULL) {
		callback->handleEvent(status->mediaObjectList());
		callback = nullptr;
	}
	if (status->getFunctionCode() == function::FUNC_DELETE_MEDIA) {
		if(mErrorCallbackList.size() > 0)
			mErrorCallbackList.erase(0);
	} else {
		if(mSuccessCallbackList.size() > 0)
			mSuccessCallbackList.erase(0);
		if(mErrorCallbackList.size() > 0)
			mErrorCallbackList.erase(0);
	}
	if (d_functionData.size() > 0) {
		d_functionData.pop_front();
	}
	continueFunction();
}

void DeviceGallery::notifyError(int errorCode, GalleryErrorCallback* callback) {
	if (callback != NULL) {
		callback->handleEvent(errorCode);
		callback = nullptr;
	}
	if (d_functionData.size() > 0) {
		d_functionData.pop_front();
	}
	if(mSuccessCallbackList.size() > 0)
		mSuccessCallbackList.erase(0);
	if(mErrorCallbackList.size() > 0)
		mErrorCallbackList.erase(0);
	continueFunction();
}

void DeviceGallery::continueFunction() {
	if (d_functionData.size() > 0) {
		switch(d_functionData.front()->functionCode) {
			case function::FUNC_FIND_MEDIA : {
				if (!FindPermissionState) {
					requestPermission(PermissionOptType::FIND);
					return;
				}
				findMediaInternal();
				break;
			}
			case function::FUNC_GET_MEDIA : {
				if (!ViewPermissionState) {
					requestPermission(PermissionOptType::VIEW);
					return;
				}
				getMediaInternal();
				break;
			}
			case function::FUNC_DELETE_MEDIA : {
				if (!DeletePermissionState) {
					requestPermission(PermissionOptType::DELETE);
					return;
				}
				deleteMediaInternal();
				break;
			}
			default: {
				break;
			}
		}
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
	if (d_functionData.front()->functionCode == function::FUNC_FIND_MEDIA) {
		if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK) {
			FindPermissionState = true;
		}
		permissionCheck(PermissionOptType::FIND);
	} else if (d_functionData.front()->functionCode == function::FUNC_GET_MEDIA) {
		if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK) {
			ViewPermissionState = true;
		}
		permissionCheck(PermissionOptType::VIEW);
	} else if (d_functionData.front()->functionCode == function::FUNC_DELETE_MEDIA) {
		if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK) {
			DeletePermissionState = true;
		}
		permissionCheck(PermissionOptType::DELETE);
	}
}

void DeviceGallery::permissionCheck(PermissionOptType type) {
	switch(type) {
		case PermissionOptType::FIND: {
			if (FindPermissionState) {
				continueFunction();
			} else {
				notifyError(GalleryError::kNotEnabledPermission, mErrorCallbackList.at(0).Get());
			}
			break;
		}
		case PermissionOptType::VIEW: {
			if (ViewPermissionState) {
				continueFunction();
			} else {
				notifyError(GalleryError::kNotEnabledPermission, mErrorCallbackList.at(0).Get());
			}
			break;
		}
		case PermissionOptType::DELETE: {
			if (DeletePermissionState) {
				continueFunction();
			} else {
				notifyError(GalleryError::kNotEnabledPermission, mErrorCallbackList.at(0).Get());
			}
			break;
		}
		default: {
			notifyError(GalleryError::kNotEnabledPermission, mErrorCallbackList.at(0).Get());
			break;
		}
	}
}

void DeviceGallery::mojoResultCallback(device::mojom::blink::DeviceGallery_ResultCodePtr result) {
	DLOG(INFO) << "mojoResultCallback " << result->resultCode << ", " << result->functionCode;
	DCHECK(result);
	if (!result.is_null()) {
		if (result->resultCode == GalleryError::kSuccess && result->functionCode != function::FUNC_DELETE_MEDIA) {
			DeviceGalleryStatus* status = DeviceGalleryStatus::create();
			status->setFunctionCode(result->functionCode);
			status->setResultCode(result->resultCode);
			if (result->mediaList.has_value()) {
				for(size_t listSize = 0; listSize < result->mediaList.value().size(); listSize++) {
					GalleryMediaObject* object = GalleryMediaObject::Create();
					convertMojoToScriptObject(object, result->mediaList.value()[listSize].get());
					status->mediaObjectList().push_back(object);
				}
			}
			notifyCallback(status, mSuccessCallbackList.at(0).Get());
		} else {
			notifyError(result->resultCode, mErrorCallbackList.at(0).Get());
		}
	} else {
		notifyError(GalleryError::kUnknownError, mErrorCallbackList.at(0).Get());
	}
}

void DeviceGallery::convertScriptFindOptionToMojo(
	device::mojom::blink::MojoDeviceGalleryFindOptions* mojoFindOption, GalleryFindOptions& option) {

	mojoFindOption->mOperation = option.operation();
	if (option.hasMaxItem()) {
		mojoFindOption->mMaxItem = option.maxItem();
	}
	else {
		mojoFindOption->mMaxItem = 0;
	}
	mojoFindOption->mObject = device::mojom::blink::MojoDeviceGalleryMediaObject::New();
	if (option.hasFindObject()) {
		convertScriptMediaObjectToMojo(mojoFindOption->mObject.get(), option.findObject());
	}
}

void DeviceGallery::convertScriptMediaObjectToMojo(
	device::mojom::blink::MojoDeviceGalleryMediaObject* mojoMediaObject, GalleryMediaObject* object) {
	mojoMediaObject->mType = object->type();
	mojoMediaObject->mDescription = object->description();
	mojoMediaObject->mId = object->id();
	mojoMediaObject->mTitle = object->title();
	mojoMediaObject->mFileName = object->fileName();
	mojoMediaObject->mFileSize = object->fileSize();
	mojoMediaObject->mCreatedDate = object->createdDate();
	mojoMediaObject->mContent = device::mojom::blink::MojoDeviceGalleryMediaContent::New();
	if (object->content() != nullptr && !object->content()->uri().IsEmpty()) {
		mojoMediaObject->mContent.get()->mUri = object->content()->uri();
	} else {
		mojoMediaObject->mContent.get()->mUri = String("");
	}
	mojoMediaObject->mContent.get()->mBlob = WTF::Vector<uint8_t>(0);
}

void DeviceGallery::convertMojoToScriptObject(GalleryMediaObject* object,
	device::mojom::blink::MojoDeviceGalleryMediaObject* mojoObject) {
	object->setType(mojoObject->mType);
	object->setDescription(mojoObject->mDescription);
	object->setId(mojoObject->mId);
	object->setTitle(mojoObject->mTitle);
	object->setFileName(mojoObject->mFileName);
	object->setFileSize(mojoObject->mFileSize);
	object->setCreatedDate(mojoObject->mCreatedDate);
	if (mojoObject->mContent.get() != nullptr) {
		MediaContent* content = MediaContent::Create();
		content->setUri(mojoObject->mContent.get()->mUri);
		if (mojoObject->mContent.get()->mBlobSize > 0) {
			WTF::Vector<uint8_t> data = mojoObject->mContent.get()->mBlob;
			Blob* blob = Blob::Create(data.data(), data.size() * sizeof(uint8_t), object->type());
			content->setBlob(blob);
		}
		object->setContent(content);
	}
}

DEFINE_TRACE(DeviceGallery)
{
	visitor->Trace(d_functionData);
	visitor->Trace(mSuccessCallbackList);
	visitor->Trace(mErrorCallbackList);
}

} // namespace blink
