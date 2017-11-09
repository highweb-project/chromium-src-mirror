// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLImage_h
#define WebCLImage_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

#include "WebCLInclude.h"
#include "WebCLMemoryObject.h"
#include "modules/webcl/WebCLImageDescriptor.h"

namespace blink {

class WebCL;
class WebCLContext;
class WebCLImageDescriptor;

class WebCLImage final : public WebCLMemoryObject {
	DEFINE_WRAPPERTYPEINFO();
public:
	static WebCLImage* create(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared) {
		return new WebCLImage(context, clContext, memory, isShared);
	}
	virtual ~WebCLImage();

	/*
	 * JS API Define
	 */
	void getInfo(WebCLImageDescriptor& descriptor);

	/*
	 * Internal functions
	 */
	WebCLImageDescriptor getImageDescriptor() { return mImageDescriptor; };

	DECLARE_VIRTUAL_TRACE();

private:
	WebCLImage(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared);

	WebCLImageDescriptor mImageDescriptor;
};

}

#endif
