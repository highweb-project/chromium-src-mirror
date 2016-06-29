// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.sound;
import org.chromium.mojom.device.DeviceSoundManager;
import org.chromium.mojom.device.DeviceSoundResultCode;
import org.chromium.mojom.device.DeviceSoundVolume;

import android.media.AudioManager;
import android.content.Context;
import android.util.Log;
import java.lang.Exception;

import org.chromium.mojo.system.MojoException;


/**
 * Android implementation of the devicesound manager service defined in
 * device/sound/devicesound_manager.mojom.
 */
public class DeviceSoundManagerImpl implements DeviceSoundManager {
	private static final String TAG = "chromium";
	private Context mContext;

	static class devicesound_ErrorCodeList{
	// Exception code
		static final int SUCCESS = 0;
		static final int FAILURE = -1;
		static final int NOT_ENABLED_PERMISSION = -2;
	};

	static class devicesound_function {
		static final int FUNC_DEVICE_VOLUME = 1;
		static final int FUNC_OUTPUT_DEVICE_TYPE = 2;
	};

	static class device_outputtype {
		static final int DEVICE_SPEAKER_PHONE = 1;
		static final int DEVICE_WIRED_HEADSET = 2;
		static final int DEVICE_BLUETOOTH = 3;
		static final int DEVICE_DEFAULT = 4;
	};

	public DeviceSoundManagerImpl(Context context) {
		mContext = context;
	}

	@Override
	public void close() {}

	@Override
	public void onConnectionError(MojoException e) {}

	@Override
	public void outputDeviceType(OutputDeviceTypeResponse callback) {
		DeviceSoundResultCode code = new DeviceSoundResultCode();
		code.resultCode = devicesound_ErrorCodeList.FAILURE;
		code.functionCode = devicesound_function.FUNC_OUTPUT_DEVICE_TYPE;
		code.outputType = device_outputtype.DEVICE_DEFAULT;
		code.volume = new DeviceSoundVolume();

		AudioManager am = (AudioManager)mContext.getSystemService(Context.AUDIO_SERVICE);

		if (am.isWiredHeadsetOn()) {
			code.outputType = device_outputtype.DEVICE_WIRED_HEADSET;
		} else if(am.isBluetoothA2dpOn()) {
			code.outputType = device_outputtype.DEVICE_BLUETOOTH;
		} else if(am.isSpeakerphoneOn()) {
			code.outputType = device_outputtype.DEVICE_SPEAKER_PHONE;
		} else {
			code.outputType = device_outputtype.DEVICE_DEFAULT;
		}
		
		code.resultCode = devicesound_ErrorCodeList.SUCCESS;
		callback.call(code);
		code = null;
	}

	@Override
	public void deviceVolume(DeviceVolumeResponse callback) {
		DeviceSoundResultCode code = new DeviceSoundResultCode();
		code.resultCode = devicesound_ErrorCodeList.FAILURE;

		code.functionCode = devicesound_function.FUNC_DEVICE_VOLUME;
		code.volume = new DeviceSoundVolume();

		float maxVolume = 0;
		float nowVolume = 0;

		AudioManager am = (AudioManager)mContext.getSystemService(Context.AUDIO_SERVICE);
		
		maxVolume = am.getStreamMaxVolume(AudioManager.STREAM_ALARM);
		nowVolume = am.getStreamVolume(AudioManager.STREAM_ALARM);
		code.volume.alarmVolume = nowVolume / maxVolume;

		maxVolume = am.getStreamMaxVolume(AudioManager.STREAM_DTMF);
		nowVolume = am.getStreamVolume(AudioManager.STREAM_DTMF);
		code.volume.dtmfVolume = nowVolume / maxVolume;

		maxVolume = am.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
		nowVolume = am.getStreamVolume(AudioManager.STREAM_MUSIC);
		code.volume.mediaVolume = nowVolume / maxVolume;

		maxVolume = am.getStreamMaxVolume(AudioManager.STREAM_NOTIFICATION);
		nowVolume = am.getStreamVolume(AudioManager.STREAM_NOTIFICATION);
		code.volume.notificationVolume = nowVolume / maxVolume;

		maxVolume = am.getStreamMaxVolume(AudioManager.STREAM_RING);
		nowVolume = am.getStreamVolume(AudioManager.STREAM_RING);
		code.volume.bellVolume = nowVolume / maxVolume;

		maxVolume = am.getStreamMaxVolume(AudioManager.STREAM_SYSTEM);
		nowVolume = am.getStreamVolume(AudioManager.STREAM_SYSTEM);
		code.volume.systemVolume = nowVolume / maxVolume;

		maxVolume = am.getStreamMaxVolume(AudioManager.STREAM_VOICE_CALL);
		nowVolume = am.getStreamVolume(AudioManager.STREAM_VOICE_CALL);
		code.volume.callVolume = nowVolume / maxVolume;

		code.resultCode = devicesound_ErrorCodeList.SUCCESS;
		callback.call(code);
		code = null;
	}

}
