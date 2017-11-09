// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceSoundScriptCallback_h
#define DeviceSoundScriptCallback_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "DeviceSoundStatus.h"

namespace blink {

class DeviceSoundStatus;

class DeviceSoundScriptCallback : public GarbageCollectedFinalized<DeviceSoundScriptCallback> {
public:
	virtual ~DeviceSoundScriptCallback() { }
	virtual void handleEvent(DeviceSoundStatus* data) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif // DeviceSoundScriptCallback_h
