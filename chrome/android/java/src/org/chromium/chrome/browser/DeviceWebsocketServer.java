// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.content.Context;

import org.chromium.base.ApiCompatibilityUtils;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.Log;

public class DeviceWebsocketServer {

    private long mNativeDeviceSocketServer;

    public DeviceWebsocketServer() {
        mNativeDeviceSocketServer = nativeInitDeviceWebsocketServer();
    }

    public void destroy() {
    }

    public void setDeviceWebsocketEnabled(boolean enabled) {
        nativeSetDeviceWebsocketEnabled(mNativeDeviceSocketServer, enabled);
    }

    private native long nativeInitDeviceWebsocketServer();
    private native void nativeSetDeviceWebsocketEnabled(
            long websocketServer, boolean enabled);
}
