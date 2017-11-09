// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "platform/wtf/build_config.h"
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
#include "services/device/public/interfaces/constants.mojom-blink.h"
#include "services/service_manager/public/cpp/connector.h"


namespace blink {
#define CPULOAD_ROUNDING(x, dig) floor((x) * pow(float(10), dig) + 0.5f) / pow(float(10), dig)

DeviceCpu::DeviceCpu(Document& document)
	: SuspendableObject((ExecutionContext*)&document)
{
	mClient = DeviceApiPermissionController::From(*document.GetFrame())->client();
	mOrigin = document.Url().StrippedForUseAsReferrer();
	mClient->SetOrigin(mOrigin.Latin1().data());
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
	if (!callbackList.Contains(callback)) {
		callbackList.insert(callback);
	}
	isPending = true;
	functionData* data = new functionData(function::FUNC_GET_CPU_LOAD);
	d_functionData.push_back(data);
	data = nullptr;

	if (d_functionData.size() == 1) {
		continueFunction();
	}
}

void DeviceCpu::loadInternal() {
	if (!deviceCpuManager.is_bound()) {
		Platform::Current()->GetConnector()->BindInterface(
      device::mojom::blink::kServiceName, mojo::MakeRequest(&deviceCpuManager));
	}
	if (!isPending) {
		stopOnLoadCallback();
		return;
	}
	deviceCpuManager->startCpuLoad();
	deviceCpuManager->getDeviceCpuLoad(
		ConvertToBaseCallback(WTF::Bind(&DeviceCpu::OnLoadCallback, WrapPersistent(this))));
}

void DeviceCpu::OnLoadCallback(device::mojom::blink::DeviceCpu_ResultCodePtr result) {
	if (mLastLoadData == nullptr) {
		mLastLoadData = DeviceCpuStatus::create();
	}
	mLastLoadData->setFunctionCode(result->functionCode);
	mLastLoadData->setResultCode(result->resultCode);
	mLastLoadData->setLoad(CPULOAD_ROUNDING(result->load, 5));
	resultCodeCallback();
}

void DeviceCpu::resultCodeCallback() {
	ExecutionContext* context = GetExecutionContext();
	if (context->IsContextSuspended() || context->IsContextDestroyed()) {
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
			notifyError(ErrorCodeList::kFailure, nullptr);
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
		functionData* data = d_functionData.front();
		if (status->resultCode() != ErrorCodeList::kSuccess) {
			stopOnLoadCallback();
		} else {
			if (d_functionData.size() > 0) {
				d_functionData.pop_front();
			}
			if (d_functionData.size() > 0 && d_functionData.front()->functionCode == function::FUNC_GET_CPU_LOAD) {
			} else {
				d_functionData.push_back(data);
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
		switch(d_functionData.front()->functionCode) {
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
		notifyError(ErrorCodeList::kNotEnabledPermission, nullptr);
	}
}

void DeviceCpu::Suspend()
{
	isPending = false;
	stopOnLoadCallback();
}

void DeviceCpu::Resume()
{
	isPending = true;
	if (callbackList.size() > 0) {
		functionData* data = new functionData(function::FUNC_GET_CPU_LOAD);
		d_functionData.push_back(data);
		data = nullptr;

		if (d_functionData.size() == 1) {
			continueFunction();
		}
	}
}

void DeviceCpu::ContextDestroyed(ExecutionContext* context)
{
	isPending = false;
	stopOnLoadCallback();
}

bool DeviceCpu::HasPendingActivity() const
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
	visitor->Trace(callbackList);
	visitor->Trace(mLastLoadData);
	// visitor->trace(d_functionData);
	//visitor->trace(callbackList);
	//GarbageCollectedFinalized<DeviceCpu>::trace(visitor);
	EventTargetWithInlineData::Trace(visitor);
	SuspendableObject::Trace(visitor);
}

} // namespace blink
