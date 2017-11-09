// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLMemoryObject_H
#define WebCLMemoryObject_H

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "platform/heap/Handle.h"

#include "WebCLInclude.h"

namespace blink {

class WebCL;
class WebCLContext;
class WebCLGLObjectInfo;

class WebCLMemoryObject : public GarbageCollectedFinalized<WebCLMemoryObject>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebCLMemoryObject* create(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared) {
		return new WebCLMemoryObject(context, clContext, memory, isShared);
	}
	virtual ~WebCLMemoryObject();

	/*
	 * JS API
	 */
	ScriptValue getInfo(ScriptState* scriptState, int name, ExceptionState& ec);
	void release(ExceptionState& ec);

	ScriptValue getGLObjectInfo(ScriptState* scriptState, ExceptionState& ec);

	cl_mem getCLMem() { return mClMemObj; };
	size_t getSize() { return mSize; };
	bool isShared() { return mShared; };
	bool isBuffer() { return CL_MEM_OBJECT_BUFFER == mMemObjType; };

	cl_int getMemObjType(ExceptionState&);
	size_t getMemSize(ExceptionState&);
	cl_ulong getMemFlag(ExceptionState&);

	//cl/gl Sharing
	bool isGLBuffer() {return mGLBuffer;};
	int getGLServiceID() {return mGlServiceID;};
	void setGLBuffer(bool isGLBuffer) {mGLBuffer = isGLBuffer;};
	void setGLServiceID(int glServiceID) {mGlServiceID = glServiceID;};

	DECLARE_VIRTUAL_TRACE();

protected:
	WebCLMemoryObject(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared);

	Member<WebCL> mContext;
	Member<WebCLContext> mClContext;
	cl_mem mClMemObj = NULL;
	cl_mem mClAssociatedMemObj = NULL;
	cl_int mMemObjType;

	bool mShared;
	long mNumMems;
	size_t mSize;
	int mFlags;
	bool mGLBuffer;
	int mGlServiceID;
};

}

#endif //WebCLMemoryObject.h
