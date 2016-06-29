// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "modules/device_cpu/DeviceCpu.h"

// #include "base/basictypes.h"
#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"
#include "public/platform/WebString.h"
#include "DeviceCpuListener.h"
#include "DeviceCpuStatus.h"
#include "DeviceCpuScriptCallback.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
namespace blink {

DeviceCpu::DeviceCpu(Document& document)
	: ActiveScriptWrappable(this), ActiveDOMObject((ExecutionContext*)&document)
{
	mClient = DeviceApiPermissionController::from(*document.frame())->client();
	mOrigin = document.url().strippedForUseAsReferrer();
	mClient->SetOrigin(mOrigin.latin1().data());
	//20160419-jphong
	d_functionData.clear();
	//callbackList.clear();
}

DeviceCpu::~DeviceCpu()
{
	mCallback = NULL;
	d_functionData.clear();
	//20160419-jphong
	callbackList.clear();
}

void DeviceCpu::load(DeviceCpuScriptCallback* passCallback) {
	DeviceCpuScriptCallback* callback = passCallback;
	if (!callbackList.contains(callback)) {
		callbackList.add(callback);
	}
	isPending = true;
	functionData* data = new functionData(function::FUNC_GET_CPU_LOAD);
	d_functionData.append(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceCpu::resultCodeCallback() {
	Document* document = toDocument(getExecutionContext());
	if (document->activeDOMObjectsAreSuspended() || document->activeDOMObjectsAreStopped()) {
		Platform::current()->resetDeviceCpuDispatcher();
		mCallback = NULL;
		return;
	}

	DeviceCpuStatus* status = nullptr;
	if (mCallback != nullptr) {
		status = mCallback->lastData();
	}
	//20160419-jphong
	if (status != nullptr) {
		if (status->getFunctionCode() == function::FUNC_GET_CPU_LOAD) {
			notifyCallback(status, nullptr);
		}
		else {
			mCallback = nullptr;
			notifyCallback(status, nullptr);
		}
	} else {
		if (status->getFunctionCode() == function::FUNC_GET_CPU_LOAD) {
			notifyError(ErrorCodeList::FAILURE, nullptr);
		}
		else {
			notifyError(ErrorCodeList::FAILURE, nullptr);
		}
	}
}

void DeviceCpu::notifyCallback(DeviceCpuStatus* status, DeviceCpuScriptCallback* callback) {
	//20160419-jphong
	if (status->getFunctionCode() == function::FUNC_GET_CPU_LOAD) {
		if (callbackList.size() > 0) {
		    CpuCallbackListIterator itEnd = callbackList.rend();

		    for (CpuCallbackListIterator it = callbackList.rbegin(); it != itEnd; ++it) {
		        DeviceCpuScriptCallback* callback = *it;
		        callback->handleEvent(status);
		    }
		}
		if (d_functionData.size() > 0 && d_functionData.first()->functionCode == function::FUNC_GET_CPU_LOAD) {
			d_functionData.removeFirst();
		}
		if (status->resultCode() != ErrorCodeList::SUCCESS) {
			mCallback = NULL;
			Platform::current()->resetDeviceCpuDispatcher();
		}
		return;
	}
	else {
		if (callback != NULL) {
			callback->handleEvent(status);
			callback = nullptr;
		}
	}

	if (d_functionData.size() > 0) {
		d_functionData.removeFirst();
	}
	continueFunction();
}

void DeviceCpu::notifyError(int errorCode, DeviceCpuScriptCallback* callback) {
	DeviceCpuStatus* status = DeviceCpuStatus::create();
	status->setResultCode(errorCode);
	notifyCallback(status, callback);
}

void DeviceCpu::continueFunction() {
	if (!ViewPermissionState) {
		requestPermission();
		return;
	}
	if (d_functionData.size() > 0) {
		if (mCallback == NULL) {
			mCallback = DeviceCpuListener::instance(this);
		}
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_GET_CPU_LOAD : {
				Platform::current()->getDeviceCpuLoad(mCallback);
			}
			default: {
				break;
			}
		}
	}
}

void DeviceCpu::requestPermission() {
	if(mClient) {
		mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
				PermissionAPIType::SYSTEM_INFORMATION,
				PermissionOptType::VIEW,
				base::Bind(&DeviceCpu::onPermissionChecked, base::Unretained(this))));
	}
}

void DeviceCpu::onPermissionChecked(PermissionResult result)
{
	if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK) {
		ViewPermissionState = true;
	}
	if (ViewPermissionState) {
		continueFunction();
	} else {
		notifyError(ErrorCodeList::NOT_ENABLED_PERMISSION, nullptr);
	}
}

void DeviceCpu::suspend()
{
	isPending = false;
	Platform::current()->resetDeviceCpuDispatcher();
	mCallback = NULL;
}

void DeviceCpu::resume()
{
	isPending = true;
	if (callbackList.size() > 0) {
		functionData* data = new functionData(function::FUNC_GET_CPU_LOAD);
		d_functionData.append(data);
		data = nullptr;

		if (d_functionData.size() == 1) {
			continueFunction();
		}
	}
}

void DeviceCpu::stop()
//void DeviceCpu::contextDestroyed()
{
	isPending = false;
	Platform::current()->resetDeviceCpuDispatcher();
	mCallback = NULL;
	d_functionData.clear();
}

bool DeviceCpu::hasPendingActivity() const
{
	return isPending;
}

DEFINE_TRACE(DeviceCpu)
{
	visitor->trace(mCallback);
	visitor->trace(callbackList);
	//visitor->trace(d_functionData);
	//visitor->trace(callbackList);
	//GarbageCollectedFinalized<DeviceCpu>::trace(visitor);
	EventTargetWithInlineData::trace(visitor);
	ActiveDOMObject::trace(visitor);	
}

} // namespace blink
