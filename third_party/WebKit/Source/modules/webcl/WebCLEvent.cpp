// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "platform/bindings/V8Binding.h"
#include "bindings/modules/v8/V8WebCLEvent.h"

#include "WebCLEvent.h"
#include "WebCL.h"
#include "WebCLContext.h"
#include "WebCLCallback.h"
#include "WebCLCommandQueue.h"
#include "core/dom/custom/WebCL/WebCLException.h"

#include "base/rand_util.h"

namespace blink {

WebCLEvent::WebCLEvent() : mContext(nullptr), mClEvent(nullptr)
{
}

WebCLEvent::WebCLEvent(cl_event clEvent) : mContext(nullptr), mClEvent(clEvent)
{
}

WebCLEvent::WebCLEvent(WebCL* context) : mContext(context), mClEvent(nullptr)
{
}

WebCLEvent::WebCLEvent(WebCL* context, cl_event clEvent) : mContext(context), mClEvent(clEvent)
{
}


ScriptValue WebCLEvent::getInfo(ScriptState* scriptState, int paramName, ExceptionState& ec)
{
	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();

	printf("getInfo Called = %d\n", paramName);
	cl_int err = WebCL::FAILURE;
	cl_uint clUInt = 0;
	cl_int clInt = 0;
	cl_command_type commandType = 0;
	cl_command_queue commandQueue = 0;
#if defined(OS_LINUX)
	cl_point clPoint;
#endif
	if (mClEvent == NULL) {
		printf("Error: Invalid CL Event\n");
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	switch(paramName)
	{
		case WebCL::EVENT_COMMAND_EXECUTION_STATUS :
			err = webcl_clGetEventInfo(webcl_channel_, mClEvent, paramName  , sizeof(clInt), &clInt, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::New(isolate, static_cast<int>(clInt)));
			break;
		case WebCL::EVENT_REFERENCE_COUNT:
			err = webcl_clGetEventInfo(webcl_channel_, mClEvent, paramName , sizeof(clUInt), &clUInt, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned int>(clUInt)));
			break;
		case WebCL::EVENT_COMMAND_TYPE:
			err = webcl_clGetEventInfo(webcl_channel_, mClEvent, paramName , sizeof(cl_command_type), &commandType, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned int>(commandType)));
			break;
		case WebCL::EVENT_COMMAND_QUEUE:
		{
			err = webcl_clGetEventInfo(webcl_channel_, mClEvent, paramName , sizeof(cl_command_queue), &commandQueue, NULL);
			Persistent<WebCLCommandQueue> cqObj = mClContext->findCLCommandQueue((cl_obj_key)commandQueue);
			if(cqObj == NULL)
			{
				return ScriptValue(scriptState, v8::Null(isolate));
			}
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, ToV8(cqObj, creationContext, isolate));
			break;
		}
		case WebCL::EVENT_CONTEXT:
#if defined(OS_ANDROID)
			err = webcl_clGetEventInfo(webcl_channel_, mClEvent, paramName, sizeof(cl_uint), &clUInt, NULL);
#elif defined(OS_LINUX)
			err = webcl_clGetEventInfo(webcl_channel_, mClEvent, paramName, sizeof(cl_point), &clPoint, NULL);
#endif

			if (err == CL_SUCCESS && mClContext != NULL) {
#if defined(OS_ANDROID)
				if (clUInt == (cl_point)mClContext->getClContext()) {
#elif defined(OS_LINUX)
				if (clPoint == (cl_point)mClContext->getClContext()) {
#endif
					return ScriptValue(scriptState, ToV8((Persistent<WebCLContext>)mClContext, creationContext, isolate));
				}
			}
			return ScriptValue(scriptState, v8::Null(isolate));
			break;
		default:
			printf("Error: Unsupported Event Info type\n");
			ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			return ScriptValue(scriptState, v8::Null(isolate));
	}
	WebCLException::throwException(err, ec);
	return ScriptValue(scriptState, v8::Null(isolate));
}

ScriptValue WebCLEvent::getProfilingInfo(ScriptState* scriptState, int paramName, ExceptionState& ec)
{
	v8::Isolate* isolate = scriptState->GetIsolate();

	cl_int err=WebCL::FAILURE;
	cl_ulong clULong = 0;

	CLLOG(INFO) << "WebCLEvent::getProfilingInfo";

	if (mClEvent == NULL) {
		CLLOG(INFO) << "WebCLEvent::getProfilingInfo : mClEvent is NULL";
		printf("Error: Invalid CL Event\n");
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	int status = getEventCommandExecutionStatus(ec);
	Member<WebCLCommandQueue> mCommandQueue = getEventCommandQueue(ec);
	if (ec.HadException()) {
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	unsigned properties = 0;
	if (mCommandQueue != nullptr) {
		properties = mCommandQueue->getQueueProperties(ec);
		if (ec.HadException()) {
			return ScriptValue(scriptState, v8::Null(isolate));
		}
	}

	if (isUserEvent() || status != WebCL::COMPLETE || !(properties == WebCL::QUEUE_PROFILING_ENABLE)) {
		ec.ThrowDOMException(WebCLException::PROFILING_INFO_NOT_AVAILABLE, "WebCLException::PROFILING_INFO_NOT_AVAILABLE");
		return ScriptValue(scriptState, v8::Null(isolate));
	}
	switch(paramName)
	{
		case WebCL::PROFILING_COMMAND_QUEUED:
			CLLOG(INFO) << "WebCLEvent::getProfilingInfo:param : PROFILING_COMMAND_QUEUED";
			err = webcl_clGetEventProfilingInfo(webcl_channel_, mClEvent, paramName, sizeof(cl_ulong), &clULong, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(clULong)));
			break;
		case WebCL::PROFILING_COMMAND_SUBMIT:
			CLLOG(INFO) << "WebCLEvent::getProfilingInfo:param : PROFILING_COMMAND_SUBMIT";
			err = webcl_clGetEventProfilingInfo(webcl_channel_, mClEvent, paramName, sizeof(cl_ulong), &clULong, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(clULong)));
			break;
		case WebCL::PROFILING_COMMAND_START:
			CLLOG(INFO) << "WebCLEvent::getProfilingInfo:param : PROFILING_COMMAND_START";
			err = webcl_clGetEventProfilingInfo(webcl_channel_, mClEvent, paramName, sizeof(cl_ulong), &clULong, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(clULong)));
			break;
		 case WebCL::PROFILING_COMMAND_END:
			CLLOG(INFO) << "WebCLEvent::getProfilingInfo:param : PROFILING_COMMAND_END";
			err = webcl_clGetEventProfilingInfo(webcl_channel_, mClEvent, paramName, sizeof(cl_ulong), &clULong, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned int>(clULong)));
			break;
		default:
			printf("Error: Unsupported Profiling Info type\n");
			ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			return ScriptValue(scriptState, v8::Null(isolate));
	}

	CLLOG(INFO) << "WebCLEvent::getProfilingInfo error : " << err;
	WebCLException::throwException(err, ec);
	return ScriptValue(scriptState, v8::Null(isolate));
}

void WebCLEvent::setCallback(CLenum commandExecCallbackType, WebCLCallback* notify, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLEvent::setCallback";
	cl_int err = WebCL::FAILURE;

	if (mClEvent == NULL) {
		printf("Error: Invalid CL Event\n");
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return;
	}

	if (commandExecCallbackType != WebCL::COMPLETE){
		CLLOG(INFO) << "WebCLEvent::setCallback commandExecCallbackType is not COMPLETE : " << commandExecCallbackType;
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
	}

	mCompleteCallback = notify;

	err = webcl_clSetEventCallback(webcl_channel_, mClEvent, commandExecCallbackType, commandExecCallbackType, mHandlerKey, OPENCL_OBJECT_TYPE::CL_EVENT);

	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	}
	return;
}

void WebCLEvent::release(ExceptionState& ec)
{
	cl_int err = WebCL::FAILURE;
	if (mClEvent == NULL && mContext == NULL){
		CLLOG(INFO) << "WebCLEvent not defined";
		return;
	}
	if (mClEvent == NULL) {
		CLLOG(INFO) << "mClEvent is NULL, INVALID_EVENT";
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return;
	}

	if(isUserEvent()) {
		webcl_clSetUserEventStatus(webcl_channel_, mClEvent , CL_COMPLETE);
	}

	err = webcl_clReleaseEvent(webcl_channel_, mClEvent);
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	}

	if (mClContext != NULL) {
		mClContext->removeCLEvent((cl_obj_key)mClEvent);
	}

	mClEvent = NULL;

	mCompleteCallback = nullptr;
	mRunningCallback = nullptr;
	mSubmittedCallback = nullptr;
	mQueuedCallback = nullptr;

	return;
}

void WebCLEvent::triggerCallback(int callbackType)
{
	WebCLCallback* callback = nullptr;
	switch(callbackType) {
	case WebCL::COMPLETE:
		callback = mCompleteCallback;
		break;
	case WebCL::RUNNING:
		callback = mRunningCallback;
		break;
	case WebCL::SUBMITTED:
		callback = mSubmittedCallback;
		break;
	case WebCL::QUEUED:
		callback = mQueuedCallback;
		break;
	}

	if(callback != nullptr)
		callback->handleEvent();
}

void WebCLEvent::setCLEvent(cl_event event) {
	this->mClEvent = event;
}

cl_event WebCLEvent::getCLEvent()
{
	return mClEvent;
}

int WebCLEvent::getEventCommandExecutionStatus(ExceptionState& ec) {

	cl_int err = WebCL::FAILURE;
	cl_int clInt = 0;

	if (mClEvent == NULL) {
		printf("Error: Invalid CL Event\n");
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return -1;
	}


	err = webcl_clGetEventInfo(webcl_channel_, mClEvent, WebCL::EVENT_COMMAND_EXECUTION_STATUS, sizeof(clInt), &clInt, NULL);

	if (err == CL_SUCCESS)
		return static_cast<int>(clInt);

	WebCLException::throwException(err, ec);

	return -1;
}

WebCLCommandQueue* WebCLEvent::getEventCommandQueue(ExceptionState& ec) {

	cl_int err = WebCL::FAILURE;
	cl_command_queue commandQueue = 0;

	if (mClEvent == NULL) {
		printf("Error: Invalid CL Event\n");
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return nullptr;
	}

	err = webcl_clGetEventInfo(webcl_channel_, mClEvent, WebCL::EVENT_COMMAND_QUEUE, sizeof(cl_command_queue), &commandQueue, NULL);
	Persistent<WebCLCommandQueue> cqObj = mClContext->findCLCommandQueue((cl_obj_key)commandQueue);

	if (cqObj != NULL && err == CL_SUCCESS) {
		return cqObj.Get();
	}

	WebCLException::throwException(err, ec);

	return nullptr;
}

DEFINE_TRACE(WebCLEvent) {
	visitor->Trace(mContext);
	visitor->Trace(mClContext);
	visitor->Trace(mCompleteCallback);
	visitor->Trace(mRunningCallback);
	visitor->Trace(mSubmittedCallback);
	visitor->Trace(mQueuedCallback);

}

}
