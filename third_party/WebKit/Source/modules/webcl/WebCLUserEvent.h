// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLUserEvent_h
#define WebCLUserEvent_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "WebCLEvent.h"

namespace blink {

class WebCL;

class WebCLUserEvent : public WebCLEvent{
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebCLUserEvent* create() {
		return new WebCLUserEvent();
	}
	static WebCLUserEvent* create(cl_event event) {
		return new WebCLUserEvent(event);
	};
	static WebCLUserEvent* create(WebCL* context) {
		return new WebCLUserEvent(context);
	};
	static WebCLUserEvent* create(WebCL* context, cl_event event) {
		return new WebCLUserEvent(context, event);
	};

	void setStatus(CLint executionStatus, ExceptionState& ec);
	
	DECLARE_VIRTUAL_TRACE();
private:
	WebCLUserEvent(cl_event);
	WebCLUserEvent();
	WebCLUserEvent(WebCL*, cl_event);
	WebCLUserEvent(WebCL*);

	bool mIsStatusSet;
};

} // namespace WebCore

#endif // WebCLEvent_h

