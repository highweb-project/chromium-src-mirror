// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"

#include "WebVKCCommandQueue.h"
#include "WebVKCDevice.h"
#include "WebVKCException.h"
#include "WebVKCProgram.h"
#include "WebVKCBuffer.h"

namespace blink {

WebVKCCommandQueue::WebVKCCommandQueue(Member<WebVKCDevice> device, ExceptionState& ec)
{
	mDevice = device;
	VKCLOG(INFO) << "WebVKCCommandQueue::WebVKCCommandQueue " << mDevice;

	VKCResult result = webvkc_createCommandQueue(webvkc_channel_, mDevice->getVKCDevice(), mDevice->getPhysicalDeviceList(),
		mDevice->getVDIndex(), &vkcCMDBuffer, &vkcCMDPool);

	if (result != VKCResult::VK_SUCCESS) {
		VKCLOG(INFO) << "WebVKCBuffer result : " << result << ", " << vkcCMDBuffer;
		WebVKCException::throwVKCException(result, ec);
	}
}

WebVKCCommandQueue::~WebVKCCommandQueue()
{
	mDevice = nullptr;
	vkcCMDBuffer = 0;
	vkcCMDPool = 0;
}

void WebVKCCommandQueue::release(ExceptionState& ec)
{
	VKCLOG(INFO) << "WebVKCCommandQueue::release";
	VKCResult result = VK_SUCCESS;
	if ((vkcCMDBuffer != 0) && (vkcCMDPool != 0)) {
		result = webvkc_releaseCommandQueue(webvkc_channel_, mDevice->getVKCDevice(), vkcCMDBuffer, vkcCMDPool);
	}

	if (result != VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		VKCLOG(INFO) << "release commandQueue fail" << result;
	}

	mDevice->deleteCommandQueue(this);

	vkcCMDBuffer = 0;
	vkcCMDPool = 0;

	mDevice = nullptr;
}

void WebVKCCommandQueue::begin(Member<WebVKCProgram> program, ExceptionState& ec) {
	VKCLOG(INFO) << "WebVKCCommandQueue::begin " << program->getPipelinePoint();

	if (isBegin) {
		WebVKCException::throwVKCException(VKC_ALREADY_BEGIN_COMMAND_QUEUE, ec);
	}

	VKCResult result = webvkc_beginQueue(webvkc_channel_, vkcCMDBuffer, program->getPipelinePoint(), program->getPipelineLayoutPoint(), program->getDescriptorSetPoint());

	if (result != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		return;
	}

	isBegin = true;
}

void WebVKCCommandQueue::end(ExceptionState& ec) {
	if (!isBegin) {
		WebVKCException::throwVKCException(VKC_COMMAND_QUEUE_NOT_BEGINING, ec);
		return;
	}

	VKCResult result = webvkc_endQueue(webvkc_channel_, vkcCMDBuffer);

	if (result != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		return;
	}

	isBegin = false;
}

void WebVKCCommandQueue::dispatch(VKCuint workGroupX, VKCuint workGroupY, VKCuint workGroupZ, ExceptionState& ec) {
	if (!isBegin) {
		WebVKCException::throwVKCException(VKC_COMMAND_QUEUE_NOT_BEGINING, ec);
		return;
	}

	VKCResult result = webvkc_dispatch(webvkc_channel_, vkcCMDBuffer, workGroupX, workGroupY, workGroupZ);

	if (result != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		return;
	}
}

void WebVKCCommandQueue::barrier(ExceptionState& ec) {
	if (!isBegin) {
		WebVKCException::throwVKCException(VKC_COMMAND_QUEUE_NOT_BEGINING, ec);
		return;
	}

	VKCResult result = webvkc_pipelineBarrier(webvkc_channel_, vkcCMDBuffer);

	if (result != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		return;
	}
}

void WebVKCCommandQueue::copyBuffer(Member<WebVKCBuffer> srcBuffer, Member<WebVKCBuffer> dstBuffer, VKCuint copyBufferSize, ExceptionState& ec) {
	if (!isBegin) {
		WebVKCException::throwVKCException(VKC_COMMAND_QUEUE_NOT_BEGINING, ec);
		return;
	}

	if (copyBufferSize <= 0 || copyBufferSize > srcBuffer->getAllocateBufferSize() || copyBufferSize > dstBuffer->getAllocateBufferSize()) {
		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, ec);
	}

	VKCResult result = webvkc_cmdCopyBuffer(webvkc_channel_, vkcCMDBuffer, srcBuffer->getBufferPoint(), dstBuffer->getBufferPoint(), copyBufferSize);

	if (result != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		return;
	}
}

DEFINE_TRACE(WebVKCCommandQueue) {
	visitor->trace(mDevice);
}

}
