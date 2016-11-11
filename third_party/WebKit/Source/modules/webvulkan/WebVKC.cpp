// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "WebVKC.h"

#include "core/dom/ExecutionContext.h"
#include "bindings/core/v8/ExceptionState.h"
#include "WebVKCOperationHandler.h"
#include "WebVKCDevice.h"
#include "WebVKCException.h"
#include "third_party/WebKit/public/platform/Platform.h"

#include <wtf/Vector.h>


gpu::GpuChannelHost* webvkc_channel_ = NULL;

namespace blink {

WebVKC::WebVKC(ExecutionContext* context)
{
}

WebVKC::~WebVKC() {
	mDeviceList.clear();
	vkcInstance = 0;
}

void WebVKC::initialize(ExceptionState& ec) {
	webvkc_channel_ = Platform::current()->createWebVKCGPUChannelContext();
	mOperationHandler = adoptPtr(new WebVKCOperationHandler());
	mOperationHandler->startHandling();

	std::string applicationName = "WebVKC";
	std::string engineName = "WebVKCEngine";
	uint32_t applicationVersion = 1;
	uint32_t engineVersion = 1;
	uint32_t apiVersion = VK_MAKE_VERSION(1, 0, 0);

	std::vector<std::string> enabledLayerNames;
	std::vector<std::string> enabledExtensionNames;

	VKCResult err = webvkc_createInstance(webvkc_channel_, applicationName, engineName, applicationVersion,
		engineVersion, apiVersion, enabledLayerNames, enabledExtensionNames, &vkcInstance);

	if (err != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(err, ec);
		return;
	}

	err = webvkc_enumeratePhysicalDeviceCount(webvkc_channel_, &vkcInstance, &mPhysicalDeviceCount, &mPhysicalDeviceList);

	if (err != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(err, ec);
		return;
	}

	VKCLOG(INFO) << "createInstance result : " << vkcInstance << ", " << err << ", " << mPhysicalDeviceCount << ", " << mPhysicalDeviceList;
	initialized = true;
}

uint32_t WebVKC::getPhysicalDeviceCount(ExceptionState& ec) {
	if (!initialized) {
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, ec);
		VKCLOG(INFO) << "VK_NOT_READY";
	}

	return mPhysicalDeviceCount;
}

HeapVector<Member<WebVKCDevice>> WebVKC::getDevices(ExceptionState& ec) {
	if (!initialized) {
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, ec);
		VKCLOG(INFO) << "VK_NOT_READY";
	}
	return mDeviceList;
}

Member<WebVKCDevice> WebVKC::createDevice(VKCuint vdIndex, ExceptionState& ec) {
	VKCLOG(INFO) << "createDevice : " << vdIndex;
	if (vdIndex >= mPhysicalDeviceCount) {
		VKCLOG(INFO) << "out of logical device index";

		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, ec);
		return nullptr;
	}
	WebVKCDevice* device = WebVKCDevice::create(vdIndex, mPhysicalDeviceList, this, ec);
	mDeviceList.append(device);
	return device;
}

void WebVKC::releaseAll(ExceptionState& ec) {
	if (!initialized) {
		VKCLOG(INFO) << "already release";
		return;
	}

	if (releasing) {
		VKCLOG(INFO) << "already releasing";
		return;
	}
	releasing = true;
	initialized = false;

	if (mDeviceList.size() > 0) {
		for(WebVKCDevice* device : mDeviceList) {
			device->release(ec);
		}
	}
	mDeviceList.clear();

	webvkc_destroyPhysicalDevice(webvkc_channel_, &mPhysicalDeviceList);
	mPhysicalDeviceCount = 0;
	mPhysicalDeviceList = 0;

	webvkc_destroyInstance(webvkc_channel_, &vkcInstance);
	vkcInstance = 0;

	mOperationHandler->finishHandling();
}

void WebVKC::removeDevice(Member<WebVKCDevice> device) {
	if (!releasing) {
		size_t index = mDeviceList.find(device);
		if (index != kNotFound) {
			mDeviceList.remove(index);
		}
	}
}

void WebVKC::startHandling()
{
	if(!mOperationHandler->canShareOperation()) {
		mOperationHandler->startHandling();
	}
}

void WebVKC::setOperationParameter(WebVKC_Operation_Base* paramPtr)
{
	mOperationHandler->setOperationParameter(paramPtr);
}

void WebVKC::setOperationData(void* dataPtr, size_t sizeInBytes)
{
	mOperationHandler->setOperationData(dataPtr, sizeInBytes);
}

void WebVKC::getOperationResult(WebVKC_Result_Base* resultPtr)
{
	mOperationHandler->getOperationResult(resultPtr);
}

void WebVKC::sendOperationSignal(int operation)
{
	mOperationHandler->sendOperationSignal(operation);
}

void WebVKC::getOperationResultData(void* resultDataPtr, size_t sizeInBytes)
{
	mOperationHandler->getOperationResultData(resultDataPtr, sizeInBytes);
}

DEFINE_TRACE(WebVKC) {
	visitor->trace(mDeviceList);
}

} // namespace blink
