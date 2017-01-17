// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLCallback_h
#define WebCLCallback_h

#include "WebCL.h"
#include "wtf/text/WTFString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"

namespace blink {
class WebCL;    
	
class WebCLCallback : public GarbageCollectedFinalized<WebCLCallback>{
public:
	virtual ~WebCLCallback() { }
	virtual void handleEvent() = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};
	
}

#endif // WebCLCallback_H
