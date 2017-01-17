// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "bindings/modules/v8/V8WebCLKernel.h"
#include "bindings/modules/v8/V8WebCLDevice.h"
#include "bindings/modules/v8/V8WebCLKernelArgInfo.h"

#include "modules/webcl/WebCLDevice.h"
#include "modules/webcl/WebCLKernelArgInfo.h"

namespace blink {


void V8WebCLKernel::getArgInfoMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	ExceptionState exceptionState(info.GetIsolate(), ExceptionState::ExecutionContext, "WebCLKernel", "getArgInfo");
	WebCLKernel* impl = V8WebCLKernel::toImpl((info.Holder()));
	int platform_index;
	platform_index = toInt32(info.GetIsolate(), info[0], NormalConversion, exceptionState);

	WebCLKernelArgInfo clInfo = impl->getArgInfo(platform_index, exceptionState);

	if (exceptionState.hadException()) {
		V8ThrowException::throwException(info.GetIsolate(), exceptionState.getException());
		return;
	}

	v8SetReturnValue(info, toV8(clInfo, info.Holder(), info.GetIsolate()));
}

} // namespace blink
