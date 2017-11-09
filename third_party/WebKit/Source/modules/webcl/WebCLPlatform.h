// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLPlatform_H
#define WebCLPlatform_H

#include "platform/heap/Handle.h"

#include "platform/wtf/Vector.h"
#include "platform/wtf/text/WTFString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "WebCLInclude.h"
#include "WebCLDevice.h"

namespace blink {

class WebCL;

typedef HeapHashMap<cl_obj_key, Member<WebCLDevice>> WebCLDeviceMap;

class WebCLPlatform
	: public GarbageCollectedFinalized<WebCLPlatform>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebCLPlatform* create(WebCL* context, cl_platform_id platform_id)
	{
		return new WebCLPlatform(context, platform_id);
	}
	virtual ~WebCLPlatform();

	/*
	 * JS API
	 */
	HeapVector<Member<WebCLDevice>> getDevices(ExceptionState& ec);
	HeapVector<Member<WebCLDevice>> getDevices(unsigned device_type, ExceptionState& ec);

	ScriptValue getInfo (ScriptState*, int, ExceptionState&);
	WTF::Vector<WTF::String> getSupportedExtensions(ExceptionState&);
	CLboolean enableExtension(const String& extensionName, ExceptionState& ec);
	WTF::Vector<WTF::String>& getEnableExtensionList() {
		return enableExtensionList;
	};

	/*
	 * Internal functions
	 */
	cl_platform_id getPlatformId() { return m_cl_platform_id; };

	CLboolean containDevice(cl_obj_key, ExceptionState& ec);
	Member<WebCLDevice> findCLDevice(cl_obj_key, ExceptionState& ec);

	DEFINE_INLINE_TRACE() {
							visitor->Trace(m_context);
							visitor->Trace(m_device_list_type_DEFAULT);
							visitor->Trace(m_device_list_type_CPU); 
							visitor->Trace(m_device_list_type_GPU); 
							visitor->Trace(m_device_list_type_ACCELERATOR);
							visitor->Trace(m_device_list_type_ALL);
						}
private:
	WebCLPlatform(WebCL* context, cl_platform_id platform_id);

	Member<WebCL> m_context;
	cl_platform_id m_cl_platform_id;

	WebCLDeviceMap getDeviceList(WebCL* compute_context, cl_platform_id platform_id, unsigned device_type, ExceptionState& ec);

	WebCLDeviceMap m_device_list_type_DEFAULT;
	WebCLDeviceMap m_device_list_type_CPU;
	WebCLDeviceMap m_device_list_type_GPU;
	WebCLDeviceMap m_device_list_type_ACCELERATOR;
	WebCLDeviceMap m_device_list_type_ALL;

	WTF::Vector<WTF::String> supportedExtensionList;
	WTF::Vector<WTF::String> enableExtensionList;
	int initSupportedExtensionState;
	int initSupportedExtension(ExceptionState& ec);

};

}

#endif // WebCLPlatform_H
