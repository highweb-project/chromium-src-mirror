// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/thirdparty/devicethirdparty_manager_impl.h"

#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

class DeviceThirdpartyManagerEmptyImpl : public DeviceThirdpartyManager {
 public:
  explicit DeviceThirdpartyManagerEmptyImpl(DeviceThirdpartyManagerRequest request)
     : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~DeviceThirdpartyManagerEmptyImpl() override {}

  void sendAndroidBroadcast(const std::string& action, const sendAndroidBroadcastCallback& callback) override {
    SendAndroidBroadcastCallbackDataPtr result = SendAndroidBroadcastCallbackData::New();
    result->action = "";
    callback.Run(result.Clone());
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
