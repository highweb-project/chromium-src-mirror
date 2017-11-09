// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_MESSAGE_THREAD_H_
#define CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_MESSAGE_THREAD_H_

#include "content/browser/device_websocket/device_subscribe_thread.h"

namespace content {

class DeviceSubscribeMessageThread: public DeviceSubscribeThread {
  public:
    typedef base::Callback<void(base::DictionaryValue* value, int connection_id)> HandlerCallback;
    DeviceSubscribeMessageThread(std::string path, int connection_id, HandlerCallback callback);
    ~DeviceSubscribeMessageThread() override;

    void notifyFromHandler(device::mojom::MessageObjectPtr message);
    void setFilter(base::DictionaryValue* filters) override;

  private:
    void notify() override;
    void notifyFromHandlerInternal();
    void runCallback();

    std::queue<device::mojom::MessageObjectPtr> messageQueue;
    int connection_id_ = 0;
    HandlerCallback callback_;

    std::unique_ptr<base::DictionaryValue> filters_;

    DISALLOW_COPY_AND_ASSIGN(DeviceSubscribeMessageThread);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_MESSAGE_THREAD_H_
