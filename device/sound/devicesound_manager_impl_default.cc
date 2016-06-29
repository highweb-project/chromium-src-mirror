// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/sound/devicesound_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

namespace {

class DeviceSoundManagerEmptyImpl : public DeviceSoundManager {
 public:
  void outputDeviceType(const outputDeviceTypeCallback& callback) override {}
  void deviceVolume(const deviceVolumeCallback& callback) override {}

 private:
  friend DeviceSoundManagerImpl;

  explicit DeviceSoundManagerEmptyImpl(
      mojo::InterfaceRequest<DeviceSoundManager> request)
      : binding_(this, std::move(request)) {}
  ~DeviceSoundManagerEmptyImpl() override {}

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<DeviceSoundManager> binding_;
};

}  // namespace

// static
void DeviceSoundManagerImpl::Create(
    mojo::InterfaceRequest<DeviceSoundManager> request) {
  new DeviceSoundManagerEmptyImpl(std::move(request));
}

}  // namespace device