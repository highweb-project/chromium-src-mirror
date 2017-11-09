// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "bindings/modules/v8/V8WebCLKernel.h"
#include "bindings/modules/v8/V8WebCLDevice.h"
#include "bindings/modules/v8/V8WebCLKernelArgInfo.h"

#include "modules/webcl/WebCLDevice.h"
#include "modules/webcl/WebCLKernelArgInfo.h"

namespace blink {


void V8WebCLKernel::getArgInfoMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	ExceptionState exceptionState(info.GetIsolate(), ExceptionState::kExecutionContext, "WebCLKernel", "getArgInfo");
	WebCLKernel* impl = V8WebCLKernel::toImpl((info.Holder()));
	int platform_index;
	platform_index = ToInt32(info.GetIsolate(), info[0], kNormalConversion, exceptionState);

	WebCLKernelArgInfo clInfo = impl->getArgInfo(platform_index, exceptionState);

	if (exceptionState.HadException()) {
		V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
		return;
	}

	V8SetReturnValue(info, ToV8(clInfo, info.Holder(), info.GetIsolate()));
}

} // namespace blink
