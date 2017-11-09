// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLProgram_h
#define WebCLProgram_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "WebCLInclude.h"

namespace blink {

class WebCL;
class WebCLKernel;
class WebCLDevice;
class WebCLCallback;
class WebCLContext;

class WebCLProgram : public GarbageCollectedFinalized<WebCLProgram>, public ScriptWrappable{
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebCLProgram* create(WebCL* context, cl_program program) {
		return new WebCLProgram(context, program);
	}
	virtual ~WebCLProgram();
	ScriptValue getInfo(ScriptState*, CLenum, ExceptionState&);
	ScriptValue getBuildInfo(ScriptState*, WebCLDevice*, CLenum, ExceptionState&);

	void build(const HeapVector<Member<WebCLDevice>>&, const String&, WebCLCallback*, ExceptionState&);
	void build(const String&, WebCLCallback*, ExceptionState&);

	void setDevice(WebCLDevice*);
	cl_program getCLProgram();

	void setSource(String source) {
		programSource = source;
	}

	String getSource() {
		return programSource;
	}

	WebCLKernel* createKernel(const String&, ExceptionState&);
	HeapVector<Member<WebCLKernel>> createKernelsInProgram(ExceptionState&);

	void triggerCallback();

	void release(ExceptionState&);

	void setHandlerKey(unsigned key) { mHandlerKey = key; };

	void setCLContext(WebCLContext* context) { mClContext = context; };

	DECLARE_TRACE();
private:
	WebCLProgram(WebCL*, cl_program);
	Member<WebCL> mContext;
	Member<WebCLContext> mClContext;
	Member<WebCLDevice> mDeviceId;
	Member<WebCLCallback> mBuildCallback;

	HeapVector<Member<WebCLDevice>> mDeviceList;

	cl_program mClProgram;
	String programSource;
	bool mIsProgramBuilt;
	unsigned mHandlerKey;
};

} // namespace blink

#endif // WebCLProgram_h
