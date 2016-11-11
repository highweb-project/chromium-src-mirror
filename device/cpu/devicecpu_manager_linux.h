// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_CPU_DEVICECPU_MANAGER_
#define DEVICE_CPU_DEVICECPU_MANAGER_

#include "device/cpu/devicecpu_manager_impl.h"

#include <deque>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "device/cpu/devicecpu_manager.mojom.h"
#include "device/cpu/devicecpu_ResultCode.mojom.h"
#include "devicecpu_service_linux.h"

namespace device {

class DeviceCpuManagerLinux : public DeviceCpuManager {
 public:
  typedef std::deque<getDeviceCpuLoadCallback> loadCallbackList;

  void getDeviceCpuLoad(const getDeviceCpuLoadCallback& callback) override;
  void startCpuLoad() override;
  void stopCpuLoad() override;
  void callbackCpuLoad();
  void callbackMojo();

  void callbackLoad(const float& load);

 private:
  friend DeviceCpuManagerImpl;

  scoped_refptr<base::SingleThreadTaskRunner> main_thread_task_runner_;

  explicit DeviceCpuManagerLinux(
      mojo::InterfaceRequest<DeviceCpuManager> request);
  ~DeviceCpuManagerLinux() override;

  mojo::StrongBinding<DeviceCpuManager> binding_;

  DeviceCpuCallbackThread* callbackThread = nullptr;
  loadCallbackList deviceCpuLoadCallbackList;

  DeviceCpuMonitorThread* monitorThread = nullptr;
  bool isAlive = false;
  volatile float load_ = 0.0f;
};

// static
void DeviceCpuManagerImpl::Create(
    mojo::InterfaceRequest<DeviceCpuManager> request) {
  new DeviceCpuManagerLinux(std::move(request));
}

}  // namespace device

#endif
