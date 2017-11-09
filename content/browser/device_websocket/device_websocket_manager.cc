// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_websocket/device_websocket_manager.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/device_socket_factory.h"
#include "content/public/browser/device_websocket_agent_host.h"

#include "chrome/browser/profiles/profile.h"

namespace content {

void DeviceWebsocketAgentHost::StartWebsocketServer(base::Thread* thread,
      std::unique_ptr<DeviceSocketFactory> server_socket_factory, Profile* profile) {
  DeviceWebsocketManager* manager = DeviceWebsocketManager::GetInstance();
  if(thread && thread->IsRunning()) {
    base::MessageLoop* message_loop = thread->message_loop();
    message_loop->task_runner()->PostTask(
        FROM_HERE,
        base::Bind(&DeviceWebsocketManager::StartServer, 
        base::Unretained(manager), 
        base::Unretained(thread), 
        server_socket_factory.release(), profile));
  } else {
    LOG(ERROR) << "Thread is empty or not running in StartWebsocketServer";
  }
}

void DeviceWebsocketAgentHost::SetDeviceAPIHandler(base::Thread* thread, ApplauncherApiHandler* handler) {
  DeviceWebsocketManager* manager = DeviceWebsocketManager::GetInstance();
  if (thread && thread->IsRunning()) {
    base::MessageLoop* message_loop = thread->message_loop();
    message_loop->task_runner()->PostTask(
        FROM_HERE,
        base::Bind(&DeviceWebsocketManager::SetDeviceAPIHandler, 
        base::Unretained(manager), 
        handler));
  } else {
    LOG(ERROR) << "Thread is empty or not running in SetDeviceAPIHandler";
  }
}

void DeviceWebsocketAgentHost::StopWebsocketServer() {
  DeviceWebsocketManager* manager = DeviceWebsocketManager::GetInstance();
  manager->StopWebsocketServer();
}

void DeviceWebsocketManager::StartServer(base::Thread* thread, DeviceSocketFactory* server_socket_factory, Profile* profile) {
  SetWebsocketHandler(base::WrapUnique(new DeviceWebsocketHandler(server_socket_factory->CreateForHttpServer(), thread, profile)));
}

void DeviceWebsocketManager::StopWebsocketServer() {
  if (websocket_handler_) {
    websocket_handler_->StopHandler();
  }
  SetWebsocketHandler(nullptr);
}

DeviceWebsocketManager* DeviceWebsocketManager::GetInstance() {
  return base::Singleton<DeviceWebsocketManager>::get();
}

void DeviceWebsocketManager::SetWebsocketHandler(std::unique_ptr<DeviceWebsocketHandler> websocket_handler) {
  websocket_handler_.swap(websocket_handler);
}

void DeviceWebsocketManager::SetDeviceAPIHandler(ApplauncherApiHandler* handler) {
  if (websocket_handler_) {
    websocket_handler_->SetDeviceAPIHandler(handler);
  }
}

DeviceWebsocketManager::DeviceWebsocketManager()
{
}

DeviceWebsocketManager::~DeviceWebsocketManager() {
  SetWebsocketHandler(nullptr);
}

}  // namespace content
