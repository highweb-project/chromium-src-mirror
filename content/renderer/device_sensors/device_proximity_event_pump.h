// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DEVICE_SENSORS_DEVICE_PROXIMITY_EVENT_PUMP_H_
#define CONTENT_RENDERER_DEVICE_SENSORS_DEVICE_PROXIMITY_EVENT_PUMP_H_

#include <utility>

#include "device/sensors/public/cpp/device_proximity_data.h"
#include "content/renderer/device_sensors/device_sensor_event_pump.h"
#include "content/renderer/shared_memory_seqlock_reader.h"
#include "device/sensors/public/interfaces/proximity.mojom.h"

namespace blink {
class WebDeviceProximityListener;
}

namespace content {

typedef SharedMemorySeqLockReader<device::DeviceProximityData>
    DeviceProximitySharedMemoryReader;

class CONTENT_EXPORT DeviceProximityEventPump
      : public DeviceSensorMojoClientMixin<DeviceSensorEventPump<blink::WebDeviceProximityListener>,
          device::mojom::ProximitySensor> {
    // : public DeviceSensorEventPump<blink::WebDeviceProximityListener> {
 public:
  explicit DeviceProximityEventPump(RenderThread* thread);
  ~DeviceProximityEventPump() override;

  // Sets the listener to receive updates for device light data at
  // regular intervals. Returns true if the registration was successful.
  // bool SetListener(blink::WebDeviceProximityListener* listener);

  // PlatformEventObserver implementation.
  // bool OnControlMessageReceived(const IPC::Message& message) override;
  void SendFakeDataForTesting(void* data) override;

 protected:
  // Methods overriden from base class DeviceSensorEventPump
  void FireEvent() override;
  bool InitializeReader(base::SharedMemoryHandle handle) override;

  // PlatformEventObserver implementation.
  // void SendStartMessage() override;
  // void SendStopMessage() override;

  bool ShouldFireEvent(double data) const;

  std::unique_ptr<DeviceProximitySharedMemoryReader> reader_;
  double last_seen_data_;

  DISALLOW_COPY_AND_ASSIGN(DeviceProximityEventPump);
};

}  // namespace content

#endif  // CONTENT_RENDERER_DEVICE_SENSORS_DEVICE_PROXIMITY_EVENT_PUMP_H_
