// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLKernel_h
#define WebCLKernel_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "WebCLInclude.h"

namespace blink {

class WebCL;
class WebCLDevice;
class WebCLKernelArgInfo;
class WebCLSampler;
class WebCLMemoryObject;
class WebCLImage;
class WebCLBuffer;
class DOMArrayBufferView;
class WebCLContext;
class WebCLProgram;
class WebCLKernelArgInfoProvider;

class WebCLKernel : public GarbageCollectedFinalized<WebCLKernel>, public ScriptWrappable {
		DEFINE_WRAPPERTYPEINFO();
public:
	static WebCLKernel* create(WebCL* context, cl_kernel kernel, WebCLProgram* program, String kernelName) {
		return new WebCLKernel(context, kernel, program, kernelName);
	}
	virtual ~WebCLKernel();
	ScriptValue getInfo (ScriptState*, CLenum, ExceptionState&);
	ScriptValue getWorkGroupInfo(ScriptState*, WebCLDevice*, CLenum, ExceptionState&);

	WebCLKernelArgInfo getArgInfo(CLuint, ExceptionState&);

	void setArg(CLuint, const ScriptValue&, ExceptionState&);
	void setArg(CLuint, WebCLSampler*, ExceptionState&);
	void setArg(CLuint, DOMArrayBufferView*, ExceptionState&);

	void setArg(unsigned index, WebCLBuffer* buffer, ExceptionState& ec);
	void setArg(unsigned index, WebCLImage* image, ExceptionState& ec);

	void setArg(unsigned index, unsigned* argValue, ExceptionState& ec);

	void release(ExceptionState&);
	
	void setDevice(WebCLDevice*);
	void setCLContext(WebCLContext*);
	cl_kernel getCLKernel();

	size_t associatedArgsNum();

	WebCLProgram* getProgram();
	String& getKernelName();
	void setKernelName(String kernelName);

	String getKernelName(ExceptionState&);

	Vector<unsigned> getCompileWorkGroupSizeVector(WebCLDevice* device, ExceptionState&);

	size_t getKernelNumArgs(ExceptionState&);

	DECLARE_TRACE();
private:
	WebCLKernel(WebCL*, cl_kernel, WebCLProgram*, String);	

	void setArg(CLuint, cl_mem, ExceptionState&);
							
	Member<WebCL> mContext;
	Member<WebCLContext> mClContext;
	Member<WebCLProgram> mClProgram;
	cl_kernel mClKernel;
	Member<WebCLDevice> mDeviceId;
	String mKernelName;
	Member<WebCLKernelArgInfoProvider> mArgInfoProvider;
	std::vector<int> mAssociatedArgsMarking;
};

} // namespace blink

#endif // WebCLKernel_h
