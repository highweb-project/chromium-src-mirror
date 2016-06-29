// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_CPU_CPU_MANAGER_IMPL_H_
#define DEVICE_CPU_CPU_MANAGER_IMPL_H_

#include "device/cpu/devicecpu_export.h"
#include "device/cpu/devicecpu_manager.mojom.h"

namespace device {

class DeviceCpuManagerImpl {
 public:
  DEVICE_CPU_EXPORT static void Create(
      mojo::InterfaceRequest<DeviceCpuManager> request);
};

}  // namespace device

#endif  // DEVICE_CPU_CPU_MANAGER_IMPL_H_
