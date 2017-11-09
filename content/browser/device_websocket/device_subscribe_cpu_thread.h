// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_CPU_THREAD_H_
#define CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_CPU_THREAD_H_

#include "content/browser/device_websocket/device_subscribe_thread.h"

namespace content {
class DeviceWebsocketHandler;

class DeviceSubscribeCpuThread: public DeviceSubscribeThread {
  public:
    typedef base::Callback<void(base::DictionaryValue* value, int connection_id)> HandlerCallback;
    DeviceSubscribeCpuThread(std::string path, int connection_id, HandlerCallback callback);
    ~DeviceSubscribeCpuThread() override;

    void notifyFromHandler(int32_t resultCode, double loadValue);
    void setFilter(base::DictionaryValue* filters) override;

  private:
    void notify() override;
    void notifyFromHandlerInternal(int32_t resultCode, double loadValue);
    void runCallback();

    double lastLoadData = 0;
    double lastNotifyData = 0;
    int connection_id_ = 0;
    int32_t resultCode_ = 0;
    HandlerCallback callback_;

    std::unique_ptr<base::DictionaryValue> filters_;

    DISALLOW_COPY_AND_ASSIGN(DeviceSubscribeCpuThread);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_CPU_THREAD_H_
