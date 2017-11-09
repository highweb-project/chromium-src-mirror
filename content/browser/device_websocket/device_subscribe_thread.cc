// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_websocket/device_subscribe_thread.h"
#include "base/time/time.h"

namespace content {

DeviceSubscribeThread::DeviceSubscribeThread(std::string path, std::string threadName) : 
    base::Thread(threadName), path_(path) {
  base::ThreadRestrictions::SetIOAllowed(true);
};

DeviceSubscribeThread::~DeviceSubscribeThread() {
  if (notify_timer_) {
    notify_timer_->AbandonAndStop();
    notify_timer_.reset();
  }
  Stop();
}

bool DeviceSubscribeThread::StartThread() {
  if (interval_ > 0 && IsRunning()) {
    notify_timer_.reset(new base::RepeatingTimer);
    notify_timer_->Start(FROM_HERE,
                        base::TimeDelta::FromMilliseconds(interval_),
                        this,
                        &DeviceSubscribeThread::notify);
  }
  return base::Thread::Start();
}

void DeviceSubscribeThread::notify() {
  LOG(ERROR) << "subscribeThread notify not implement";
}

void DeviceSubscribeThread::setFilter(base::DictionaryValue* filters) {
  LOG(ERROR) << "subscribeThread setFilter not implement";
}

void DeviceSubscribeThread::setInterval(int32_t interval) {
  if (interval >= 500) {
    interval_ = interval;
  }
}

void DeviceSubscribeThread::setMaxValue(double maxValue) {
  maxValue_ = maxValue;
}

void DeviceSubscribeThread::setMinValue(double minValue) {
  minValue_ = minValue;
}

void DeviceSubscribeThread::setMinChange(double minChange) {
  minChange_ = minChange;
}

bool DeviceSubscribeThread::IsRunning() {
  return notify_timer_.get();
}


}  // namespace content
