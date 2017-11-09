// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/device_sensors/device_proximity_event_pump.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "content/public/renderer/render_thread.h"
#include "third_party/WebKit/public/platform/WebDeviceProximityListener.h"

namespace {
// Default rate for firing of DeviceLight events.
const int kDefaultProximityPumpFrequencyHz = 5;
const int kDefaultProximityPumpDelayMicroseconds =
    base::Time::kMicrosecondsPerSecond / kDefaultProximityPumpFrequencyHz;
}  // namespace

namespace content {

DeviceProximityEventPump::DeviceProximityEventPump(RenderThread* thread)
  : DeviceSensorMojoClientMixin<DeviceSensorEventPump<blink::WebDeviceProximityListener>,
      device::mojom::ProximitySensor>(thread),
    // : DeviceSensorEventPump<blink::WebDeviceProximityListener>(thread),
      last_seen_data_(-1) {
  pump_delay_microseconds_ = kDefaultProximityPumpDelayMicroseconds;
}

DeviceProximityEventPump::~DeviceProximityEventPump() {
}

void DeviceProximityEventPump::FireEvent() {
  DCHECK(listener());
  device::DeviceProximityData data;
  if (reader_->GetLatestData(&data) && ShouldFireEvent(data.value)) {
    last_seen_data_ = data.value;
    listener()->didChangeDeviceProximity(data.value);
  }
}

bool DeviceProximityEventPump::ShouldFireEvent(double distance) const {
  if (distance < 0)
    return false;

  if (distance == std::numeric_limits<double>::infinity()) {
    // no sensor data can be provided, fire an Infinity event to Blink.
    return true;
  }

  return distance != last_seen_data_;
}

bool DeviceProximityEventPump::InitializeReader(base::SharedMemoryHandle handle) {
  if (!reader_)
    reader_.reset(new DeviceProximitySharedMemoryReader());
  return reader_->Initialize(handle);
}

void DeviceProximityEventPump::SendFakeDataForTesting(void* fake_data) {
  double data = *static_cast<double*>(fake_data);

  listener()->didChangeDeviceProximity(data);
}

}  // namespace content
