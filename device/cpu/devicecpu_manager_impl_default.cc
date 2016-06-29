// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/cpu/devicecpu_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

namespace {

class DeviceCpuManagerEmptyImpl : public DeviceCpuManager {
 public:
  void getDeviceCpuLoad(const getDeviceCpuLoadCallback& callback) override {}
  void startCpuLoad() override {}
  void stopCpuLoad() override {}

 private:
  friend DeviceCpuManagerImpl;

  explicit DeviceCpuManagerEmptyImpl(
      mojo::InterfaceRequest<DeviceCpuManager> request)
      : binding_(this, std::move(request)) {}
  ~DeviceCpuManagerEmptyImpl() override {}

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<DeviceCpuManager> binding_;
};

}  // namespace

// static
void DeviceCpuManagerImpl::Create(
    mojo::InterfaceRequest<DeviceCpuManager> request) {
  new DeviceCpuManagerEmptyImpl(std::move(request));
}

}  // namespace device
