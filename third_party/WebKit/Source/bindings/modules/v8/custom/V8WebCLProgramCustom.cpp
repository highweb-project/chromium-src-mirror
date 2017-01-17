// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"

#include "bindings/modules/v8/V8WebCLProgram.h"
#include "bindings/modules/v8/V8WebCLDevice.h"
#include "bindings/modules/v8/V8WebCLCallback.h"

#include "modules/webcl/WebCLDevice.h"

namespace blink {

void V8WebCLProgram::buildMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	ExceptionState es(info.GetIsolate(), ExceptionState::ExecutionContext, "WebCLProgram", "build");
	WebCLProgram* impl = V8WebCLProgram::toImpl(info.Holder());
	HeapVector<Member<WebCLDevice>> devices;
	V8StringResource<TreatNullAndUndefinedAsNullString> options;
	WebCLCallback* whenFinished = nullptr;
	{
		if (info.Length() > 0 && !isUndefinedOrNull(info[0])) {
			devices = toMemberNativeArray<WebCLDevice>(info[0], 1, info.GetIsolate(), es);
			if(es.hadException()) {
				V8ThrowException::throwException(info.GetIsolate(), es.getException());
				return;
			}
		}

		options = info[1];
		if(!options.prepare())
			return;

		if (!isUndefinedOrNull(info[2])) {
			if (!info[2]->IsFunction()) {
				es.throwTypeError("The callback provided as parameter 3 is not a function.");
				if (es.hadException()) {
					V8ThrowException::throwException(info.GetIsolate(), es.getException());
				}
				return;
			}

			whenFinished = V8WebCLCallback::create(v8::Handle<v8::Function>::Cast(info[2]), ScriptState::current(info.GetIsolate()));
		}
	}

	if (!devices.isEmpty())
		impl->build(devices, options, whenFinished, es);
	else
		impl->build(options, whenFinished, es);

	if (es.hadException()) {
		V8ThrowException::throwException(info.GetIsolate(), es.getException());
	}
}

} // namespace blink
