// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/sound/devicesound_manager_impl.h"

#include <stddef.h>
#include <string>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "device/sound/devicesound_manager.mojom.h"
#include "device/sound/devicesound_resultData.mojom.h"

namespace device {

namespace {

class DeviceSoundManagerLinux : public DeviceSoundManager {
 public:
  void outputDeviceType(const outputDeviceTypeCallback& callback) override;
  void deviceVolume(const deviceVolumeCallback& callback) override;

 private:
  friend DeviceSoundManagerImpl;

  std::string exec(const char* cmd);
  int32_t getOutputDeviceLinux(std::string& execResult);
  float getDeviceVolume(std::string& execResult);

  explicit DeviceSoundManagerLinux(
      mojo::InterfaceRequest<DeviceSoundManager> request);
  ~DeviceSoundManagerLinux() override;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<DeviceSoundManager> binding_;
};

}  // namespace

// static
void DeviceSoundManagerImpl::Create(
    mojo::InterfaceRequest<DeviceSoundManager> request) {
  new DeviceSoundManagerLinux(std::move(request));
}

}  // namespace device
