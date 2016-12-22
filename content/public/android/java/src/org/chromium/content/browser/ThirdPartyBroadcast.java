// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

@JNINamespace("content")
class ThirdPartyBroadcast {

    @CalledByNative
    static void SendAndroidBroadcast(Context context, String action, final int process_id_, final int routing_id_) {
      Log.e("IntentHelper", "=SAB=, ThirdPartyBroadcast.java, sendAndroidBroadcast, action : " + action + ", process_id_ " + process_id_ + ", routing_id_ : " + routing_id_);

      if(action.contains("<>")) {
        String[] list = action.split("<>");
        Intent intent = new Intent(list[0]);

        if(list.length > 1) {
          intent.putExtra("text", list[1]);
        }
        Log.d("IntentHelper", "SendBroadcast");
        context.sendBroadcast(intent);
      } else {
        Log.d("IntentHelper", "SendBroadcast");
        context.sendBroadcast(new Intent(action));
      }

      BroadcastReceiver receiver = new BroadcastReceiver() {
          @Override
          public void onReceive(Context context, Intent intent) {
              String action = intent.getAction();
              Log.e("IntentHelper", "=SAB=, ThirdPartyBroadcast.java, sendAndroidBroadcast, onReceive, action : " + action + ", process_id_ : " + process_id_ + ", routing_id_ : " + routing_id_);

              ThirdPartyBroadcast.nativeReceiveAndroidBroadcast(action, process_id_, routing_id_);
              context.unregisterReceiver(this);
          }
      };

      String responseAction = "webcl.broadcast.response";
      IntentFilter filter = new IntentFilter();
      filter.addAction(responseAction);
      context.registerReceiver(receiver, filter);
      Log.e("IntentHelper", "=SAB=, ThirdPartyBroadcast.java, sendAndroidBroadcast, Register Response Action : " + responseAction);
      Log.e("IntentHelper", "=SAB=, ThirdPartyBroadcast.java, sendAndroidBroadcast, END");
    }

    private static native void nativeReceiveAndroidBroadcast(String action, int process_id_, int routing_id_);
}
