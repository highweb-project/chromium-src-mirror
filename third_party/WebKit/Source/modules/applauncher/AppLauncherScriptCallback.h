// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AppLauncherScriptCallback_h
#define AppLauncherScriptCallback_h

#include "platform/wtf/text/WTFString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "AppStatus.h"

namespace blink {

class AppStatus;

class AppLauncherScriptCallback : public GarbageCollectedFinalized<AppLauncherScriptCallback> {
public:
	virtual ~AppLauncherScriptCallback() { }
	virtual void handleEvent(AppStatus* data) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif // AppLauncherScriptCallback_h
