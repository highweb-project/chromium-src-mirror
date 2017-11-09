// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/cpu/devicecpu_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "services/device/public/interfaces/devicecpu_manager.mojom.h"
#include "services/device/public/interfaces/devicecpu_ResultCode.mojom.h"

namespace device {

namespace {

class DeviceCpuManagerEmptyImpl : public DeviceCpuManager {
 public:

  explicit DeviceCpuManagerEmptyImpl(mojom::DeviceCpuManagerRequest request)
    : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~DeviceCpuManagerEmptyImpl() override {}

  void getDeviceCpuLoad(getDeviceCpuLoadCallback callback) override {
    mojom::DeviceCpu_ResultCodePtr result(mojom::DeviceCpu_ResultCode::New());
    result->resultCode = int32_t(mojom::device_cpu_ErrorCodeList::NOT_SUPPORT_API);
    result->functionCode = int32_t(mojom::device_cpu_function::FUNC_GET_CPU_LOAD);
    result->load = 0.0f;
    std::move(callback).Run(result.Clone());
  }
  void startCpuLoad() override {
  }
  void stopCpuLoad() override {
  }

 private:
  friend DeviceCpuManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<DeviceCpuManager> binding_;
};

}  // namespace

// static
void DeviceCpuManagerImpl::Create(mojom::DeviceCpuManagerRequest request) {
  new DeviceCpuManagerEmptyImpl(std::move(request));
}

}  // namespace device
