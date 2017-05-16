// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/thirdparty/devicethirdparty_manager_impl.h"

#include "mojo/public/cpp/bindings/strong_binding.h"

#include <sys/socket.h>
#include <arpa/inet.h>

namespace device {

class DeviceThirdpartyManagerEmptyImpl : public DeviceThirdpartyManager {
 public:
  explicit DeviceThirdpartyManagerEmptyImpl(DeviceThirdpartyManagerRequest request)
     : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~DeviceThirdpartyManagerEmptyImpl() override {}

    void sendAndroidBroadcast(const std::string & action, const sendAndroidBroadcastCallback & callback) override {
      LOG(ERROR) << "[3rd-party API][" << __FUNCTION__ << "][" << __LINE__ << "] action : " << action;
      SendAndroidBroadcastCallbackDataPtr result = SendAndroidBroadcastCallbackData::New();
      result->action = "ERROR";

      const char* IP = "127.0.0.1";
      int PORT = 8888;

      int sock;
      struct sockaddr_in server;
      char server_reply[2000] = { 0, };
      sock = socket(AF_INET , SOCK_STREAM , 0);
      if (sock != -1) {
        server.sin_addr.s_addr = inet_addr(IP);
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);

        if (connect(sock , (struct sockaddr *)&server , sizeof(server)) == 0) {
          if (send(sock , action.c_str() , strlen(action.c_str()) , 0) > 0) {
            if (recv(sock , server_reply , 2000 , 0) > 0) {
              std::string reply(server_reply);
              result->action = reply;
              LOG(ERROR) << "[3rd-party API][" << __FUNCTION__ << "][" << __LINE__ << "] reply : " << reply;
            }
          }
          close(sock);
        }
        callback.Run(result.Clone());
      }
    }

 private:
  friend DeviceThirdpartyManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<DeviceThirdpartyManager> binding_;
};

// static
void DeviceThirdpartyManagerImpl::Create(
    DeviceThirdpartyManagerRequest request) {
  new DeviceThirdpartyManagerEmptyImpl(std::move(request));
}

}  // namespace device
