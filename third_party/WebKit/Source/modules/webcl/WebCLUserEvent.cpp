// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "WebCL.h"

#include "WebCLUserEvent.h"
#include "core/dom/custom/WebCL/WebCLException.h"

namespace blink{

WebCLUserEvent::WebCLUserEvent() : mIsStatusSet(false) {
}

WebCLUserEvent::WebCLUserEvent(cl_event Event) : WebCLEvent(Event) {
}

WebCLUserEvent::WebCLUserEvent(WebCL* context) : WebCLEvent(context){
}

WebCLUserEvent::WebCLUserEvent(WebCL* context, cl_event Event) : WebCLEvent(context, Event) {
}

void WebCLUserEvent::setStatus(CLint executionStatus, ExceptionState& ec){
	cl_int err = -1;
	if (mClEvent == NULL) {
		printf("Error: Invalid CL Event\n");
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return;
	}

	if(mIsStatusSet) {
		ec.ThrowDOMException(WebCLException::INVALID_OPERATION, "WebCLException::INVALID_OPERATION");
		return;
	}

	// TODO (siba samal) To be uncommented for  OpenCL 1.1
	// http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clSetUserEventStatus.html
	if(executionStatus == WebCL::COMPLETE)	{
		err = webcl_clSetUserEventStatus(webcl_channel_, mClEvent , executionStatus);
	}
	else if (executionStatus < 0) {
		err = webcl_clSetUserEventStatus(webcl_channel_, mClEvent , executionStatus);
	}
	else
	{
		printf("Error: Invaild Error Type\n");
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	}

	mIsStatusSet = true;

	return;
}

DEFINE_TRACE(WebCLUserEvent) {
	WebCLEvent::Trace(visitor);

}

}
