// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.app.Activity;

import org.chromium.chrome.browser.infobar.InfoBarIdentifier;
import org.chromium.chrome.browser.infobar.SimpleConfirmInfoBarBuilder;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.ui.base.ActivityWindowAndroid;

import java.util.Vector;

import org.chromium.base.Log;

/**
 * The window that has access to the main activity and is able to create and receive intents,
 * and show error messages.
 */
public class ChromeWindow extends ActivityWindowAndroid {
    /**
     * Creates Chrome specific ActivityWindowAndroid.
     * @param activity The activity that owns the ChromeWindow.
     */
    public ChromeWindow(ChromeActivity activity) {
        super(activity);
        try{
            String[] permissionList = {
                "android.permission.WRITE_EXTERNAL_STORAGE",
                "android.permission.READ_CALENDAR",
                "android.permission.WRITE_CALENDAR",
                "android.permission.READ_CONTACTS",
                "android.permission.WRITE_CONTACTS",
                "android.permission.READ_SMS",
                "android.permission.SEND_SMS",
                "android.permission.RECEIVE_SMS",
                "android.permission.READ_PHONE_STATE" };
            ;
            Vector<String> requestPermission = new Vector<String>();
            for(String permission : permissionList) {
                if (!hasPermission(permission) && canRequestPermission(permission)) {
                    requestPermission.add(permission);
                }
            }
            if (!requestPermission.isEmpty()) {
                requestPermissions(requestPermission.toArray(new String[requestPermission.size()]), null);
            }
        } catch(Exception e) {
            Log.e("chromium", "request permission error");
            e.printStackTrace();
        }
    }

    /**
     * Shows an infobar error message overriding the WindowAndroid implementation.
     */
    @Override
    protected void showCallbackNonExistentError(String error) {
        Activity activity = getActivity().get();

        // We can assume that activity is a ChromeActivity because we require one to be passed in
        // in the constructor.
        Tab tab = activity != null ? ((ChromeActivity) activity).getActivityTab() : null;

        if (tab != null) {
            SimpleConfirmInfoBarBuilder.create(
                    tab, InfoBarIdentifier.CHROME_WINDOW_ERROR, error, false);
        } else {
            super.showCallbackNonExistentError(error);
        }
    }
}
