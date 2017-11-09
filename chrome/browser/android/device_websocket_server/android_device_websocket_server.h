// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_DEVICE_WEBSOCKET_SERVER_ANDROID_DEVICE_WEBSOCKET_SERVER_H_
#define CHROME_BROWSER_ANDROID_DEVICE_WEBSOCKET_SERVER_ANDROID_DEVICE_WEBSOCKET_SERVER_H_

#include <jni.h>

#include <string>

#include "base/macros.h"
#include "chrome/browser/device_websocket_server/device_websocket_server.h"

class AndroidDeviceWebsocketServer {
 public:
  AndroidDeviceWebsocketServer();
  ~AndroidDeviceWebsocketServer();

  void Start();

  void Stop();

  bool IsStarted() const;
  void appcet();

 private:
  std::string socket_name_;
  bool is_started_;

  DeviceWebsocketServer* server;

  DISALLOW_COPY_AND_ASSIGN(AndroidDeviceWebsocketServer);
};

bool RegisterDeviceWebsocketServer(JNIEnv* env);

#endif  // CHROME_BROWSER_ANDROID_DEVICE_WEBSOCKET_SERVER_ANDROID_DEVICE_WEBSOCKET_SERVER_H_
