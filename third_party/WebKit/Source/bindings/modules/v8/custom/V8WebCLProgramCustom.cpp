// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "bindings/modules/v8/V8WebCLProgram.h"
#include "bindings/modules/v8/V8WebCLDevice.h"
#include "bindings/modules/v8/V8WebCLCallback.h"
#include "bindings/core/v8/NativeValueTraitsImpl.h"

#include "modules/webcl/WebCLDevice.h"

namespace blink {

void V8WebCLProgram::buildMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	ExceptionState es(info.GetIsolate(), ExceptionState::kExecutionContext, "WebCLProgram", "build");
	WebCLProgram* impl = V8WebCLProgram::toImpl(info.Holder());
	HeapVector<Member<WebCLDevice>> devices;
	V8StringResource<kTreatNullAndUndefinedAsNullString> options;
	WebCLCallback* whenFinished = nullptr;
	{
		if (info.Length() > 0 && !IsUndefinedOrNull(info[0])) {
			devices = NativeValueTraits<IDLSequence<WebCLDevice>>::NativeValue(info.GetIsolate(), info[0], es);
			if(es.HadException()) {
				V8ThrowException::ThrowException(info.GetIsolate(), es.GetException());
				return;
			}
		}

		options = info[1];
		if(!options.Prepare())
			return;

		if (!IsUndefinedOrNull(info[2])) {
			if (!info[2]->IsFunction()) {
				es.ThrowTypeError("The callback provided as parameter 3 is not a function.");
				if (es.HadException()) {
					V8ThrowException::ThrowException(info.GetIsolate(), es.GetException());
				}
				return;
			}

			whenFinished = V8WebCLCallback::Create(v8::Handle<v8::Function>::Cast(info[2]), ScriptState::Current(info.GetIsolate()));
		}
	}

	if (!devices.IsEmpty())
		impl->build(devices, options, whenFinished, es);
	else
		impl->build(options, whenFinished, es);

	if (es.HadException()) {
		V8ThrowException::ThrowException(info.GetIsolate(), es.GetException());
	}
}

} // namespace blink
