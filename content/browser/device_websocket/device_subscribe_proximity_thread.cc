// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_websocket/device_subscribe_proximity_thread.h"

#include "base/message_loop/message_loop.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/strings/string_number_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "mojo/public/cpp/system/platform_handle.h"

namespace content {

void DeviceSubscribeProximitySensorThread::DidStart(mojo::ScopedSharedBufferHandle buffer_handle) {
    if (state_ == RunningState::RUNNING) {
      LOG(ERROR) << "Already Started";
      return;
    }
    base::SharedMemoryHandle handle;
    MojoResult result = mojo::UnwrapSharedMemoryHandle(
        std::move(buffer_handle), &handle, nullptr, nullptr);
    if (result == MOJO_RESULT_OK && InitializeReader(handle)) {
      timer_->Start(FROM_HERE,
                   base::TimeDelta::FromMicroseconds(base::Time::kMicrosecondsPerSecond / 60),
                   this,
                   &DeviceSubscribeProximitySensorThread::FireEvent);
      state_ = RunningState::RUNNING;
    } else {
      LOG(ERROR) << "InitializeReader fail";
      state_ = RunningState::ERROR;
      notify();
    }
  }

  bool DeviceSubscribeProximitySensorThread::InitializeReader(base::SharedMemoryHandle handle) {
  if (!reader_)
    reader_.reset(new DeviceProximitySharedMemoryReader());
  return reader_->Initialize(handle);
}

void DeviceSubscribeProximitySensorThread::FireEvent() {
  device::DeviceProximityData data;
  if (reader_->GetLatestData(&data)) {
    lastProximityData = data.value;
    if (lastNotifyData != lastProximityData && !IsRunning()) {
      notify();
    }
  }
}

DeviceSubscribeProximitySensorThread::DeviceSubscribeProximitySensorThread(std::string path, int connection_id, HandlerCallback callback) : 
    DeviceSubscribeThread(path, "devicesubscribeProximitySensorthread"), timer_(new base::RepeatingTimer) {
  connection_id_ = connection_id;
  callback_ = callback;
  state_ = RunningState::IDLE;
}

DeviceSubscribeProximitySensorThread::~DeviceSubscribeProximitySensorThread() {
  connection_id_ = -1;
  state_ = RunningState::STOPPED;
  timer_->AbandonAndStop();
  timer_.reset();
  proxySensor->StopPolling();
  proxySensor.reset();
}

void DeviceSubscribeProximitySensorThread::setFilter(base::DictionaryValue* filters) {
  if (filters) {
    filters_ = filters->CreateDeepCopy();
    int32_t interval = 0;
    double minValue = 0, maxValue = 0, minChange = 0;
    base::Value* tempValue = nullptr;
    std::string tempString;
    if (filters_->HasKey("interval")) {
      filters_->Get("interval", &tempValue);
      if (tempValue->GetType() == base::Value::Type::INTEGER) {
        tempValue->GetAsInteger(&interval);
      } else if(tempValue->GetType() == base::Value::Type::STRING) {
        tempValue->GetAsString(&tempString);
        base::StringToInt(tempString, &interval);
      }
      setInterval(interval);
    }
    if (filters_->HasKey("range")) {
      DictionaryValue* tempDictionary;
      filters_->GetDictionary("range", &tempDictionary);
      if (tempDictionary) {
        if (tempDictionary->HasKey("above")) {
          tempDictionary->Get("above", &tempValue);
          if (tempValue->GetType() == base::Value::Type::DOUBLE) {
            tempValue->GetAsDouble(&minValue);
          } else if(tempValue->GetType() == base::Value::Type::STRING) {
            tempValue->GetAsString(&tempString);
            base::StringToDouble(tempString, &minValue);
          }
          setMinValue(minValue);
        }
        if (tempDictionary->HasKey("below")) {
          tempDictionary->Get("below", &tempValue);
          if (tempValue->GetType() == base::Value::Type::DOUBLE) {
            tempValue->GetAsDouble(&maxValue);
          } else if(tempValue->GetType() == base::Value::Type::STRING) {
            tempValue->GetAsString(&tempString);
            base::StringToDouble(tempString, &maxValue);
          }
          setMaxValue(maxValue);
        }
      }
    }
    if (filters_->HasKey("minChange")) {
      filters_->Get("minChange", &tempValue);
      if (tempValue->GetType() == base::Value::Type::DOUBLE) {
        tempValue->GetAsDouble(&minChange);
      } else if(tempValue->GetType() == base::Value::Type::STRING) {
        tempValue->GetAsString(&tempString);
        base::StringToDouble(tempString, &minChange);
      }
      setMinChange(minChange);
    }
  }
}

void DeviceSubscribeProximitySensorThread::notify() {
  if (!(minValue_ != 0 && lastProximityData < minValue_) &&
      !(maxValue_ != 0 && lastProximityData > maxValue_) &&
      !(minChange_ != 0 && std::fabs(lastNotifyData - lastProximityData) < minChange_)) {
    lastNotifyData = lastProximityData;
    BrowserThread::PostTask(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &DeviceSubscribeProximitySensorThread::runCallback, base::Unretained(this)));
  } else {
    lastNotifyData = -1;
  }
}

void DeviceSubscribeProximitySensorThread::runCallback() {
  if (connection_id_ > -1) {
    std::unique_ptr<base::DictionaryValue> result(new base::DictionaryValue());

    result->SetString("subscriptionId", base::Uint64ToString((uint64_t)this));
    DeviceWebsocketHandler::setValue(result.get(), "path", &path_);
    if (state_ == RunningState::RUNNING) {
      std::unique_ptr<base::DictionaryValue> value(new base::DictionaryValue());
      value->SetDouble("value", lastNotifyData);
      result->Set("value", std::move(value));
    } else {
      std::unique_ptr<base::DictionaryValue> tempFilters = filters_->CreateDeepCopy();
      result->Set("filters", std::move(tempFilters));
      std::unique_ptr<base::DictionaryValue> error(new base::DictionaryValue());
      std::string errorMessage = "DeviceSensor is " + base::IntToString(state_);
      DeviceWebsocketHandler::setErrorMessage(error.get(), "500", "devicesensor_internal_error", errorMessage);
      result->Set("error", std::move(error));
    }
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(result.get(), "timestamp", &timestamp);
    callback_.Run(result.release(), connection_id_);
  }
}

bool DeviceSubscribeProximitySensorThread::StartThread(device::mojom::ProximitySensorPtr proxy) {
  if (state_ != RunningState::IDLE && !timer_->IsRunning()) {
    LOG(ERROR) << "Already started thread";
    return false;
  }
  proxySensor = std::move(proxy);
  proxySensor->StartPolling(base::Bind(&DeviceSubscribeProximitySensorThread::DidStart, base::Unretained(this)));
  return DeviceSubscribeThread::StartThread();
}

}  // namespace content
