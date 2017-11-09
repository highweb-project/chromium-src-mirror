// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "platform/bindings/V8Binding.h"
#include "platform/bindings/ToV8.h"

#include "WebVKCDevice.h"
#include "WebVKCBuffer.h"
#include "WebVKCCommandQueue.h"
#include "WebVKCProgram.h"
#include "core/dom/custom/WebVulkan/WebVKCException.h"

namespace blink {

WebVKCDevice::WebVKCDevice(VKCuint& vdIndex, VKCPoint& physicalDeviceList, Member<WebVKC> vkc, ExceptionState& ec)
{
	this->vdIndex = vdIndex;
	this->mVKC = vkc;
	vkcPhysicalDeviceList = physicalDeviceList;
	VKCLOG(INFO) << "WebVKCDevice create " << vdIndex << ", " << physicalDeviceList;

	VKCResult result = webvkc_createDevice(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, &vkcDevice, &vkcQueue);

	if (result != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		VKCLOG(INFO) << "WebVKCDevice result : " << result;
	}
}

WebVKCDevice::~WebVKCDevice()
{
	vkcPhysicalDeviceList = 0;
	vdIndex = 0;
	vkcDevice = 0;
	vkcQueue = 0;
	mVKC = NULL;
}

ScriptValue WebVKCDevice::getInfo(ScriptState* scriptState, unsigned name, ExceptionState& ec)
{
	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();

 	if (vkcPhysicalDeviceList == 0) {
 		VKCLOG(INFO) << "Error: Invalid physicalDeviceList";
 		WebVKCException::throwVKCException(VKCResult::VK_ERROR_INITIALIZATION_FAILED, ec);
 		return ScriptValue(scriptState, v8::Null(isolate));
 	}

 	uint32_t device_uint = 0;
 	char device_string[256] = { 0 };
 	VKCuint device_array[3] = { 0 };
 	VKCResult result;

 	switch(name)	{
 		case WebVKC::VKC_apiVersion:
 			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_uint);
 			if (result == VKCResult::VK_SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(device_uint)));
 			}
 			break;
		case WebVKC::VKC_driverVersion:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_uint);
			if (result == VKCResult::VK_SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(device_uint)));
			}
			break;
		case WebVKC::VKC_vendorID:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_uint);
			if (result == VKCResult::VK_SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(device_uint)));
			}
			break;
		case WebVKC::VKC_deviceID:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_uint);
			if (result == VKCResult::VK_SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(device_uint)));
			}
			break;
		case WebVKC::VKC_deviceType:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_uint);
			if (result == VKCResult::VK_SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(device_uint)));
			}
			break;
		case WebVKC::VKC_deviceName:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_string);
			if (result == VKCResult::VK_SUCCESS) {
				return ScriptValue(scriptState, V8String(isolate, String(device_string)));
			}
			break;
		case WebVKC::VKC_maxMemoryAllocationCount:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_uint);
			if (result == VKCResult::VK_SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(device_uint)));
			}
			break;
		case WebVKC::VKC_maxComputeWorkGroupCount:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_array);
			if (result == VKCResult::VK_SUCCESS) {
				Vector<unsigned> maxComputeWorkGroupCount;
				maxComputeWorkGroupCount.push_back(device_array[0]);
				maxComputeWorkGroupCount.push_back(device_array[1]);
				maxComputeWorkGroupCount.push_back(device_array[2]);
				return ScriptValue(scriptState, ToV8(maxComputeWorkGroupCount, creationContext, isolate));
			}
			break;
		case WebVKC::VKC_maxComputeWorkGroupInvocations:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_uint);
			if (result == VKCResult::VK_SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(device_uint)));
			}
			break;
		case WebVKC::VKC_maxComputeWorkGroupSize:
			result = webvkc_getDeviceInfo(webvkc_channel_, vdIndex, vkcPhysicalDeviceList, name, &device_array);
			if (result == VKCResult::VK_SUCCESS) {
				Vector<unsigned> maxComputeWorkGroupSize;
				maxComputeWorkGroupSize.push_back(device_array[0]);
				maxComputeWorkGroupSize.push_back(device_array[1]);
				maxComputeWorkGroupSize.push_back(device_array[2]);
				return ScriptValue(scriptState, ToV8(maxComputeWorkGroupSize, creationContext, isolate));
			}
			break;
		default:
			VKCLOG(INFO) << "input name not include name table";
 			return ScriptValue(scriptState, v8::Null(isolate));
 	}

 	return ScriptValue(scriptState, v8::Null(isolate));
 }

void WebVKCDevice::release(ExceptionState& ec)
{
	VKCLOG(INFO) << "WebVKCDevice::release";
	if (releasing) {
		VKCLOG(INFO) << "WebVKCDevice is already releasing";
		return;
	}

	releasing = true;

	if (mBufferBindingMap.size() > 0) {
		for(WebVKCBufferBindingMap::iterator bufferIter = mBufferBindingMap.begin(); bufferIter != mBufferBindingMap.end(); ++bufferIter) {
			WebVKCBuffer* buffer = (WebVKCBuffer*)bufferIter->value;
			VKCLOG(INFO) << "WebVKCBuffer = " << buffer;
			if (buffer != NULL)
				buffer->release(ec);
		}
		mBufferBindingMap.clear();
	}

	if (mCommandQueueBindingMap.size() > 0) {
		for(WebVKCCommandQueueBindingMap::iterator queueIter = mCommandQueueBindingMap.begin(); queueIter != mCommandQueueBindingMap.end(); ++queueIter) {
			WebVKCCommandQueue* queue = (WebVKCCommandQueue*)queueIter->value;
			VKCLOG(INFO) << "WebVKCCommandQueue = " << queue;
			if (queue != NULL)
				queue->release(ec);
		}
		mCommandQueueBindingMap.clear();
	}

	if (mProgramBindingMap.size() > 0) {
		for(WebVKCProgramBindingMap::iterator programIter = mProgramBindingMap.begin(); programIter != mProgramBindingMap.end(); ++programIter) {
			WebVKCProgram* program = (WebVKCProgram*)programIter->value;
			VKCLOG(INFO) << "WebVKCProgram = " << program;
			if (program != NULL)
				program->release(ec);
		}
		mProgramBindingMap.clear();
	}

	VKCResult result = webvkc_destroyDevice(webvkc_channel_, &vkcDevice, &vkcQueue);

	if (result != VKCResult::VK_SUCCESS) {
		WebVKCException::throwVKCException(result, ec);
		VKCLOG(INFO) << "destoryDevice fail : " << result;
	}

	vkcPhysicalDeviceList = 0;
	vdIndex = 0;
	vkcDevice = 0;
	vkcQueue = 0;

	mVKC->removeDevice(this);

	mVKC = NULL;
}

Member<WebVKCBuffer> WebVKCDevice::createBuffer(unsigned sizeInBytes, ExceptionState& es) {
	if (vkcDevice == 0) {
		VKCLOG(INFO) << "buffer not initialized";
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, es);
		return nullptr;
	}
	if (sizeInBytes % 4 != 0) {
		VKCLOG(INFO) << "sizeInBytes not valid";
		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, es);
		return nullptr;
	}
	WebVKCBuffer* buffer = WebVKCBuffer::create(sizeInBytes, this, es);
	if (es.HadException()) {
		es.RethrowV8Exception(es.GetException());
		buffer->release(es);
		return nullptr;
	}
	VKCLOG(INFO) << "buffer : " << buffer->getBufferPoint();
	if (buffer->getBufferPoint() == 0) {
		VKCLOG(INFO) << "buffer create fail";
		WebVKCException::throwVKCException(VKC_FAILURE, es);
		buffer->release(es);
		return nullptr;
	}
	mBufferBindingMap.Set(buffer->getBufferPoint(), buffer);
	return buffer;
}

Member<WebVKCCommandQueue> WebVKCDevice::createCommandQueue(ExceptionState& es) {
	if (vkcDevice == 0) {
		VKCLOG(INFO) << "device not initialized";
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, es);
		return nullptr;
	}
	WebVKCCommandQueue* queue = WebVKCCommandQueue::create(this, es);
	if (es.HadException()) {
		es.RethrowV8Exception(es.GetException());
		queue->release(es);
		return nullptr;
	}
	VKCLOG(INFO) << "queue : " << queue->getCMDBufferPoint();
	if (queue->getCMDBufferPoint() == 0) {
		VKCLOG(INFO) << "queue create fail";
		WebVKCException::throwVKCException(VKC_FAILURE, es);
		queue->release(es);
		return nullptr;
	}
	mCommandQueueBindingMap.Set(queue->getCMDBufferPoint(), queue);
	return queue;
}

Member<WebVKCProgram> WebVKCDevice::createProgram(String shaderPath, VKCuint useBufferCount, ExceptionState& es) {
	if (vkcDevice == 0) {
		VKCLOG(INFO) << "device not initialized";
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, es);
		return nullptr;
	}
	if (shaderPath.IsEmpty()) {
		VKCLOG(INFO) << "invalid argument";
		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, es);
		return nullptr;
	}
	WebVKCProgram* program = WebVKCProgram::createWithShaderUrl(this, shaderPath, useBufferCount, es);
	if (es.HadException()) {
		es.RethrowV8Exception(es.GetException());
		program->release(es);
		return nullptr;
	}
	VKCLOG(INFO) << "program : " << program->getPipelinePoint();
	if (program->getPipelinePoint() == 0) {
		VKCLOG(INFO) << "program create fail";
		WebVKCException::throwVKCException(VKC_FAILURE, es);
		program->release(es);
		return nullptr;
	}
	mProgramBindingMap.Set(program->getPipelinePoint(), program);
	return program;
}

Member<WebVKCProgram> WebVKCDevice::createProgramWithShaderCode(String shaderCode, VKCuint useBufferCount, ExceptionState& es) {
	if (vkcDevice == 0) {
		VKCLOG(INFO) << "device not initialized";
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, es);
		return nullptr;
	}
	if (shaderCode.IsEmpty()) {
		VKCLOG(INFO) << "invalid argument";
		WebVKCException::throwVKCException(VKC_ARGUMENT_NOT_VALID, es);
		return nullptr;
	}
	WebVKCProgram* program = WebVKCProgram::createWithShaderCode(this, shaderCode, useBufferCount, es);
	if (es.HadException()) {
		es.RethrowV8Exception(es.GetException());
		program->release(es);
		return nullptr;
	}
	VKCLOG(INFO) << "program : " << program->getPipelinePoint();
	if (program->getPipelinePoint() == 0) {
		VKCLOG(INFO) << "program create fail";
		WebVKCException::throwVKCException(VKC_FAILURE, es);
		program->release(es);
		return nullptr;
	}
	mProgramBindingMap.Set(program->getPipelinePoint(), program);
	return program;
}

void WebVKCDevice::submit(Member<WebVKCCommandQueue> commandBuffer, ExceptionState& ec) {
	if (vkcDevice == 0) {
		VKCLOG(INFO) << "device not initialized";
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, ec);
		return;
	}

	VKCResult result = webvkc_queueSubmit(webvkc_channel_, vkcQueue, commandBuffer->getCMDBufferPoint());

	if (result != VKCResult::VK_SUCCESS) {
		VKCLOG(INFO) << "submit fail : " << result;
		WebVKCException::throwVKCException(result, ec);
		return;
	}
}

void WebVKCDevice::wait(ExceptionState& ec) {
	if (vkcDevice == 0) {
		VKCLOG(INFO) << "device not initialized";
		WebVKCException::throwVKCException(VKCResult::VK_NOT_READY, ec);
		return;
	}

	VKCResult result = webvkc_deviceWaitIdle(webvkc_channel_, vkcDevice);

	if (result != VKCResult::VK_SUCCESS) {
		VKCLOG(INFO) << "submit fail : " << result;
		WebVKCException::throwVKCException(result, ec);
		return;
	}
}

void WebVKCDevice::deleteVKCBuffer(Member<WebVKCBuffer> buffer) {
	VKCPoint key = buffer->getBufferPoint();
	if (key == 0) {
		VKCLOG(INFO) << "key error";
		return;
	}
	if (!releasing) {
		if (mBufferBindingMap.Contains(key)) {
			mBufferBindingMap.erase(key);
		}
	}

}
void WebVKCDevice::deleteCommandQueue(Member<WebVKCCommandQueue> queue) {
	VKCPoint key = queue->getCMDBufferPoint();
	if (key == 0) {
		VKCLOG(INFO) << "key error";
		return;
	}
	if (!releasing) {
		if (mCommandQueueBindingMap.Contains(key)) {
			mCommandQueueBindingMap.erase(key);
		}
	}
}
void WebVKCDevice::deleteProgram(Member<WebVKCProgram> program) {
	VKCPoint key = program->getPipelinePoint();
	if (key == 0) {
		VKCLOG(INFO) << "key error";
		return;
	}
	if (!releasing) {
		if (mProgramBindingMap.Contains(key)) {
			mProgramBindingMap.erase(key);
		}
	}
}

DEFINE_TRACE(WebVKCDevice) {
	visitor->Trace(mBufferBindingMap);
	visitor->Trace(mCommandQueueBindingMap);
	visitor->Trace(mProgramBindingMap);
	visitor->Trace(mVKC);
}

}
