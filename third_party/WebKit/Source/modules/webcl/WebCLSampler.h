// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLSampler_h
#define WebCLSampler_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "platform/heap/Handle.h"

#include "WebCLInclude.h"

namespace blink {
class WebCL;
class WebCLContext;

class WebCLSampler : public GarbageCollectedFinalized<WebCLSampler>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebCLSampler* create(WebCL* clContext, cl_sampler sampler) {
		return new WebCLSampler(clContext, sampler);
	}
	virtual ~WebCLSampler();
	ScriptValue getInfo(ScriptState* scriptState, CLenum name, ExceptionState&);
	void release(ExceptionState& ec);

	cl_sampler getSampler() { return mClSampler; }

	void setCLContext(WebCLContext*);

	DECLARE_TRACE();

private:
	WebCLSampler(WebCL* context, cl_sampler sampler);
	
	Member<WebCL> mContext;
	Member<WebCLContext> mClContext;

	cl_sampler mClSampler = NULL;
};

} // namespace WebCore

#endif // WebCLSampler_h
