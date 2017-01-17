// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "bindings/modules/v8/V8AppLauncher.h"
#include "modules/applauncher/AppLauncher.h"
#include "bindings/modules/v8/V8AppLauncherScriptCallback.h"
#include "base/logging.h"

namespace blink {

void V8AppLauncher::getAppListMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	AppLauncher* impl = V8AppLauncher::toImpl((info.Holder()));
	V8StringResource<> V8mimeType;
	AppLauncherScriptCallback* callback;
	if(info.Length() == 2) {
		V8mimeType = info[0];
		if (!V8mimeType.prepare())
			return;
		if (!info[1]->IsFunction()) {
			V8ThrowException::throwTypeError(info.GetIsolate(), ExceptionMessages::failedToExecute("getAppList", "Navigator", "The callback provided as parameter 2 is not a function."));
			return;
		}
		callback = V8AppLauncherScriptCallback::create(v8::Local<v8::Function>::Cast(info[1]), ScriptState::current(info.GetIsolate()));
	}
	else if(info.Length() == 1) {
		if (!info[0]->IsFunction()) {
			V8ThrowException::throwTypeError(info.GetIsolate(), ExceptionMessages::failedToExecute("getAppList", "Navigator", "The callback provided as parameter 1 is not a function."));
			return;
		} else {
			V8mimeType = nullptr;
			callback = V8AppLauncherScriptCallback::create(v8::Local<v8::Function>::Cast(info[0]), ScriptState::current(info.GetIsolate()));
		}
	}
	else {
		V8ThrowException::throwTypeError(info.GetIsolate(),
			ExceptionMessages::failedToConstruct("getAppList", ExceptionMessages::notEnoughArguments(1, info.Length())));
		return;
	}
	impl->getAppList(V8mimeType, callback);
}

void V8AppLauncher::getApplicationInfoMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	ExceptionState exceptionState(info.GetIsolate(), ExceptionState::ExecutionContext, "AppLauncher", "getApplicationInfo");
	AppLauncher* impl = V8AppLauncher::toImpl((info.Holder()));
	V8StringResource<> V8PackageName;
	int flags = 0;
	AppLauncherScriptCallback* callback;
	if(info.Length() == 3) {
		V8PackageName = info[0];
		if (!V8PackageName.prepare())
			return;
		flags = toInt32(info.GetIsolate(), info[1], EnforceRange, exceptionState);
		if (exceptionState.hadException()) {
			V8ThrowException::throwTypeError(info.GetIsolate(), ExceptionMessages::failedToExecute("getApplicationInfo", "Navigator", "The flags provided as parameter 2 is not a valid flags."));
			return;
		}

		if (!info[2]->IsFunction()) {
			V8ThrowException::throwTypeError(info.GetIsolate(), ExceptionMessages::failedToExecute("getApplicationInfo", "Navigator", "The callback provided as parameter 3 is not a function."));
			return;
		}
		callback = V8AppLauncherScriptCallback::create(v8::Local<v8::Function>::Cast(info[2]), ScriptState::current(info.GetIsolate()));
	}
	else if(info.Length() == 2) {
		if (!info[1]->IsFunction()) {
			V8ThrowException::throwTypeError(info.GetIsolate(), ExceptionMessages::failedToExecute("getApplicationInfo", "Navigator", "The callback provided as parameter 2 is not a function."));
			return;
		} else {
			V8PackageName = info[0];
			if (!V8PackageName.prepare())
				return;
			callback = V8AppLauncherScriptCallback::create(v8::Local<v8::Function>::Cast(info[1]), ScriptState::current(info.GetIsolate()));
		}
	}
	else {
		V8ThrowException::throwTypeError(info.GetIsolate(),
			ExceptionMessages::failedToConstruct("getApplicationInfo", ExceptionMessages::notEnoughArguments(1, info.Length())));
		return;
	}
	impl->getApplicationInfo(V8PackageName, flags, callback);
}

} // namespace blink
