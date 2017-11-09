// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "platform/wtf/build_config.h"
#include "bindings/core/v8/V8ObjectBuilder.h"
#include "DeviceSoundStatus.h"

namespace blink {

	void DeviceSoundStatus::setResultCode(int code) {
		mResultCode = code;
	}
	void DeviceSoundStatus::setVolume(DeviceVolume& value) {
		mVolume = value;
	}

	void DeviceSoundStatus::setOutputType(int type) {
		mOutputType = type;
	}
	
	int DeviceSoundStatus::resultCode() {
		return mResultCode;
	}

	int DeviceSoundStatus::outputType() {
		return mOutputType;
	}

	ScriptValue DeviceSoundStatus::volume(ScriptState* state) {
		// return ScriptValue(state, ToV8(mVolume, state->GetContext(), state->GetIsolate()));
		V8ObjectBuilder object_builder(state);
		object_builder.AddNumber("MediaVolume", mVolume.MediaVolume());
		object_builder.AddNumber("NotificationVolume", mVolume.NotificationVolume());
		object_builder.AddNumber("AlarmVolume", mVolume.AlarmVolume());
		object_builder.AddNumber("BellVolume", mVolume.BellVolume());
		object_builder.AddNumber("CallVolume", mVolume.CallVolume());
		object_builder.AddNumber("SystemVolume", mVolume.SystemVolume());
		object_builder.AddNumber("DTMFVolume", mVolume.DTMFVolume());
		return object_builder.GetScriptValue();
	}


	DeviceSoundStatus::~DeviceSoundStatus() {
	}

	DeviceSoundStatus::DeviceSoundStatus() {
	}
} // namespace blink