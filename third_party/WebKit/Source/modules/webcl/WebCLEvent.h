// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLEvent_h
#define WebCLEvent_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include <wtf/HashMap.h>
#include <wtf/text/WTFString.h>

#include "WebCLInclude.h"

namespace blink {

class WebCL;
class WebCLContext;
class WebCLCallback;
class WebCLCommandQueue;

class WebCLEvent : public GarbageCollected<WebCLEvent>, public ScriptWrappable{
	DEFINE_WRAPPERTYPEINFO();
public:

	static WebCLEvent* create() {
		return new WebCLEvent();
	}
	static WebCLEvent* create(cl_event event) {
		return new WebCLEvent(event);
	};
	static WebCLEvent* create(WebCL* context) {
		return new WebCLEvent(context);
	};
	static WebCLEvent* create(WebCL* context, cl_event event) {
		return new WebCLEvent(context, event);
	};

	ScriptValue getInfo(ScriptState*, int, ExceptionState&);
	ScriptValue getProfilingInfo(ScriptState*, int, ExceptionState&);
	void setCallback(CLenum commandExecCallbackType, WebCLCallback* notify, ExceptionState& ec);
	void release(ExceptionState&);

	cl_event getCLEvent();
	void setCLEvent(cl_event event);
	void setCLContext(WebCLContext* context) { mClContext = context; };
	WebCLContext* getCLContext() { return mClContext; }
	
	void triggerCallback(int callbackType);

	void setIsUserEvent() { mIsUserEvent = true; };
	bool isUserEvent() { return mIsUserEvent; };

	void setHandlerKey(int key) { mHandlerKey = key; };

	int getEventCommandExecutionStatus(ExceptionState&);

	DECLARE_VIRTUAL_TRACE();
private:

	WebCLCommandQueue* getEventCommandQueue(ExceptionState&);

	Member<WebCL> mContext;
	Member<WebCLContext> mClContext;

	Member<WebCLCallback> mCompleteCallback;
	Member<WebCLCallback> mRunningCallback;
	Member<WebCLCallback> mSubmittedCallback;
	Member<WebCLCallback> mQueuedCallback;

	int mHandlerKey;
protected:
	cl_event mClEvent;
	bool mIsUserEvent = false;
	WebCLEvent(cl_event);
	WebCLEvent();
	WebCLEvent(WebCL*);
	WebCLEvent(WebCL*, cl_event);
};

}
#endif // WebCLEvent_h
