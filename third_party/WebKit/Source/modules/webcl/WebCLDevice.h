// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLDevice_h
#define WebCLDevice_h

#include "bindings/core/v8/Nullable.h"
#include "platform/bindings/ScriptWrappable.h"

#include "platform/wtf/Vector.h"
#include "platform/wtf/text/WTFString.h"

#include "WebCLInclude.h"
#include "WebCL.h"

namespace blink {

class ExceptionState;

class WebCLDevice : public GarbageCollectedFinalized<WebCLDevice>, public ScriptWrappable {
		DEFINE_WRAPPERTYPEINFO();
public:
	virtual ~WebCLDevice();
	static WebCLDevice* create(WebCL* context, cl_device_id device_id);
	ScriptValue getInfo(ScriptState*, unsigned, ExceptionState&);

	Nullable<WTF::Vector<WTF::String>> getSupportedExtensions(ExceptionState&);
	cl_device_id getCLDevice();
	CLboolean enableExtension(const String& extensionName, ExceptionState& ec);
	WTF::Vector<WTF::String>& getEnableExtensionList() {
		return enableExtensionList;
	};

	void setDefaultValue(ExceptionState&);
	unsigned getDeviceType();
	cl_ulong getDeviceMaxMemAllocSize();
	size_t getImage2DMaxWidth();
	size_t getImage2DMaxHeight();
	Vector<unsigned> getMaxWorkItemSizes();
	size_t getMaxWorkGroupSize();

	DEFINE_INLINE_TRACE() {
		visitor->Trace(m_context);
	}

	void ref() {}
	void deref() {}
private:
	WebCLDevice(WebCL* context, cl_device_id device_id);
	Member<WebCL> m_context;
	cl_device_id m_cl_device_id;

	WTF::Vector<WTF::String> supportedExtensionList;
	WTF::Vector<WTF::String> enableExtensionList;

	int initSupportedExtensionState;
	int initSupportedExtensions();

	unsigned mDeviceType;
	cl_ulong mMaxMemAllocSize;
	size_t mImage2DMaxWidth;
	size_t mImage2DMaxHeight;
	Vector<unsigned> mMaxWorkItemSizes;
	size_t mMaxWorkGroupSize;
};

}

#endif // WebCLDevice_h
