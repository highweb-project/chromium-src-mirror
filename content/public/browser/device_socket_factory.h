// Copyright 2016 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_DEVICE_SOCKET_FACTORY_H_
#define CONTENT_PUBLIC_BROWSER_DEVICE_SOCKET_FACTORY_H_

#include <string>

#include "net/socket/server_socket.h"
#include "net/socket/tcp_server_socket.h"

namespace content {

class DeviceSocketFactory {
 public:
  virtual ~DeviceSocketFactory() {}

  virtual std::unique_ptr<net::ServerSocket> CreateForHttpServer() = 0;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_DEVICE_SOCKET_FACTORY_H_
