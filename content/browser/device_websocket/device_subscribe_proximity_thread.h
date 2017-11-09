// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_PROXIMITY_THREAD_H_
#define CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_PROXIMITY_THREAD_H_

#include "base/timer/timer.h"
#include "content/browser/device_websocket/device_subscribe_thread.h"
#include "content/renderer/shared_memory_seqlock_reader.h"
#include "device/sensors/public/cpp/device_proximity_data.h"
#include "device/sensors/public/interfaces/proximity.mojom.h"

namespace content {
class DeviceWebsocketHandler;
typedef SharedMemorySeqLockReader<device::DeviceProximityData>
    DeviceProximitySharedMemoryReader;

class DeviceSubscribeProximitySensorThread: public DeviceSubscribeThread {
  public:

    enum RunningState {
      RUNNING,
      IDLE,
      STOPPED,
      ERROR,
    };

    typedef base::Callback<void(base::DictionaryValue* value, int connection_id)> HandlerCallback;
    DeviceSubscribeProximitySensorThread(std::string path, int connection_id, HandlerCallback callback);
    ~DeviceSubscribeProximitySensorThread() override;

    void setFilter(base::DictionaryValue* filters) override;

    void DidStart(mojo::ScopedSharedBufferHandle buffer_handle);
    bool InitializeReader(base::SharedMemoryHandle handle);
    void FireEvent();

    bool StartThread(device::mojom::ProximitySensorPtr sensor);

  private:
    void notify() override;
    void runCallback();

    double lastProximityData = 0;
    double lastNotifyData = 0;
    int connection_id_ = 0;
    HandlerCallback callback_;

    std::unique_ptr<base::DictionaryValue> filters_;
    std::unique_ptr<base::RepeatingTimer> timer_;
    std::unique_ptr<DeviceProximitySharedMemoryReader> reader_;
    device::mojom::ProximitySensorPtr proxySensor;

    RunningState state_;

    DISALLOW_COPY_AND_ASSIGN(DeviceSubscribeProximitySensorThread);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_PROXIMITY_THREAD_H_
