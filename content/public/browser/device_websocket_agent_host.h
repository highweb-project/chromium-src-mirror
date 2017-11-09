// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_DEVICE_WEBSOCKET_AGENT_HOST_H_
#define CONTENT_PUBLIC_BROWSER_DEVICE_WEBSOCKET_AGENT_HOST_H_

#include <memory>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "content/common/content_export.h"
#include "url/gurl.h"

#include "chrome/browser/profiles/profile.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace net {
class ServerSocket;
}

namespace content {

class BrowserContext;
class DeviceSocketFactory;
class RenderFrameHost;
class WebContents;
class ApplauncherApiHandler;

class CONTENT_EXPORT DeviceWebsocketAgentHost
    : public base::RefCounted<DeviceWebsocketAgentHost> {
 public:
  static void StartWebsocketServer(base::Thread* thread,
      std::unique_ptr<DeviceSocketFactory> server_socket_factory, Profile* profile);
  static void SetDeviceAPIHandler(base::Thread* thread, ApplauncherApiHandler* handler);

  static void StopWebsocketServer();

 protected:
  friend class base::RefCounted<DeviceWebsocketAgentHost>;
  virtual ~DeviceWebsocketAgentHost() {}
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_DEVICE_WEBSOCKET_AGENT_HOST_H_
