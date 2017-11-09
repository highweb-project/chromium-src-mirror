// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "WebVKCProgram.h"
#include "WebVKCDevice.h"
#include "WebVKCBuffer.h"
#include "core/dom/custom/WebVulkan/WebVKCException.h"

namespace blink {

WebVKCProgram* WebVKCProgram::createWithShaderUrl(
		Member<WebVKCDevice> device, 
		String shaderPath, 
		VKCuint useBufferCount, 
		ExceptionState& ec) {
	WebVKCProgram* program = new WebVKCProgram(device, useBufferCount, ec);
	if (program && !ec.HadException()) {
		VKCPoint modules = 0;
		std::string stringPath = std::string(shaderPath.Utf8().data());
		VKCResult result = webvkc_createShaderModuleWithUrl(webvkc_channel_, device->getVKCDevice(), stringPath, &modules);
		VKCLOG(INFO) << "webvkc_createShaderModuleWithUrl : " << result << ", " << modules;
		program->setShaderModule(modules);
		if (program->needThrowException(result, ec)) {
			return program;
		}
		result = program->createPipeline();
		if (program->needThrowException(result, ec)) {
			return program;
		}
	}
	return program;
}

WebVKCProgram* WebVKCProgram::createWithShaderCode(
		Member<WebVKCDevice> device, 
		String shaderCode, 
		VKCuint useBufferCount, 
		ExceptionState& ec) {
	WebVKCProgram* program = new WebVKCProgram(device, useBufferCount, ec);
	if (program && !ec.HadException()) {
		VKCPoint modules = 0;
		std::string stringSources = std::string(shaderCode.Utf8().data());
		VKCResult result = webvkc_createShaderModuleWithSource(webvkc_channel_, device->getVKCDevice(), stringSources, &modules);
		VKCLOG(INFO) << "webvkc_createShaderModuleWithSource : " << result << ", " << modules;
		program->setShaderModule(modules);
		if (program->needThrowException(result, ec)) {
			return program;
		}
		result = program->createPipeline();
		if (program->needThrowException(result, ec)) {
			return program;
		}
	}
	return program;
}

WebVKCProgram::WebVKCProgram(Member<WebVKCDevice> device, VKCuint useBufferCount, ExceptionState& ec)
{
	mDevice = device;
	mBufferCount = useBufferCount;
	bufferTable.resize(mBufferCount);
	VKCLOG(INFO) << "WebVKCProgram::WebVKCProgram " << mDevice;

	VKCResult result = webvkc_createDescriptorSetLayout(webvkc_channel_, mDevice->getVKCDevice(), useBufferCount, &vkcDescriptorSetLayout);
	VKCLOG(INFO) << "webvkc_createDescriptorSetLayout : " << result << ", " << vkcDescriptorSetLayout;
	if (needThrowException(result, ec)) {
		return;
	}

	result = webvkc_createDescriptorPool(webvkc_channel_, mDevice->getVKCDevice(), useBufferCount, &vkcDescriptorPool);
	VKCLOG(INFO) << "webvkc_createDescriptorPool : " << result << ", " << vkcDescriptorPool;
	if (needThrowException(result, ec)) {
		return;
	}

	result = webvkc_createDescriptorSet(webvkc_channel_, mDevice->getVKCDevice(), vkcDescriptorPool, vkcDescriptorSetLayout, &vkcDescriptorSet);
	VKCLOG(INFO) << "webvkc_createDescriptorSet : " << result << ", " << vkcDescriptorSet;
	if (needThrowException(result, ec)) {
		return;
	}

	result = webvkc_createPipelineLayout(webvkc_channel_, mDevice->getVKCDevice(), vkcDescriptorSetLayout, &vkcPipelineLayout);
	VKCLOG(INFO) << "webvkc_createPipelineLayout : " << result << ", " << vkcPipelineLayout;
	if (needThrowException(result, ec)) {
		return;
	}
}

VKCResult WebVKCProgram::createPipeline() {
	VKCResult result = webvkc_createPipeline(webvkc_channel_, mDevice->getVKCDevice(), vkcPipelineLayout, vkcShaderModule, &vkcPipelineCache, &vkcPipeline);
	VKCLOG(INFO) << "webvkc_createPipeline : " << result << ", " << vkcPipelineCache << ", " << vkcPipeline;
	return result;
}

WebVKCProgram::~WebVKCProgram()
{
	mDevice = nullptr;
	vkcDescriptorSetLayout = 0;
	vkcDescriptorPool = 0;
	vkcDescriptorSet = 0;

	vkcPipelineLayout = 0;
	vkcPipelineCache = 0;
	vkcPipeline = 0;

	vkcShaderModule = 0;
	mBufferCount = 0;

	bufferTable.clear();

	mDevice = nullptr;
}

void WebVKCProgram::release(ExceptionState& ec)
{
	VKCLOG(INFO) << "WebVKCProgram::release";

	VKCResult result = VK_SUCCESS;
	if (vkcPipeline != 0 && vkcPipelineCache != 0) {
		result = webvkc_releasePipeline(webvkc_channel_, mDevice->getVKCDevice(), vkcPipelineCache, vkcPipeline);
	}

	VKCLOG(INFO) << "release vkcPipeline & vkcPipelineCache";
	if (vkcShaderModule != 0) {
		result = webvkc_releaseShaderModule(webvkc_channel_, mDevice->getVKCDevice(), vkcShaderModule);
	}
	VKCLOG(INFO) << "release vkcShaderModule";
	if (vkcPipelineLayout != 0) {
		result = webvkc_releasePipelineLayout(webvkc_channel_, mDevice->getVKCDevice(), vkcPipelineLayout);
	}
	VKCLOG(INFO) << "release vkcPipelineLayout";
	if (vkcDescriptorPool != 0 && vkcDescriptorSet != 0) {
		result = webvkc_releaseDescriptorSet(webvkc_channel_, mDevice->getVKCDevice(), vkcDescriptorPool, vkcDescriptorSet);
	}
	VKCLOG(INFO) << "release vkcDescriptorPool & vkcDescriptorSet";
	if (vkcDescriptorSetLayout != 0) {
		result = webvkc_releaseDescriptorSetLayout(webvkc_channel_, mDevice->getVKCDevice(), vkcDescriptorSetLayout);
	}
	VKCLOG(INFO) << "release vkcDescriptorSetLayout";
	if (needThrowException(result, ec)) {
		return;
	}
	if (vkcDescriptorPool != 0) {
		result = webvkc_releaseDescriptorPool(webvkc_channel_, mDevice->getVKCDevice(), vkcDescriptorPool);
	}
	if (needThrowException(result, ec)) {
		return;
	}

	mDevice->deleteProgram(this);

	vkcDescriptorSetLayout = 0;
	vkcDescriptorPool = 0;
	vkcDescriptorSet = 0;

	vkcPipelineLayout = 0;
	vkcPipelineCache = 0;
	vkcPipeline = 0;

	vkcShaderModule = 0;
	mBufferCount = 0;

	bufferTable.clear();

	mDevice = nullptr;
}

bool WebVKCProgram::needThrowException(VKCResult& result, ExceptionState& ec) {
	if (result != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		return true;
	}
	return false;
}

void WebVKCProgram::setArg(VKCuint id, Member<WebVKCBuffer> buffer, ExceptionState& ec) {
	if (id >= mBufferCount) {
		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, ec);
		return;
	}

	bufferTable.data()[id] = buffer;
	buffer->setBinding(id);
}

void WebVKCProgram::updateDescriptor(ExceptionState& ec) {
	std::vector<VKCPoint> bufferVector;

	for(VKCuint i = 0; i < mBufferCount; i++) {
		if (bufferTable[i] == nullptr) {
			VKCLOG(INFO) << "not enough buffer " << i;
			WebVKCException::throwVKCException(VKC_ERROR_NOT_SETARG_BUFFER_INDEX, ec);
			return;
		}
		bufferVector.push_back(bufferTable[i]->getBufferPoint());
	}

	VKCLOG(INFO) << "bufferVector.size : " << bufferVector.size();

	VKCResult result = webvkc_updateDescriptorSets(webvkc_channel_, mDevice->getVKCDevice(), vkcDescriptorSet, bufferVector);
	VKCLOG(INFO) << "webvkc_updateDescriptorSets : " << result;
}

DEFINE_TRACE(WebVKCProgram) {
	visitor->Trace(mDevice);
	visitor->Trace(bufferTable);
}

}
