// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.accounts.Account;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.text.Html;
import android.text.TextUtils;
import android.util.Patterns;

import org.chromium.base.ContentUriUtils;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.sync.signin.AccountManagerHelper;

import java.io.File;

// sendAndroidBroadcast
import android.util.Log;
import android.os.Build;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;

/**
 * Helper for issuing intents to the android framework.
 */
public abstract class IntentHelper {

    private IntentHelper() {}

    /**
     * Triggers a send email intent.  If no application has registered to receive these intents,
     * this will fail silently.  If an email is not specified and the device has exactly one
     * account and the account name matches the email format, the email is set to the account name.
     *
     * @param context The context for issuing the intent.
     * @param email The email address to send to.
     * @param subject The subject of the email.
     * @param body The body of the email.
     * @param chooserTitle The title of the activity chooser.
     * @param fileToAttach The file name of the attachment.
     */
    @CalledByNative
    static void sendEmail(Context context, String email, String subject, String body,
            String chooserTitle, String fileToAttach) {
        if (TextUtils.isEmpty(email)) {
            Account[] accounts = AccountManagerHelper.get(context).getGoogleAccounts();
            if (accounts != null && accounts.length == 1
                    && Patterns.EMAIL_ADDRESS.matcher(accounts[0].name).matches()) {
                email = accounts[0].name;
            }
        }

        Intent send = new Intent(Intent.ACTION_SEND);
        send.setType("message/rfc822");
        if (!TextUtils.isEmpty(email)) send.putExtra(Intent.EXTRA_EMAIL, new String[] { email });
        send.putExtra(Intent.EXTRA_SUBJECT, subject);
        send.putExtra(Intent.EXTRA_TEXT, Html.fromHtml(body));
        if (!TextUtils.isEmpty(fileToAttach)) {
            File fileIn = new File(fileToAttach);
            Uri fileUri;
            // Attempt to use a content Uri, for greater compatibility.  If the path isn't set
            // up to be shared that way with a <paths> meta-data element, just use a file Uri
            // instead.
            try {
                fileUri = ContentUriUtils.getContentUriFromFile(context, fileIn);
            } catch (IllegalArgumentException ex) {
                fileUri = Uri.fromFile(fileIn);
            }
            send.putExtra(Intent.EXTRA_STREAM, fileUri);
        }

        try {
            Intent chooser = Intent.createChooser(send, chooserTitle);
            // we start this activity outside the main activity.
            chooser.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(chooser);
        } catch (android.content.ActivityNotFoundException ex) {
            // If no app handles it, do nothing.
        }
    }

    /**
     * Opens date and time in Android settings.
     *
     * @param context The context for issuing the intent.
     */
    @CalledByNative
    static void openDateAndTimeSettings(Context context) {
        Intent intent = new Intent(android.provider.Settings.ACTION_DATE_SETTINGS);

        try {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        } catch (android.content.ActivityNotFoundException ex) {
            // If it doesn't work, avoid crashing.
        }
    }

    /**
     * Send Android Broadcast
     */
    @CalledByNative
    static void sendAndroidBroadcast(Context context, String action, final int process_id_, final int routing_id_) {
        Log.e("IntentHelper", "=SAB=, IntentHelper.java, sendAndroidBroadcast, action : " + action + ", process_id_ " + process_id_ + ", routing_id_ : " + routing_id_);

        if(action.contains("<>")) {
      		String[] list = action.split("<>");
      		Intent intent = new Intent(list[0]);

      		if(list.length > 1) {
      			intent.putExtra("text", list[1]);
      		}

      		context.sendBroadcast(intent);
      	} else {
      		context.sendBroadcast(new Intent(action));
      	}

        BroadcastReceiver receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Log.e("IntentHelper", "=SAB=, IntentHelper.java, sendAndroidBroadcast, onReceive, action : " + action + ", process_id_ : " + process_id_ + ", routing_id_ : " + routing_id_);

                nativeOnReceiveAndroidBroadcast(action, process_id_, routing_id_);
                context.unregisterReceiver(this);
            }
        };

        String responseAction = "webcl.broadcast.response";
        IntentFilter filter = new IntentFilter();
        filter.addAction(responseAction);
        context.registerReceiver(receiver, filter);
        Log.e("IntentHelper", "=SAB=, IntentHelper.java, sendAndroidBroadcast, Register Response Action : " + responseAction);
        Log.e("IntentHelper", "=SAB=, IntentHelper.java, sendAndroidBroadcast, END");
    }

    private static native void nativeOnReceiveAndroidBroadcast(String action, int process_id_, int routing_id_);
}
