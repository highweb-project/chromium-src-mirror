// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/storage/devicestorage_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

class DeviceStorageManagerLinux : public DeviceStorageManager {
 public:

  explicit DeviceStorageManagerLinux(DeviceStorageManagerRequest request);
  ~DeviceStorageManagerLinux() override;

  void getDeviceStorage(const getDeviceStorageCallback& callback) override;

 private:
  friend DeviceStorageManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<DeviceStorageManager> binding_;

  std::string exec(const char* cmd);
};

// static
void DeviceStorageManagerImpl::Create(DeviceStorageManagerRequest request) {
  new DeviceStorageManagerLinux(std::move(request));
}

}  // namespace device
