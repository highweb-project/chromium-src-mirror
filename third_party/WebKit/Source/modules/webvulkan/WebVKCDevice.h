// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebVKCDevice_h
#define WebVKCDevice_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "platform/heap/Handle.h"
#include <wtf/Vector.h>

#include "WebVKCInclude.h"
#include "WebVKC.h"

namespace blink {

class WebVKCBuffer;
class WebVKCCommandQueue;
class WebVKCOperationHandler;
class WebVKCProgram;

typedef HeapHashMap<VKCPoint, Member<WebVKCBuffer>> WebVKCBufferBindingMap;
typedef HeapHashMap<VKCPoint, Member<WebVKCCommandQueue>> WebVKCCommandQueueBindingMap;
typedef HeapHashMap<VKCPoint, Member<WebVKCProgram>> WebVKCProgramBindingMap;

class WebVKCDevice : public GarbageCollectedFinalized<WebVKCDevice>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();	
public:
	static WebVKCDevice* create(VKCuint& vdIndex, VKCPoint& mPhysicalDeviceList, Member<WebVKC> vkc, ExceptionState& ec) {
		return new WebVKCDevice(vdIndex, mPhysicalDeviceList, vkc, ec);
	}
	virtual ~WebVKCDevice();

	ScriptValue getInfo(ScriptState*, unsigned, ExceptionState&);
	void release(ExceptionState& ec);

	Member<WebVKCBuffer> createBuffer(VKCuint sizeInBytes, ExceptionState&);
	Member<WebVKCCommandQueue> createCommandQueue(ExceptionState&);
	Member<WebVKCProgram> createProgram(String shaderPath, VKCuint useBufferCount, ExceptionState&);

	VKCPoint& getVKCDevice(){return vkcDevice;};
	VKCPoint& getPhysicalDeviceList() {return vkcPhysicalDeviceList;};
	VKCuint& getVDIndex() {return vdIndex;};

	Member<WebVKC> getVKC() {return mVKC;};

	void submit(Member<WebVKCCommandQueue> commandBuffer, ExceptionState& ec);
	void wait(ExceptionState& ec);

	void deleteVKCBuffer(Member<WebVKCBuffer> buffer);
	void deleteCommandQueue(Member<WebVKCCommandQueue> queue);
	void deleteProgram(Member<WebVKCProgram> program);

	DECLARE_VIRTUAL_TRACE();

private:
	WebVKCDevice(VKCuint& vdIndex, VKCPoint& mPhysicalDeviceList, Member<WebVKC> vkc, ExceptionState& ec);

	VKCPoint vkcPhysicalDeviceList = 0;
	VKCPoint vkcDevice = 0;
	VKCPoint vkcQueue = 0;

	VKCuint vdIndex = 0;

	WebVKCBufferBindingMap mBufferBindingMap;
	WebVKCCommandQueueBindingMap mCommandQueueBindingMap;
	WebVKCProgramBindingMap mProgramBindingMap;
	Member<WebVKC> mVKC;

	bool releasing = false;
};

}

#endif //WebVKCDevice_h
