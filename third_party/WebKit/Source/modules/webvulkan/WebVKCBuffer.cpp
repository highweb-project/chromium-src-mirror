// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"

#include "WebVKCBuffer.h"
#include "WebVKCDevice.h"
#include "WebVKCException.h"

namespace blink {

WebVKCBuffer::WebVKCBuffer(VKCuint& byteSize, Member<WebVKCDevice> device, ExceptionState& ec)
{
	mDevice = device;
	mByteSize = byteSize;
	VKCLOG(INFO) << "WebVKCBuffer::WebVKCBuffer " << mDevice << ", " << mByteSize;

	VKCResult result = webvkc_createBuffer(webvkc_channel_, mDevice->getVKCDevice(), mDevice->getPhysicalDeviceList(),
		mDevice->getVDIndex(), mByteSize, &vkcBuffer, &vkcMemory);

	if (result != VKCResult::VK_SUCCESS) {
		VKCLOG(INFO) << "WebVKCBuffer result : " << result << ", " << vkcBuffer << ", " << vkcMemory;
		WebVKCException::throwVKCException(result, ec);
	}
}

WebVKCBuffer::~WebVKCBuffer()
{
	mDevice = nullptr;
}

VKCResult WebVKCBuffer::fillBuffer(VKCuint index, VKCuint data, ExceptionState& ec) {
	if (vkcBuffer == 0) {
		VKCLOG(INFO) << "WebVKCBuffer not initialized";
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, ec);
		return (VKCResult)VKCResult::VK_NOT_READY;
	}
	if (index % sizeof(VKCuint) != 0 || index > mByteSize) {
		VKCLOG(INFO) << "index not valid";
		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, ec);
		return (VKCResult)VKC_ARGUMENT_NOT_VALID;
	}
	VKCLOG(INFO) << "fillBuffer : " << index << ", " << data;

	std::vector<VKCuint> uintVector;
	uintVector.push_back(index);
	uintVector.push_back(data);
	uintVector.push_back(mByteSize);

	VKCResult result = webvkc_fillBuffer(webvkc_channel_, mDevice->getVKCDevice(), vkcMemory, uintVector);

	if (result != VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		VKCLOG(INFO) << "fillBuffer fail : " << result;
	}

	return result;
}

VKCResult WebVKCBuffer::readBuffer(VKCuint index, VKCuint bufferByteSize, DOMArrayBufferView* hostPtr, ExceptionState& ec) {
	if (index > mByteSize || (index + bufferByteSize) > mByteSize) {
		VKCLOG(INFO) << "index not valid";
		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, ec);
		return (VKCResult)VKC_ARGUMENT_NOT_VALID;
	}

	mDevice->getVKC()->startHandling();

	WebVKC_RW_Buffer_Operation operation = WebVKC_RW_Buffer_Operation(VULKAN_OPERATION_TYPE::VKC_READ_BUFFER);
	operation.index = index;
	operation.bufferByteSize = bufferByteSize;
	operation.vkcDevice = mDevice->getVKCDevice();
	operation.vkcMemory = vkcMemory;

	mDevice->getVKC()->setOperationParameter(&operation);

	mDevice->getVKC()->sendOperationSignal(VULKAN_OPERATION_TYPE::VKC_READ_BUFFER);

	WebVKC_Result_Base result = WebVKC_Result_Base();
	mDevice->getVKC()->getOperationResult(&result);

	if(result.result == VK_SUCCESS) {
		mDevice->getVKC()->getOperationResultData(hostPtr->baseAddress(), hostPtr->byteLength());
	} else {
		WebVKCException::throwVKCException(result.result, ec);
	}

	return result.result;
}

VKCResult WebVKCBuffer::writeBuffer(VKCuint index, VKCuint bufferByteSize, DOMArrayBufferView* hostPtr, ExceptionState& ec) {
	if (index > mByteSize || (index + bufferByteSize) > mByteSize) {
		VKCLOG(INFO) << "index not valid";
		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, ec);
		return (VKCResult)VKC_ARGUMENT_NOT_VALID;
	}

	mDevice->getVKC()->startHandling();

	WebVKC_RW_Buffer_Operation operation = WebVKC_RW_Buffer_Operation(VULKAN_OPERATION_TYPE::VKC_WRITE_BUFFER);
	operation.index = index;
	operation.bufferByteSize = bufferByteSize;
	operation.vkcDevice = mDevice->getVKCDevice();
	operation.vkcMemory = vkcMemory;

	mDevice->getVKC()->setOperationParameter(&operation);

	mDevice->getVKC()->setOperationData(hostPtr->baseAddress(), hostPtr->byteLength());

	mDevice->getVKC()->sendOperationSignal(VULKAN_OPERATION_TYPE::VKC_WRITE_BUFFER);

	WebVKC_Result_Base result = WebVKC_Result_Base();
	mDevice->getVKC()->getOperationResult(&result);

	VKCLOG(INFO) << "result : " << result.result;

	return result.result;
}

void WebVKCBuffer::release(ExceptionState& ec)
{
	VKCLOG(INFO) << "WebVKCBuffer::release";
	VKCResult result = VK_SUCCESS;
	if (vkcBuffer != 0 && vkcMemory != 0) {
		result = webvkc_releaseBuffer(webvkc_channel_, mDevice->getVKCDevice(), vkcBuffer, vkcMemory);
	}

	if (result != VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		VKCLOG(INFO) << "release buffer fail";
	}

	mDevice->deleteVKCBuffer(this);

	vkcBuffer = 0;
	vkcMemory = 0;
	mByteSize = 0;
	mBindingIndex = 0;

	mDevice = nullptr;
}

DEFINE_TRACE(WebVKCBuffer) {
	visitor->trace(mDevice);
}

}
