// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "DeviceSoundListener.h"
#include "DeviceSound.h"
#include "DeviceSoundStatus.h"

#include "public/platform/modules/device_systeminformation/WebDeviceSoundStatus.h"
#include "public/platform/Platform.h"


namespace blink {

DeviceSoundListener* DeviceSoundListener::instance(DeviceSound* sound)
{
	DeviceSoundListener* callback = new DeviceSoundListener(sound);
	return callback;
}

DeviceSoundListener::DeviceSoundListener(DeviceSound* sound)
{
	mDeviceSound = sound;
}

DeviceSoundListener::~DeviceSoundListener()
{
	mDeviceSound = NULL;
	mSoundStatus = NULL;
}

DEFINE_TRACE(DeviceSoundListener)
{
	visitor->trace(mDeviceSound);
	visitor->trace(mSoundStatus);
}

void DeviceSoundListener::resultDeviceSound(const WebDeviceSoundStatus& soundStatus) {
	mSoundStatus = DeviceSoundStatus::create();
	mSoundStatus.get()->setResultCode(soundStatus.resultCode);
	switch(soundStatus.functionCode) {
		case WebDeviceSoundStatus::function::FUNC_DEVICE_VOLUME: {
			DeviceVolume volume = DeviceVolume();
			volume.setMediaVolume(soundStatus.volume.mediaVolume);
			volume.setNotificationVolume(soundStatus.volume.notificationVolume);
			volume.setAlarmVolume(soundStatus.volume.alarmVolume);
			volume.setBellVolume(soundStatus.volume.bellVolume);
			volume.setCallVolume(soundStatus.volume.callVolume);
			volume.setSystemVolume(soundStatus.volume.systemVolume);
			volume.setDTMFVolume(soundStatus.volume.DTMFVolume);

			mSoundStatus->setVolume(volume);
			break;
		}
		case WebDeviceSoundStatus::function::FUNC_OUTPUT_DEVICE_TYPE: {
			mSoundStatus->setOutputType(soundStatus.outputType);
			break;
		}
	}
	mDeviceSound->resultCodeCallback();
}

DeviceSoundStatus* DeviceSoundListener::lastData()
{
	if (mSoundStatus != nullptr)
		return mSoundStatus.get();
	else
		return nullptr;
}

} // namespace blink