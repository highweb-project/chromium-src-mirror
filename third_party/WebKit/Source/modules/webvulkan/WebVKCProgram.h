// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebVKCProgram_h
#define WebVKCProgram_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "core/typed_arrays/DOMArrayBufferView.h"

#include "WebVKCInclude.h"
#include "WebVKC.h"

namespace blink {

class WebVKCBuffer;

// typedef HeapHashMap<VKCuint, Member<WebVKCBuffer>> BufferBindingMap;

class WebVKCProgram : public GarbageCollectedFinalized<WebVKCProgram>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebVKCProgram* createWithShaderUrl(Member<WebVKCDevice> device, String shaderPath, VKCuint useBufferCount, ExceptionState& ec);
	static WebVKCProgram* createWithShaderCode(Member<WebVKCDevice> device, String shaderCode, VKCuint useBufferCount, ExceptionState& ec);
	virtual ~WebVKCProgram();

	VKCPoint getPipelinePoint() {return vkcPipeline;};
	VKCPoint getDescriptorSetPoint() {return vkcDescriptorSet;};
	VKCPoint getPipelineLayoutPoint() {return vkcPipelineLayout;};
	
	void setShaderModule(VKCPoint modules) {
		vkcShaderModule = modules;
	};

	void release(ExceptionState& ec);

	void setArg(VKCuint id, Member<WebVKCBuffer> buffer, ExceptionState& ec);
	void updateDescriptor(ExceptionState& ec);

	DECLARE_VIRTUAL_TRACE();

private:
	WebVKCProgram(Member<WebVKCDevice> device, VKCuint useBufferCount, ExceptionState& ec);
	VKCResult createPipeline();

	Member<WebVKCDevice> mDevice;

	VKCPoint vkcDescriptorSetLayout = 0;
	VKCPoint vkcDescriptorPool = 0;
	VKCPoint vkcDescriptorSet = 0;

	VKCPoint vkcPipelineLayout = 0;
	VKCPoint vkcPipelineCache = 0;
	VKCPoint vkcPipeline = 0;

	VKCPoint vkcShaderModule = 0;

	HeapVector<Member<WebVKCBuffer>> bufferTable;
	VKCuint mBufferCount = 0;

	bool needThrowException(VKCResult& result, ExceptionState& ec);
};

}

#endif //WebVKCProgram_h
