// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebVKCCommandQueue_h
#define WebVKCCommandQueue_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/DOMArrayBufferView.h"

#include "WebVKCInclude.h"
#include "WebVKC.h"

namespace blink {

class WebVKCProgram;
class WebVKCBuffer;

class WebVKCCommandQueue : public GarbageCollectedFinalized<WebVKCCommandQueue>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebVKCCommandQueue* create(Member<WebVKCDevice> device, ExceptionState& ec) {
		return new WebVKCCommandQueue(device, ec);
	}
	virtual ~WebVKCCommandQueue();

	VKCPoint getCMDBufferPoint() {return vkcCMDBuffer;};

	void release(ExceptionState& ec);

	void begin(Member<WebVKCProgram> program, ExceptionState& ec);
	void end(ExceptionState& ec);
	void dispatch(VKCuint workGroupX, VKCuint workGroupY, VKCuint workGroupZ, ExceptionState& ec);
	void barrier(ExceptionState& ec);
	void copyBuffer(Member<WebVKCBuffer> srcBuffer, Member<WebVKCBuffer> dstBuffer, VKCuint copyBufferSize, ExceptionState& ec);

	DECLARE_VIRTUAL_TRACE();

private:
	WebVKCCommandQueue(Member<WebVKCDevice> device, ExceptionState& ec);

	Member<WebVKCDevice> mDevice;
	VKCPoint vkcCMDBuffer = 0;
	VKCPoint vkcCMDPool = 0;

	bool isBegin = false;
};

}

#endif //WebVKCCommandQueue_h
