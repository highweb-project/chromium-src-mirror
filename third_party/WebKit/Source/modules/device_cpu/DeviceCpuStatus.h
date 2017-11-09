// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef DeviceCpuStatus_h
#define DeviceCpuStatus_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"

namespace blink {

class DeviceCpuStatus final : public GarbageCollectedFinalized<DeviceCpuStatus>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static DeviceCpuStatus* create() {return new DeviceCpuStatus();};
	virtual ~DeviceCpuStatus();

	void setResultCode(int code);
	void setLoad(double load);
	int resultCode();
	double load();

	void setFunctionCode(int function);
	int getFunctionCode();

	DEFINE_INLINE_TRACE() {};

private:
	DeviceCpuStatus();
	
	int mResultCode = -1;
	double mLoad = -1;
	int mFunctionCode = -1;
};

} // namespace blink

#endif // DeviceCpuStatus_h