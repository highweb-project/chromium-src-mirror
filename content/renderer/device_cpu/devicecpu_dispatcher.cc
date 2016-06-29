// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/device_cpu/devicecpu_dispatcher.h"

#include "content/public/common/service_registry.h"
#include "content/public/renderer/render_thread.h"

#include "base/threading/platform_thread.h"
#include "base/time/time.h"

namespace content {

DeviceCpuDispatcher::DeviceCpuDispatcher() {
	if (ServiceRegistry* registry = RenderThread::Get()->GetServiceRegistry()) {
		registry->ConnectToRemoteService(mojo::GetProxy(&manager_));
	}
}

DeviceCpuDispatcher::~DeviceCpuDispatcher() {
	runGetCpuLoad = false;
	manager_->stopCpuLoad();
	manager_.reset();
	listener_ = nullptr;
}

void DeviceCpuDispatcher::getDeviceCpuLoad(blink::WebDeviceCpuListener* mCallback) {
	if (listener_ == nullptr) {
		listener_ = mCallback;	
		runGetCpuLoad = true;
	}
	getCpuLoad();
}

void DeviceCpuDispatcher::getCpuLoad() {
	if (!runGetCpuLoad) {
		manager_->stopCpuLoad();
		listener_ = nullptr;
		return;
	}
	manager_->startCpuLoad();
	if (manager_) {
		manager_->getDeviceCpuLoad(base::Bind(&DeviceCpuDispatcher::resultCodeCallback, base::Unretained(this)));
	}
}

void DeviceCpuDispatcher::resultCodeCallback(device::DeviceCpu_ResultCodePtr resultCodePtr) {
	DCHECK(resultCodePtr);
	if (resultCodePtr != nullptr) {
		webDeviceCpustatus.functionCode = resultCodePtr->functionCode;
		webDeviceCpustatus.resultCode = resultCodePtr->resultCode;
		if (resultCodePtr->resultCode == blink::WebDeviceCpuStatus::status::SUCCESS) {
			switch(resultCodePtr->functionCode) {
				case blink::WebDeviceCpuStatus::function::FUNC_GET_CPU_LOAD: {
					webDeviceCpustatus.load = resultCodePtr->load;
					break;
				}
			}
		}
	}
	if (listener_ != nullptr) {
		listener_->resultDeviceCpu(webDeviceCpustatus);
		if (resultCodePtr->functionCode == blink::WebDeviceCpuStatus::function::FUNC_GET_CPU_LOAD
			&& resultCodePtr->resultCode == blink::WebDeviceCpuStatus::status::SUCCESS) {
				getCpuLoad();
		}
	}
}

}  // namespace content
