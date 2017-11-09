// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "platform/wtf/build_config.h"
#include "platform/bindings/V8Binding.h"
#include "bindings/modules/v8/V8WebCLSampler.h"
#include "WebCLSampler.h"

#include "WebCL.h"
#include "WebCLContext.h"
#include "core/dom/custom/WebCL/WebCLException.h"

namespace blink {

WebCLSampler::~WebCLSampler()
{
}

WebCLSampler::WebCLSampler(WebCL* context, cl_sampler sampler)
		:mClSampler(sampler)
{
	mContext = context;
}

ScriptValue WebCLSampler::getInfo(ScriptState* scriptState, CLenum name, ExceptionState& ec)
{
	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();

	cl_sampler_info samplerInfo = name;
	cl_int err = 0;
	cl_uint clUInt = 0;
	cl_bool clBool = false;
	cl_context clContextId = NULL;
	Persistent<WebCLContext> contextObj;

	if (mClSampler == NULL) {
		ec.ThrowDOMException(WebCLException::INVALID_SAMPLER, "WebCLException::INVALID_SAMPLER");
		printf("Error: Invalid Sampler\n");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	switch(samplerInfo)
	{
		case WebCL::SAMPLER_REFERENCE_COUNT:
			err = gpu::webcl_clGetSamplerInfo (webcl_channel_, mClSampler, samplerInfo, sizeof(cl_uint), &clUInt, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(clUInt)));
			break;
		case WebCL::SAMPLER_NORMALIZED_COORDS:
			err = gpu::webcl_clGetSamplerInfo(webcl_channel_, mClSampler, samplerInfo, sizeof(cl_bool), &clBool, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Boolean::New(isolate, static_cast<bool>(clBool)));
			break;
		case WebCL::SAMPLER_CONTEXT:
			err = gpu::webcl_clGetSamplerInfo(webcl_channel_, mClSampler, samplerInfo, sizeof(cl_context), &clContextId, NULL);
			CLLOG(INFO) << "WebCLSampler::getInfo:SAMPLER_CONTEXT";
			CLLOG(INFO) << "WebCLSampler::getInfo:err : " << err << " : " << contextObj;

			if (err == CL_SUCCESS)
			{
				Persistent<WebCLContext> savedContext = mContext->findCLContext((cl_obj_key)clContextId);
				if(savedContext.Get() == NULL) {
					CLLOG(INFO) << ">>can't find saved context!!";

					contextObj = WebCLContext::create(mContext.Get(), clContextId);
					contextObj->initializeContextDevice(ec);
					return ScriptValue(scriptState, ToV8(contextObj, creationContext, isolate));
				}
				else {
					CLLOG(INFO) << ">>find saved context!!";

					return ScriptValue(scriptState, ToV8(savedContext, creationContext, isolate));
				}
			}
			break;
		case WebCL::SAMPLER_ADDRESSING_MODE:
			err = gpu::webcl_clGetSamplerInfo (webcl_channel_, mClSampler, samplerInfo, sizeof(cl_uint), &clUInt, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(clUInt)));
			break;
		case WebCL::SAMPLER_FILTER_MODE:
			err = gpu::webcl_clGetSamplerInfo (webcl_channel_, mClSampler, samplerInfo , sizeof(cl_uint), &clUInt, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(clUInt)));
			break;
		default:
			printf("Error: Unsupported Sampler Info type\n");
			ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			return ScriptValue(scriptState, v8::Null(isolate));
	}
	WebCLException::throwException(err, ec);
	return ScriptValue(scriptState, v8::Null(isolate));
}
void WebCLSampler::release( ExceptionState& ec)
{
	cl_int err = 0;
	if (mClSampler == NULL) {
		printf("Error: Invalid Sampler\n");
		ec.ThrowDOMException(WebCLException::INVALID_SAMPLER, "WebCLException::INVALID_SAMPLER");
		return;
	}
	err = webcl_clReleaseSampler(webcl_channel_, mClSampler);
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	} else {
		if (mClContext != NULL) {
			mClContext->removeCLSampler((cl_obj_key)mClSampler);
		}
		mClSampler = NULL;
		return;
	}
	return;
}

void WebCLSampler::setCLContext(WebCLContext* clContext)
{
	mClContext = clContext;
}

DEFINE_TRACE(WebCLSampler) {
	visitor->Trace(mContext);
	visitor->Trace(mClContext);
}

} // namespace WebCore
