// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "platform/bindings/V8Binding.h"
#include "bindings/modules/v8/V8WebCLMemoryObject.h"
#include "WebCLMemoryObject.h"

#include "WebCL.h"
#include "WebCLContext.h"
#include "WebCLBuffer.h"
#include "core/dom/custom/WebCL/WebCLException.h"

#include "modules/webcl/WebCLGLObjectInfo.h"
#include "bindings/modules/v8/V8WebCLGLObjectInfo.h"
#include "bindings/modules/v8/V8WebGLBuffer.h"
#include "bindings/modules/v8/V8WebGLRenderbuffer.h"
#include "bindings/modules/v8/V8WebGLTexture.h"

namespace blink {

WebCLMemoryObject::WebCLMemoryObject(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared)
	: mContext(context),
	  mClContext(clContext),
	  mClMemObj(memory),
	  mShared(isShared)
{
	mNumMems = 0;

	cl_int err = WebCLException::FAILURE;
	err = webcl_clGetMemObjectInfo(webcl_channel_, mClMemObj, CL_MEM_TYPE, sizeof(cl_int), &mMemObjType, NULL);

	if(err != CL_SUCCESS)
		mMemObjType = 0;

	mGLBuffer = false;
	mGlServiceID = -1;
}

WebCLMemoryObject::~WebCLMemoryObject()
{
}

ScriptValue WebCLMemoryObject::getInfo(ScriptState* scriptState, int name, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLMemoryObject::getInfo, name=" << name;

	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();

	if(mContext.Get() == NULL || mClMemObj == NULL)
	{
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	cl_int err = WebCLException::FAILURE;
	switch(name) {
	case WebCL::MEM_TYPE:
		if(mMemObjType) {
			return ScriptValue(scriptState, v8::Integer::New(isolate, mMemObjType));
		}
		else {
			err = webcl_clGetMemObjectInfo(webcl_channel_, mClMemObj, CL_MEM_TYPE, sizeof(cl_int), &mMemObjType, NULL);
			if(err == WebCL::SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::New(isolate, mMemObjType));
			}
		}
		break;
	case WebCL::MEM_FLAGS:
		cl_ulong memFlags;
		err = webcl_clGetMemObjectInfo(webcl_channel_, mClMemObj, CL_MEM_FLAGS, sizeof(cl_ulong), &memFlags, NULL);
		if(err == WebCL::SUCCESS) {
			mFlags = static_cast<unsigned int>(memFlags);

			CLLOG(INFO) << "FLAGS===" << mFlags;

			return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(memFlags)));
		}
		break;
	case WebCL::MEM_SIZE:
		size_t memSize;
		err = webcl_clGetMemObjectInfo(webcl_channel_, mClMemObj, CL_MEM_SIZE, sizeof(size_t), &memSize, NULL);
		if(err == WebCL::SUCCESS) {
			mSize = memSize;
			return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(memSize)));
		}
		break;
	case WebCL::MEM_CONTEXT:
		if(mClContext.Get()) {
			Persistent<WebCLContext> clContextInfo = mClContext.Get();
			return ScriptValue(scriptState, ToV8(clContextInfo, creationContext, isolate));
		}
		else {
			return ScriptValue(scriptState, v8::Null(isolate));
		}
		break;
	case WebCL::MEM_ASSOCIATED_MEMOBJECT:
		if(isBuffer()) {
			WebCLBuffer* buffer = (WebCLBuffer*)this;
			if(buffer->isSubBuffer()) {
				Persistent<WebCLBuffer> clBufferInfo = buffer->getAssociatedBuffer();
				return ScriptValue(scriptState, ToV8(clBufferInfo, creationContext, isolate));
			}
			else {
				return ScriptValue(scriptState, v8::Null(isolate));
			}
		}
		break;
	case WebCL::MEM_OFFSET:
		if (isBuffer()) {
			WebCLBuffer* buffer = (WebCLBuffer*)this;
			if(buffer->isSubBuffer()) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(buffer->getSubOffset())));
			}
			else {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(0)));
			}
		}
		break;
	default:
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	WebCLException::throwException(err, ec);

	return ScriptValue(scriptState, v8::Null(isolate));
}

void WebCLMemoryObject::release(ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLMemoryObject::release";

	int errcode = CL_INVALID_VALUE;

	if (mClMemObj == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return;
	}

	errcode = webcl_clReleaseCommon(webcl_channel_, OPENCL_OBJECT_TYPE::CL_MEMORY_OBJECT, (cl_point)mClMemObj);

	if(errcode != CL_SUCCESS) {
		WebCLException::throwException(errcode, ec);
	}
	else {
		WebCLContext* clContext = mContext->findCLContext((cl_obj_key)mClContext->getClContext());
		if (clContext != NULL) {
			clContext->removeCLMemObj((cl_obj_key)mClMemObj);
		}
		mClMemObj = NULL;
	}

	//cl/gl sharing
	if (isGLBuffer()) {
		WebCLContext* clContext = mContext->findCLContext((cl_obj_key)mClContext->getClContext());
		if (clContext != NULL) {
			clContext->removeGLBuffer((cl_obj_key)getGLServiceID());
		}
	}
}

ScriptValue WebCLMemoryObject::getGLObjectInfo(ScriptState* scriptState, ExceptionState& ec)
{
	WebCLGLObjectInfo info = WebCLGLObjectInfo();
	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();
	if (!mContext->isEnableExtension("cl_khr_gl_sharing")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return ScriptValue(scriptState, v8::Null(isolate));
	} else {
		mContext->startHandling();

		WebCL_Operation_getGLObjectInfo operation = WebCL_Operation_getGLObjectInfo();
		operation.memobj = (cl_point)mClMemObj;
		operation.needObjectType = true;
		operation.needObjectName = true;

		mContext->setOperationParameter(&operation);
		mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::GET_GL_OBJECT_INFO);

		WebCL_Result_getGLObjectInfo result = WebCL_Result_getGLObjectInfo();
		mContext->getOperationResult(&result);
		CLLOG(INFO) << "WebCLContext::getGLObjectInfo, result.result : " << result.result << ", result.glObjectType : " << result.glObjectType
		<< ", glObjectName : " << result.glObjectName << ", clglTextureTarget : " << result.clGLTextureTarget << ", clclGLMipMapLevel : " << result.clGLMipMapLevel;

		if (result.result != CL_SUCCESS) {
			WebCLException::throwException(result.result, ec);
			return ScriptValue(scriptState, v8::Null(isolate));
		}
		info.setType(result.glObjectType);
		switch(result.glObjectType) {
			case CL_GL_OBJECT_BUFFER: {
				WebGLBuffer* glBuffer = (WebGLBuffer*)mClContext->findGLBuffer((cl_obj_key)result.glObjectName);
				info.setGlObject(ScriptValue(scriptState, ToV8(glBuffer, creationContext, isolate)));
				break;
			}
			case CL_GL_OBJECT_RENDERBUFFER: {
				WebGLRenderbuffer* glBuffer = (WebGLRenderbuffer*)mClContext->findGLBuffer((cl_obj_key)result.glObjectName);
				info.setGlObject(ScriptValue(scriptState, ToV8(glBuffer, creationContext, isolate)));
				break;
			}
			case CL_GL_OBJECT_TEXTURE2D:
			case CL_GL_OBJECT_TEXTURE3D: {
				WebGLTexture* glBuffer = (WebGLTexture*)mClContext->findGLBuffer((cl_obj_key)result.glObjectName);
				info.setGlObject(ScriptValue(scriptState, ToV8(glBuffer, creationContext, isolate)));
				info.setTextureTarget(result.clGLTextureTarget);
				info.setMipmapLevel(result.clGLMipMapLevel);
				break;
			}
		}
		return ScriptValue(scriptState, ToV8(info, creationContext, isolate));
	}
	return ScriptValue(scriptState, v8::Null(isolate));
}

cl_int WebCLMemoryObject::getMemObjType(ExceptionState& ec) {

	if(mContext.Get() == NULL || mClMemObj == NULL)
	{
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return 0;
	}

	cl_int err = WebCLException::FAILURE;

	if(!mMemObjType) {
		err = webcl_clGetMemObjectInfo(webcl_channel_, mClMemObj, CL_MEM_TYPE, sizeof(cl_int), &mMemObjType, NULL);
		if (err != CL_SUCCESS) {
			WebCLException::throwException(err, ec);
		}
	}

	return mMemObjType;
}

size_t WebCLMemoryObject::getMemSize(ExceptionState& ec) {

	if(mContext.Get() == NULL || mClMemObj == NULL)
	{
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return 0;
	}

	cl_int err = WebCLException::FAILURE;

	size_t memSize = 0;
	err = webcl_clGetMemObjectInfo(webcl_channel_, mClMemObj, CL_MEM_SIZE, sizeof(size_t), &memSize, NULL);
	if(err == WebCL::SUCCESS) {
		mSize = memSize;
		return memSize;
	}
	WebCLException::throwException(err, ec);

	return 0;
}

cl_ulong WebCLMemoryObject::getMemFlag(ExceptionState& ec) {

	if(mContext.Get() == NULL || mClMemObj == NULL)
	{
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return 0;
	}

	cl_int err = WebCLException::FAILURE;

	cl_ulong memFlags;

	err = webcl_clGetMemObjectInfo(webcl_channel_, mClMemObj, CL_MEM_FLAGS, sizeof(cl_ulong), &memFlags, NULL);

	if(err == WebCL::SUCCESS) {
		return memFlags;
	}

	WebCLException::throwException(err, ec);

	return 0;
}

DEFINE_TRACE(WebCLMemoryObject) {
	visitor->Trace(mContext);
	visitor->Trace(mClContext);
}

}
