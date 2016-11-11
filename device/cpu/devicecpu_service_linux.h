// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_CPU_DEVICECPU_SERVICE_
#define DEVICE_CPU_DEVICECPU_SERVICE_

#include "base/threading/platform_thread.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "base/bind.h"
#include "base/time/time.h"

namespace device {

class DeviceCpuCallbackThread : public base::Thread {
  public:
    typedef base::Callback<void()> DeviceCpuCallback;
    static std::string threadName(){
      return std::string("DeviceCpuCallbackThread");
    }

    DeviceCpuCallbackThread();
    ~DeviceCpuCallbackThread() override;

    void startCallbackThread(const DeviceCpuCallback& callback);
  private:
    void callbackThread(const DeviceCpuCallback& callback);
    void innerStart();
    void innerStop();

    bool stillRunning = false;
    volatile bool isAlive = true;

    DISALLOW_COPY_AND_ASSIGN(DeviceCpuCallbackThread);
};

class DeviceCpuMonitorThread : public base::Thread {
  public:
    typedef base::Callback<void(const float&)> DeviceCpuMonitorCallback;
    static std::string threadName() {
      return std::string("DeviceCpuMonitorThread");
    }

    DeviceCpuMonitorThread();
    ~DeviceCpuMonitorThread() override;

    void startMonitor(const DeviceCpuMonitorCallback& callback);
    void stopMonitor();
  private:
    void readUsage(const DeviceCpuMonitorCallback& callback);

    volatile bool isAlive = false;
    bool stillRunning = false;

    DeviceCpuMonitorCallback* callback_;

    DISALLOW_COPY_AND_ASSIGN(DeviceCpuMonitorThread);
};

} // namespace device

#endif
