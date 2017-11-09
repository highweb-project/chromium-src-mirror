// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/sound/devicesound_manager_impl.h"

#include <stddef.h>
#include <string>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "services/device/public/interfaces/devicesound_manager.mojom.h"
#include "services/device/public/interfaces/devicesound_resultData.mojom.h"

namespace device {

namespace {

class DeviceSoundManagerLinux : public mojom::DeviceSoundManager {
 public:
  explicit DeviceSoundManagerLinux(mojom::DeviceSoundManagerRequest request);
  ~DeviceSoundManagerLinux() override;

  void outputDeviceType(outputDeviceTypeCallback callback) override;
  void deviceVolume(deviceVolumeCallback callback) override;

 private:
  friend DeviceSoundManagerImpl;

  std::string exec(const char* cmd);
  int32_t getOutputDeviceLinux(std::string& execResult);
  float getDeviceVolume(std::string& execResult);

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<mojom::DeviceSoundManager> binding_;
};

}  // namespace

// static
void DeviceSoundManagerImpl::Create(
    mojom::DeviceSoundManagerRequest request) {
  new DeviceSoundManagerLinux(std::move(request));
}

}  // namespace device
