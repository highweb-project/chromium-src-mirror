// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_THREAD_H_
#define CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_THREAD_H_

#include "base/threading/thread.h"
#include "base/timer/timer.h"
#include "content/browser/device_websocket/device_websocket_handler.h"

namespace base{
  class DictionaryValue;
}

namespace content {

class DeviceSubscribeThread: public base::Thread {
  public:

    DeviceSubscribeThread(std::string path, std::string threadName);

    ~DeviceSubscribeThread() override;

    void setInterval(int32_t interval);
    void setMaxValue(double maxValue);
    void setMinValue(double minValue);
    void setMinChange(double minChange);

    bool IsRunning();

    bool StartThread();
    virtual void setFilter(base::DictionaryValue* filters);

  protected:
    int32_t interval_ = 0;
    double maxValue_ = 0;
    double minValue_ = 0;
    double minChange_ = 0;
    std::string path_;

    virtual void notify();

    std::unique_ptr<base::RepeatingTimer> notify_timer_;

    DISALLOW_COPY_AND_ASSIGN(DeviceSubscribeThread);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_SUBSCRIBE_THREAD_H_
