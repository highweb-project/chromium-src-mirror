// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/sound/devicesound_manager_impl.h"

#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "services/device/public/interfaces/devicesound_manager.mojom.h"
#include "services/device/public/interfaces/devicesound_resultData.mojom.h"

namespace device {

namespace {

class DeviceSoundManagerEmptyImpl : public mojom::DeviceSoundManager {
 public:

   explicit DeviceSoundManagerEmptyImpl(mojom::DeviceSoundManagerRequest request)
      : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))){
    }

   ~DeviceSoundManagerEmptyImpl() override {}

  void outputDeviceType(outputDeviceTypeCallback& callback) override {
    mojom::DeviceSound_ResultCodePtr result(mojom::DeviceSound_ResultCode::New());
    result->resultCode = int32_t(mojom::devicesound_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(mojom::devicesound_function::FUNC_OUTPUT_DEVICE_TYPE);
    result->outputType = -1;
    result->volume = mojom::DeviceSound_Volume::New();
    std::move(callback).Run(result.Clone());
  }
  void deviceVolume(deviceVolumeCallback& callback) override {
    mojom::DeviceSound_ResultCodePtr result(mojom::DeviceSound_ResultCode::New());
    result->resultCode = int32_t(mojom::devicesound_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(mojom::devicesound_function::FUNC_OUTPUT_DEVICE_TYPE);
    result->outputType = -1;
    result->volume = mojom::DeviceSound_Volume::New();
    std::move(callback).Run(result.Clone());
  }

 private:
  friend DeviceSoundManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<mojom::DeviceSoundManager> binding_;
};

}  // namespace

// static
void DeviceSoundManagerImpl::Create(
    mojom::DeviceSoundManagerRequest request) {
  new DeviceSoundManagerEmptyImpl(std::move(request));
}

}  // namespace device
