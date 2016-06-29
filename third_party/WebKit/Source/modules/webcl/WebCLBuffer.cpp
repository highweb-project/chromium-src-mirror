// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "bindings/core/v8/V8Binding.h"
#include "bindings/core/v8/ToV8.h"

#include "WebCLBuffer.h"

#include "WebCL.h"
#include "WebCLException.h"

namespace blink {

WebCLBuffer::WebCLBuffer(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared)
	: WebCLMemoryObject(context, clContext, memory, isShared)
{
	mIsSubBuffer = false;
}

WebCLBuffer::~WebCLBuffer()
{
}

Member<WebCLBuffer> WebCLBuffer::createSubBuffer(int memFlags, unsigned origin, unsigned sizeInBytes, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLBuffer::createSubBuffer";

	Member<WebCLBuffer> subBufferObject;

	if(mClMemObj == NULL) {
		ec.throwDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
		CLLOG(INFO) << "WebCLException::INVALID_MEM_OBJECT, mClMemObj is null";
		return subBufferObject;
	}

	if(mIsSubBuffer) {
		ec.throwDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
		CLLOG(INFO) << "WebCLException::INVALID_MEM_OBJECT";
		return subBufferObject;
	}

	if(memFlags != WebCL::MEM_READ_ONLY
			&& memFlags != WebCL::MEM_READ_WRITE
			&& memFlags != WebCL::MEM_WRITE_ONLY) {
		ec.throwDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE reason=not compatible memType");
		CLLOG(INFO) << "WebCLException::INVALID_MEM_OBJECT reason=not compatible memType";
		return subBufferObject;
	}

	if((sizeInBytes == 0)
			||(origin + sizeInBytes) > mSize) {
		ec.throwDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE reason=size 0");
		CLLOG(INFO) << "WebCLException::INVALID_VALUE reason=size 0";
		return subBufferObject;
	}

	if((memFlags & mFlags) != mFlags && mFlags != WebCL::MEM_READ_WRITE) {
		ec.throwDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE reason=memFlags is not equal");
		CLLOG(INFO) << "WebCLException::INVALID_VALUE reason=memFlags is not equal, origin=" << mFlags << ", new=" << memFlags;
		return subBufferObject;
	}

	CLLOG(INFO) << "origin=" << origin << ", size=" << sizeInBytes;

	cl_int errcode_ret = WebCLException::FAILURE;
	cl_mem subBuffer;
	cl_buffer_region region =
	{
		origin,
		sizeInBytes
	};

	CLLOG(INFO) << "origin=" << region.origin << ", size=" << region.size;

	subBuffer = webcl_clCreateSubBuffer(webcl_channel_, mClMemObj, memFlags, CL_BUFFER_CREATE_TYPE_REGION, &region, &errcode_ret);
	if(errcode_ret == CL_SUCCESS) {
		subBufferObject = WebCLBuffer::create(mContext, mClContext.get(), subBuffer, true);
		subBufferObject->setIsSubBuffer();
		subBufferObject->setAssociatedBuffer(this);
		subBufferObject->setSubOffset(origin);

		mSubBufferList.append(subBufferObject);

		return subBufferObject;
	}
	else {
		ec.throwDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		CLLOG(INFO) << "WebCLException::INVALID_VALUE, err=" << errcode_ret;
		return subBufferObject;
	}
}

void WebCLBuffer::release(ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLBuffer::release";

	if(!mIsSubBuffer) {
		for(size_t i=0; i<mSubBufferList.size(); i++)
			mSubBufferList[i].get()->release(ec);
	}

	WebCLMemoryObject::release(ec);
}

DEFINE_TRACE(WebCLBuffer) {
	visitor->trace(mContext);
	visitor->trace(mSubBufferList);
	visitor->trace(mAssociatedBuffer);

	WebCLMemoryObject::trace(visitor);
}

}
