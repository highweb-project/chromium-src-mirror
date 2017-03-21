// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.thirdparty;
import org.chromium.device.DeviceThirdpartyManager;
import org.chromium.device.SendAndroidBroadcastCallbackData;
import org.chromium.services.service_manager.InterfaceFactory;
import org.chromium.mojo.system.MojoException;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;

public class DeviceThirdpartyManagerImpl implements DeviceThirdpartyManager {
  Context mContext = null;

  class thirdpartyBroadcastReceiver extends BroadcastReceiver {
    SendAndroidBroadcastResponse mCallback = null;
    public thirdpartyBroadcastReceiver(SendAndroidBroadcastResponse callback) {
      super();
      mCallback = callback;
    }

    @Override
      public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        //web2app
        String result = intent.getStringExtra("result");
        Log.e("IntentHelper", "=SAB=, IntentHelper.java, sendAndroidBroadcast, onReceive, result : " + result);

        if(result == null) {
          result = "";
        }

        SendAndroidBroadcastCallbackData code = new SendAndroidBroadcastCallbackData();
		    code.action = result;
		    mCallback.call(code);
		    code = null;
        context.unregisterReceiver(this);
      }
  }

	public DeviceThirdpartyManagerImpl(Context context) {
		mContext = context;
	}

	@Override
	public void close() {}

  @Override
  public void onConnectionError(MojoException e) {}

  public void sendAndroidBroadcast(String action, SendAndroidBroadcastResponse callback) {
    if(action.contains("<>")) {
      String[] list = action.split("<>");
      Intent intent = new Intent(list[0]);

      if(list.length > 1) {
        intent.putExtra("text", list[1]);
      }
      Log.d("IntentHelper", "SendBroadcast");
      mContext.sendBroadcast(intent);
    } else {
      Log.d("IntentHelper", "SendBroadcast");
      mContext.sendBroadcast(new Intent(action));
    }

    thirdpartyBroadcastReceiver receiver = new thirdpartyBroadcastReceiver(callback);
    // String responseAction = "webcl.broadcast.response";
    String responseAction = "web2app.response";
    IntentFilter filter = new IntentFilter();
    filter.addAction(responseAction);
    mContext.registerReceiver(receiver, filter);
    Log.e("IntentHelper", "=SAB=, ThirdPartyBroadcast.java, sendAndroidBroadcast, Register Response Action : " + responseAction);
    Log.e("IntentHelper", "=SAB=, ThirdPartyBroadcast.java, sendAndroidBroadcast, END");
  }

  /**
	 * A factory for implementations of the DeviceThirdpartyManager interface.
	 */
	public static class Factory implements InterfaceFactory<DeviceThirdpartyManager> {
			private Context mContext;
			public Factory(Context context) {
					mContext = context;
			}

			@Override
			public DeviceThirdpartyManagerImpl createImpl() {
					return new DeviceThirdpartyManagerImpl(mContext);
			}
	}
}
