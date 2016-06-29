// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "DeviceCpuListener.h"
#include "DeviceCpu.h"
#include "DeviceCpuStatus.h"

#include "public/platform/modules/device_systeminformation/WebDeviceCpuStatus.h"
#include "public/platform/Platform.h"
#include "public/platform/WebPlatformEventType.h"
#include "base/logging.h"


namespace blink {

#define CPULOAD_ROUNDING(x, dig) floor((x) * pow(float(10), dig) + 0.5f) / pow(float(10), dig)

DeviceCpuListener* DeviceCpuListener::mListener;

DeviceCpuListener* DeviceCpuListener::instance(DeviceCpu* cpu)
{
	if (DeviceCpuListener::mListener == NULL) {
		DeviceCpuListener::mListener = new DeviceCpuListener(cpu);
	}
	return DeviceCpuListener::mListener;
}

DeviceCpuListener::DeviceCpuListener(DeviceCpu* cpu)
{
	mDeviceCpu = cpu;
}

DeviceCpuListener::~DeviceCpuListener()
{
	DeviceCpuListener::mListener = NULL;
	mDeviceCpu = NULL;
	mCpuStatus = NULL;
}

DEFINE_TRACE(DeviceCpuListener)
{
	visitor->trace(mDeviceCpu);
	visitor->trace(mCpuStatus);
}

void DeviceCpuListener::resultDeviceCpu(const WebDeviceCpuStatus& cpuStatus) {
	mCpuStatus = DeviceCpuStatus::create();
	mCpuStatus->setResultCode(cpuStatus.resultCode);
	mCpuStatus->setFunctionCode(cpuStatus.functionCode);
	switch(cpuStatus.functionCode) {
		case WebDeviceCpuStatus::function::FUNC_GET_CPU_LOAD: {
			mCpuStatus->setLoad(CPULOAD_ROUNDING(cpuStatus.load, 5));
			break;
		}
	}
	mDeviceCpu->resultCodeCallback();
}

DeviceCpuStatus* DeviceCpuListener::lastData()
{
	if (mCpuStatus != nullptr)
		return mCpuStatus;
	else
		return nullptr;
}

} // namespace blink