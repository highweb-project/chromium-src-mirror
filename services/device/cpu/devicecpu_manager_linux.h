// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_CPU_DEVICECPU_MANAGER_
#define DEVICE_CPU_DEVICECPU_MANAGER_

#include "services/device/cpu/devicecpu_manager_impl.h"

#include <deque>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "services/device/public/interfaces/devicecpu_manager.mojom.h"
#include "services/device/public/interfaces/devicecpu_ResultCode.mojom.h"
#include "devicecpu_service_linux.h"

namespace device {

class DeviceCpuManagerLinux : public mojom::DeviceCpuManager {
 public:
  typedef std::deque<getDeviceCpuLoadCallback> loadCallbackList;

  explicit DeviceCpuManagerLinux(mojom::DeviceCpuManagerRequest request);
  ~DeviceCpuManagerLinux() override;

  void getDeviceCpuLoad(getDeviceCpuLoadCallback callback) override;
  void startCpuLoad() override;
  void stopCpuLoad() override;
  void callbackCpuLoad();
  void callbackMojo();

  void callbackLoad(const float& load);

 private:
  friend DeviceCpuManagerImpl;

  scoped_refptr<base::SingleThreadTaskRunner> main_thread_task_runner_;

  mojo::StrongBindingPtr<DeviceCpuManager> binding_;

  DeviceCpuCallbackThread* callbackThread = nullptr;
  loadCallbackList deviceCpuLoadCallbackList;

  DeviceCpuMonitorThread* monitorThread = nullptr;
  bool isAlive = false;
  volatile float load_ = 0.0f;
};

// static
void DeviceCpuManagerImpl::Create(mojom::DeviceCpuManagerRequest request) {
  new DeviceCpuManagerLinux(std::move(request));
}

}  // namespace device

#endif
