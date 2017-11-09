// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/sensors/data_fetcher_shared_memory.h"

#include "base/logging.h"
#include "device/sensors/public/cpp/device_motion_hardware_buffer.h"
#include "device/sensors/public/cpp/device_orientation_hardware_buffer.h"
#include "device/sensors/sensor_manager_android.h"

#if defined(ENABLE_HIGHWEB_DEVICEAPI)
#include "device/sensors/public/cpp/device_proximity_hardware_buffer.h"
#endif

namespace device {

DataFetcherSharedMemory::DataFetcherSharedMemory() {}

DataFetcherSharedMemory::~DataFetcherSharedMemory() {}

bool DataFetcherSharedMemory::Start(ConsumerType consumer_type, void* buffer) {
  DCHECK(buffer);

  switch (consumer_type) {
    case CONSUMER_TYPE_MOTION:
      SensorManagerAndroid::GetInstance()->StartFetchingDeviceMotionData(
          static_cast<DeviceMotionHardwareBuffer*>(buffer));
      return true;
    case CONSUMER_TYPE_ORIENTATION:
      SensorManagerAndroid::GetInstance()->StartFetchingDeviceOrientationData(
          static_cast<DeviceOrientationHardwareBuffer*>(buffer));
      return true;
    case CONSUMER_TYPE_ORIENTATION_ABSOLUTE:
      SensorManagerAndroid::GetInstance()
          ->StartFetchingDeviceOrientationAbsoluteData(
              static_cast<DeviceOrientationHardwareBuffer*>(buffer));
      return true;
  #if defined(ENABLE_HIGHWEB_DEVICEAPI)
    case CONSUMER_TYPE_PROXIMITY:
      SensorManagerAndroid::GetInstance()->StartFetchingDeviceProximityData(
          static_cast<DeviceProximityHardwareBuffer*>(buffer));
      return true;
  #endif
    default:
      NOTREACHED();
  }
  return false;
}

bool DataFetcherSharedMemory::Stop(ConsumerType consumer_type) {
  switch (consumer_type) {
    case CONSUMER_TYPE_MOTION:
      SensorManagerAndroid::GetInstance()->StopFetchingDeviceMotionData();
      return true;
    case CONSUMER_TYPE_ORIENTATION:
      SensorManagerAndroid::GetInstance()->StopFetchingDeviceOrientationData();
      return true;
    case CONSUMER_TYPE_ORIENTATION_ABSOLUTE:
      SensorManagerAndroid::GetInstance()
          ->StopFetchingDeviceOrientationAbsoluteData();
      return true;
  #if defined(ENABLE_HIGHWEB_DEVICEAPI)
    case CONSUMER_TYPE_PROXIMITY:
      SensorManagerAndroid::GetInstance()->StopFetchingDeviceProximityData();
      return true;
  #endif
    default:
      NOTREACHED();
  }
  return false;
}

void DataFetcherSharedMemory::Shutdown() {
  DataFetcherSharedMemoryBase::Shutdown();
  SensorManagerAndroid::GetInstance()->Shutdown();
}

}  // namespace device
