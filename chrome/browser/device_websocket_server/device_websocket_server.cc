// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/device_websocket_server/device_websocket_server.h"

#include "base/bind.h"
#include "base/strings/stringprintf.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/device_websocket_agent_host.h"
#include "content/public/browser/device_socket_factory.h"
#include "net/base/ip_address.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/base/network_interfaces.h"
#include "net/server/http_server.h"

using net::IPAddress;
using net::IPEndPoint;
using content::DeviceWebsocketAgentHost;

namespace {

#if defined(ENABLE_HIGHWEB_DEVICEAPI)
const char kDeviceWebsocketHandlerThreadName[] = "Chrome_DeviceWebsocketHandlerThread";
#endif
const int kBackLog = 10;

class TCPDeviceSocketFactory : public content::DeviceSocketFactory {
 public:
  TCPDeviceSocketFactory(
      const std::string& socket_address,
      const uint16_t port)
      : socket_address_(socket_address),
        socket_port_(port) {
  }

 private:
  std::unique_ptr<net::ServerSocket> CreateForHttpServer() override {
    std::unique_ptr<net::TCPServerSocket> socket(
        new net::TCPServerSocket(nullptr, net::NetLogSource()));
    if (socket->ListenWithAddressAndPort(socket_address_, socket_port_, kBackLog) == net::OK) {
      return std::move(socket);
    }
    return std::unique_ptr<net::ServerSocket>();
  }

  std::string socket_address_;
  uint16_t socket_port_;

  DISALLOW_COPY_AND_ASSIGN(TCPDeviceSocketFactory);
};

}  // namespace

DeviceWebsocketServer::DeviceWebsocketServer(Profile* profile)
    : is_started_(false) {
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  profile_ = profile;
  handler_ = ApplauncherApiHandlerImpl::GetInstance();
#endif
}

DeviceWebsocketServer::~DeviceWebsocketServer() {
  Stop();
}

void DeviceWebsocketServer::Start() {
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  if (is_started_)
    return;

  std::string ip = "192.168.70.12";
  uint16_t port = 20000;

  net::NetworkInterfaceList networks;
  bool success = net::GetNetworkList(&networks, net::EXCLUDE_HOST_SCOPE_VIRTUAL_INTERFACES);
  if (success) {
    for (size_t i = 0; i < networks.size(); i++) {
      net::AddressFamily address_family = net::GetAddressFamily(networks[i].address);

      // LOG(ERROR) << "networks[i].name : " << networks[i].name << ", networks[i].address : " << networks[i].address.ToString();
      // LOG(ERROR) << "networks[i].name.find_first_of(targetAdapter) : " << networks[i].name.find_first_of(targetAdapter) << ", std::string::npos : " << std::string::npos;

#if defined(OS_ANDROID)
      std::string targetAdapter = "wlan0";
#elif defined(OS_LINUX)
      std::string targetAdapter = "enp";
#endif
      if (address_family == net::ADDRESS_FAMILY_IPV4 && networks[i].name.find_first_of(targetAdapter) == 0) {
        ip = networks[i].address.ToString();
        break;
      }
    }
  }

  LOG(ERROR) << "device_websocket_server IP : " << ip;

  std::unique_ptr<content::DeviceSocketFactory> factory(
      new TCPDeviceSocketFactory(ip, port));

  thread_.reset(new base::Thread(kDeviceWebsocketHandlerThreadName));
  base::Thread::Options options;
  options.message_loop_type = base::MessageLoop::TYPE_IO;
  if (thread_.get()->StartWithOptions(options)) {
    DeviceWebsocketAgentHost::StartWebsocketServer(thread_.get(), std::move(factory), profile_);
    DeviceWebsocketAgentHost::SetDeviceAPIHandler(thread_.get(), (content::ApplauncherApiHandler*)handler_);
    is_started_ = true;
  } else {
    LOG(ERROR) << "Start WebsocketHandlerThread error";
  }
#endif
}

void DeviceWebsocketServer::Stop() {
#if defined(ENABLE_HIGHWEB_DEVICEAPI)
  is_started_ = false;
  DeviceWebsocketAgentHost::StopWebsocketServer();
  if (thread_) {
    thread_.get()->Stop();
    thread_.reset();
  }
#endif
}

bool DeviceWebsocketServer::IsStarted() const {
  return is_started_;
}
