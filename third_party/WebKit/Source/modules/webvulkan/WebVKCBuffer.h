// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebVKCBuffer_h
#define WebVKCBuffer_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/DOMArrayBufferView.h"

#include "WebVKCInclude.h"
#include "WebVKC.h"

namespace blink {

class WebVKCBuffer : public GarbageCollectedFinalized<WebVKCBuffer>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();	
public:
	static WebVKCBuffer* create(VKCuint& sizeInBytes, Member<WebVKCDevice> device, ExceptionState& ec) {
		return new WebVKCBuffer(sizeInBytes, device, ec);
	}
	virtual ~WebVKCBuffer();

	VKCPoint getBufferPoint() {return vkcBuffer;};
	VKCPoint getMemoryPoint() {return vkcMemory;};

	void setBinding(VKCint bindingIndex) {mBindingIndex = bindingIndex;};
	VKCint getBindingIndex() {return mBindingIndex;};

	VKCuint getAllocateBufferSize() {return mByteSize;};
	VKCResult fillBuffer(VKCuint index, VKCuint data, ExceptionState& ec);

	VKCResult readBuffer(VKCuint index, VKCuint bufferByteSize, DOMArrayBufferView* hostPtr, ExceptionState& ec);
	VKCResult writeBuffer(VKCuint index, VKCuint bufferByteSize, DOMArrayBufferView* hostPtr, ExceptionState& ec);

	void release(ExceptionState& ec);

	DECLARE_VIRTUAL_TRACE();

private:
	WebVKCBuffer(VKCuint& sizeInBytes, Member<WebVKCDevice> device, ExceptionState& ec);

	VKCuint mByteSize = 0;

	Member<WebVKCDevice> mDevice;
	VKCPoint vkcBuffer = 0;
	VKCPoint vkcMemory = 0;

	VKCint mBindingIndex = -1;
};

}

#endif //WebVKCBuffer_h
