// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.webapps;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Base64;
import android.os.AsyncTask;
import android.graphics.Bitmap;

import org.chromium.base.ApiCompatibilityUtils;
import org.chromium.base.ContextUtils;
import org.chromium.base.Log;
import org.chromium.base.ThreadUtils;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.chrome.browser.webapps.WebappAuthenticator;
import org.chromium.chrome.browser.webapps.WebappLauncherActivity;
import org.chromium.content_public.common.ScreenOrientationConstants;
import org.chromium.chrome.browser.ShortcutHelper;
import org.chromium.chrome.browser.webapps.WebappDataStorage;
import org.chromium.chrome.browser.webapps.WebappRegistry;

public class ManifestUtils {
    @SuppressWarnings("unused")
    @CalledByNative
    private static void launchPWA(final String id, final String url, final String scopeUrl,
            final String userTitle, final String name, final String shortName, final String iconUrl,
            final int displayMode, final int orientation, final int source,
            final String themeColor, final String backgroundColor) {
              Log.d("chromium", "launchPWA : " + id + ", " + url);
              Context context = ContextUtils.getApplicationContext();
              long longThemeColor = 0, longBackgroundColor = 0;
              if (!themeColor.isEmpty()) {
                longThemeColor = Long.parseLong(themeColor);
              }
              if (!backgroundColor.isEmpty()){
                longBackgroundColor = Long.parseLong(backgroundColor);
              }
              Intent intent = new Intent();
              intent.setAction(WebappLauncherActivity.ACTION_START_WEBAPP)
                      .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
                      .putExtra(ShortcutHelper.EXTRA_ID, id)
                      .putExtra(ShortcutHelper.EXTRA_URL, url)
                      .putExtra(ShortcutHelper.EXTRA_SCOPE, scopeUrl)
                      .putExtra(ShortcutHelper.EXTRA_NAME, name)
                      .putExtra(ShortcutHelper.EXTRA_SHORT_NAME, shortName)
                      .putExtra(ShortcutHelper.EXTRA_VERSION, ShortcutHelper.WEBAPP_SHORTCUT_VERSION)
                      .putExtra(ShortcutHelper.EXTRA_DISPLAY_MODE, displayMode)
                      .putExtra(ShortcutHelper.EXTRA_ORIENTATION, orientation)
                      .putExtra(ShortcutHelper.EXTRA_THEME_COLOR, longThemeColor)
                      .putExtra(ShortcutHelper.EXTRA_BACKGROUND_COLOR, longBackgroundColor)
                      .putExtra(ShortcutHelper.EXTRA_IS_ICON_GENERATED, iconUrl.isEmpty())
                      .putExtra(ShortcutHelper.EXTRA_MAC, ShortcutHelper.getEncodedMac(context, url))
                      .putExtra(ShortcutHelper.EXTRA_SOURCE, source);
              intent.setPackage(context.getPackageName());
              context.startActivity(intent);
    }

    @SuppressWarnings("unused")
    @CalledByNative
    private static void launchShortcut(final String url, final int source) {
              Log.d("chromium", "launchShortcut : " +  url);
              Context context = ContextUtils.getApplicationContext();
              Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
              intent.putExtra(ShortcutHelper.REUSE_URL_MATCHING_TAB_ELSE_NEW_TAB, true);
              intent.putExtra(ShortcutHelper.EXTRA_SOURCE, source);
              intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
              intent.setPackage(context.getPackageName());
              context.startActivity(intent);
    }

    @SuppressWarnings("unused")
    @CalledByNative
    private static void createWebappDataStorage(final String id) {
      Log.d("chromium", "createWebappDataStorage : " + id);
      WebappRegistry.getInstance().register(
                  id, new WebappRegistry.FetchWebappDataStorageCallback() {
                      @Override
                      public void onWebappDataStorageRetrieved(WebappDataStorage storage) {
                        Log.d("chromium", "onWebappDataStorageRetrieved " + storage);
                      }
                  });
    }

    @SuppressWarnings("unused")
    @CalledByNative
    private static void deleteWebappDataStorage(final String id) {
      Log.d("chromium", "DeleteWebappDataStorage : " + id);
      final WebappDataStorage storage = WebappRegistry.getInstance().getWebappDataStorage(id);
      if (storage != null) {
        Log.d("chromium", "storage is not null storage delete");
        storage.delete();
      } else {
        Log.w("chromium", "storage is null " + id);
      }
    }
}


