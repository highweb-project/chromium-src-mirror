// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceCpuScriptCallback_h
#define DeviceCpuScriptCallback_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "DeviceCpuStatus.h"

namespace blink {

class DeviceCpuStatus;

class DeviceCpuScriptCallback : public GarbageCollectedFinalized<DeviceCpuScriptCallback> {
public:
	virtual ~DeviceCpuScriptCallback() { }
	virtual void handleEvent(DeviceCpuStatus* data) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif // DeviceCpuScriptCallback_h
