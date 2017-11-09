// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef DeviceSoundStatus_h
#define DeviceSoundStatus_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "modules/device_sound/DeviceVolume.h"

namespace blink {

class DeviceVolume;

class DeviceSoundStatus final : public GarbageCollectedFinalized<DeviceSoundStatus>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static DeviceSoundStatus* Create() {return new DeviceSoundStatus();};
	virtual ~DeviceSoundStatus();

	void setResultCode(int code);
	void setVolume(DeviceVolume& value);
	void setOutputType(int type);
	int resultCode();
	int outputType();
	ScriptValue volume(ScriptState*);

	DEFINE_INLINE_TRACE() {};

private:
	DeviceSoundStatus();
	
	int mResultCode = -1;
	int mOutputType = -1;
	DeviceVolume mVolume;
};

} // namespace blink

#endif // DeviceSoundStatus_h