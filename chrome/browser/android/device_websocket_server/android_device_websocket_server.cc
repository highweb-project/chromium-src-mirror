// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/device_websocket_server/android_device_websocket_server.h"

#include "base/android/jni_string.h"
#include "jni/DeviceWebsocketServer_jni.h"

using base::android::JavaParamRef;

AndroidDeviceWebsocketServer::AndroidDeviceWebsocketServer()
{
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  server = new DeviceWebsocketServer(nullptr);
#else
  server = nullptr;
#endif
}

AndroidDeviceWebsocketServer::~AndroidDeviceWebsocketServer() {
}

void AndroidDeviceWebsocketServer::Start() {
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  server->Start();
#endif
}

void AndroidDeviceWebsocketServer::Stop() {
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  server->Stop();
#endif
}

bool AndroidDeviceWebsocketServer::IsStarted() const {
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  return server->IsStarted();
#else
  return false;
#endif
}

bool RegisterDeviceWebsocketServer(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

static jlong InitDeviceWebsocketServer(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  AndroidDeviceWebsocketServer* server = new AndroidDeviceWebsocketServer();
  return reinterpret_cast<intptr_t>(server);
#else
  return 0;
#endif
}

static void SetDeviceWebsocketEnabled(JNIEnv* env,
                                      const JavaParamRef<jobject>& obj,
                                      jlong server,
                                      jboolean enabled) {
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  AndroidDeviceWebsocketServer* websocket_server = reinterpret_cast<AndroidDeviceWebsocketServer*>(server);
  if (enabled) {
    websocket_server->Start();
  } else {
    websocket_server->Stop();
  }
#endif
}
