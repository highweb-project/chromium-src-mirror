// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLBuffer_h
#define WebCLBuffer_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "WebCLInclude.h"
#include "WebCLMemoryObject.h"

namespace blink {

class WebCL;

class WebCLBuffer final : public WebCLMemoryObject {
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebCLBuffer* create(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared) {
		return new WebCLBuffer(context, clContext, memory, isShared);
	}
	virtual ~WebCLBuffer();

	Member<WebCLBuffer> createSubBuffer(int memFlags, unsigned origin, unsigned sizeInBytes, ExceptionState& ec);

	void release(ExceptionState& ec);

	/*
	 * Internal functions
	 */
	void setIsSubBuffer() { mIsSubBuffer = true; };
	bool isSubBuffer() { return mIsSubBuffer;};
	void setAssociatedBuffer(WebCLBuffer* buffer) { mAssociatedBuffer = buffer; };
	WebCLBuffer* getAssociatedBuffer() { return mAssociatedBuffer;};
	void setSubOffset(unsigned offset) { mSubOffset = offset; };
	unsigned getSubOffset() { return mSubOffset;};

	DECLARE_VIRTUAL_TRACE();

private:
	WebCLBuffer(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared);

	bool mIsSubBuffer;
	HeapVector<Member<WebCLBuffer>> mSubBufferList;
	Member<WebCLBuffer> mAssociatedBuffer;
	unsigned mSubOffset;
};

}

#endif
