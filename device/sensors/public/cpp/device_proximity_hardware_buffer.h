// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_SENSORS_PUBLIC_CPP_DEVICE_PROXIMITY_HARDWARE_BUFFER_H_
#define DEVICE_SENSORS_PUBLIC_CPP_DEVICE_PROXIMITY_HARDWARE_BUFFER_H_

#include "device/sensors/public/cpp/device_proximity_data.h"
#include "device/base/synchronization/shared_memory_seqlock_buffer.h"

namespace device {

typedef device::SharedMemorySeqLockBuffer<DeviceProximityData> DeviceProximityHardwareBuffer;

}  // namespace content

#endif  // DEVICE_SENSORS_PUBLIC_CPP_DEVICE_PROXIMITY_HARDWARE_BUFFER_H_
