// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "modules/device_cpu/DeviceCpu.h"

#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/Platform.h"
#include "DeviceCpuStatus.h"
#include "DeviceCpuScriptCallback.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"

#include "platform/mojo/MojoHelper.h"
#include "public/platform/ServiceRegistry.h"


namespace blink {
#define CPULOAD_ROUNDING(x, dig) floor((x) * pow(float(10), dig) + 0.5f) / pow(float(10), dig)

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
	stopOnLoadCallback();
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

void DeviceCpu::loadInternal() {
	if (!deviceCpuManager.is_bound()) {
		Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&deviceCpuManager));
	}
	if (!isPending) {
		stopOnLoadCallback();
		return;
	}
	deviceCpuManager->startCpuLoad();
	deviceCpuManager->getDeviceCpuLoad(createBaseCallback(bind<device::blink::DeviceCpu_ResultCodePtr>(&DeviceCpu::OnLoadCallback, this)));
}

void DeviceCpu::OnLoadCallback(device::blink::DeviceCpu_ResultCodePtr result) {
	if (mLastLoadData == nullptr) {
		mLastLoadData = DeviceCpuStatus::create();
	}
	mLastLoadData->setFunctionCode(result->functionCode);
	mLastLoadData->setResultCode(result->resultCode);
	mLastLoadData->setLoad(CPULOAD_ROUNDING(result->load, 5));
	resultCodeCallback();
}

void DeviceCpu::resultCodeCallback() {
	Document* document = toDocument(getExecutionContext());
	if (document->activeDOMObjectsAreSuspended() || document->activeDOMObjectsAreStopped()) {
		stopOnLoadCallback();
		return;
	}

	DeviceCpuStatus* status = mLastLoadData;;
	//20160419-jphong
	if (status != nullptr) {
		if (status->getFunctionCode() == function::FUNC_GET_CPU_LOAD) {
			notifyCallback(status, nullptr);
		}
	} else {
		if (status->getFunctionCode() == function::FUNC_GET_CPU_LOAD) {
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
		functionData* data = d_functionData.first();
		if (status->resultCode() != ErrorCodeList::SUCCESS) {
			stopOnLoadCallback();
		} else {
			if (d_functionData.size() > 0) {
				d_functionData.removeFirst();
			}
			if (d_functionData.size() > 0 && d_functionData.first()->functionCode == function::FUNC_GET_CPU_LOAD) {
			} else {
				d_functionData.append(data);
			}
		}
	}
	continueFunction();
}

void DeviceCpu::notifyError(int errorCode, DeviceCpuScriptCallback* callback) {
	DeviceCpuStatus* status = DeviceCpuStatus::create();
	status->setResultCode(errorCode);
	notifyCallback(status, callback);
}

void DeviceCpu::continueFunction() {
	if (!ViewPermissionState && d_functionData.size() > 0) {
		requestPermission();
		return;
	}
	if (d_functionData.size() > 0) {
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_GET_CPU_LOAD : {
				loadInternal();
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
	stopOnLoadCallback();
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
	stopOnLoadCallback();
}

bool DeviceCpu::hasPendingActivity() const
{
	return isPending;
}

void DeviceCpu::stopOnLoadCallback() {
	if (deviceCpuManager.is_bound()) {
		deviceCpuManager.reset();
	}
	mLastLoadData = nullptr;
	d_functionData.clear();
}

DEFINE_TRACE(DeviceCpu)
{
	visitor->trace(callbackList);
	visitor->trace(mLastLoadData);
	// visitor->trace(d_functionData);
	//visitor->trace(callbackList);
	//GarbageCollectedFinalized<DeviceCpu>::trace(visitor);
	EventTargetWithInlineData::trace(visitor);
	ActiveDOMObject::trace(visitor);
}

} // namespace blink
