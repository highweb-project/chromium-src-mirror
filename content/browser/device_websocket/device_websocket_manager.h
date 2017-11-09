// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_MANAGER_H_
#define CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_MANAGER_H_

#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/singleton.h"
#include "content/browser/device_websocket/device_websocket_handler.h"
#include "content/common/content_export.h"
#include "content/public/browser/device_socket_factory.h"

#include "chrome/browser/profiles/profile.h"

namespace content {
class ApplauncherApiHandler;

class CONTENT_EXPORT DeviceWebsocketManager {
 public:
  static DeviceWebsocketManager* GetInstance();

  DeviceWebsocketManager();
  virtual ~DeviceWebsocketManager();

  void SetWebsocketHandler(std::unique_ptr<DeviceWebsocketHandler> websocket_handler);
  void StartServer(base::Thread* thread, DeviceSocketFactory* server_socket_factory, Profile* profile);
  void StopWebsocketServer();
  void SetDeviceAPIHandler(ApplauncherApiHandler* handler);
 private:
  friend struct base::DefaultSingletonTraits<DeviceWebsocketManager>;
  std::unique_ptr<DeviceWebsocketHandler> websocket_handler_;

  DISALLOW_COPY_AND_ASSIGN(DeviceWebsocketManager);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_MANAGER_H_
