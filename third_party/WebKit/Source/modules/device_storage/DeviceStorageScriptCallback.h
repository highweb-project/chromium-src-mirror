// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DeviceStorageScriptCallback_h
#define DeviceStorageScriptCallback_h

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "DeviceStorageStatus.h"

namespace blink {

class DeviceStorageStatus;

class DeviceStorageScriptCallback : public GarbageCollectedFinalized<DeviceStorageScriptCallback> {
public:
	virtual ~DeviceStorageScriptCallback() { }
	virtual void handleEvent(DeviceStorageStatus* data) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif // DeviceStorageScriptCallback_h
