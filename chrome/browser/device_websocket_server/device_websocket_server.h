// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DEVICE_WEBSOCKET_SERVER_DEVICE_WEBSOCKET_SERVER_H_
#define CHROME_BROWSER_DEVICE_WEBSOCKET_SERVER_DEVICE_WEBSOCKET_SERVER_H_

#include <string>

#include "base/macros.h"
#include "chrome/browser/profiles/profile.h"

#if defined(ENABLE_HIGHWEB_DEVICEAPI)
#if defined(OS_ANDROID)
#include "chrome/browser/android/device_api/android_applauncher_api_handler_impl.h"
#else
#include "chrome/browser/device_api/applauncher/applauncher_api_handler_impl.h"
#endif
#endif

class DeviceWebsocketServer {
 public:
  explicit DeviceWebsocketServer(Profile* profile);
  ~DeviceWebsocketServer();

  void Start();

  void Stop();

  bool IsStarted() const;

 private:
  bool is_started_;
  std::unique_ptr<base::Thread> thread_;

#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  Profile* profile_;
  ApplauncherApiHandlerImpl* handler_;
#endif

  DISALLOW_COPY_AND_ASSIGN(DeviceWebsocketServer);
};

#endif  // CHROME_BROWSER_DEVICE_WEBSOCKET_SERVER_DEVICE_WEBSOCKET_SERVER_H_
