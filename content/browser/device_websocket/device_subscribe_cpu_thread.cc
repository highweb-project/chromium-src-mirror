// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_websocket/device_subscribe_cpu_thread.h"

#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"

#include "content/public/browser/browser_thread.h"

namespace content {

DeviceSubscribeCpuThread::DeviceSubscribeCpuThread(std::string path, int connection_id, HandlerCallback callback) : 
    DeviceSubscribeThread(path, "devicesubscribecputhread") {
  connection_id_ = connection_id;
  callback_ = callback;
}

DeviceSubscribeCpuThread::~DeviceSubscribeCpuThread() {
  connection_id_ = -1;
}

void DeviceSubscribeCpuThread::notifyFromHandler(int32_t resultCode, double loadValue) {
  task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&DeviceSubscribeCpuThread::notifyFromHandlerInternal,
                 base::Unretained(this), resultCode, loadValue));
}

void DeviceSubscribeCpuThread::notifyFromHandlerInternal(int32_t resultCode, double loadValue) {
  lastLoadData = loadValue;
  resultCode_ = resultCode;
  if (!IsRunning()) {
    notify();
  }
}

void DeviceSubscribeCpuThread::setFilter(base::DictionaryValue* filters) {
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

void DeviceSubscribeCpuThread::notify() {
  if (!(minValue_ != 0 && lastLoadData < minValue_) &&
      !(maxValue_ != 0 && lastLoadData > maxValue_) &&
      !(minChange_ != 0 && std::fabs(lastNotifyData - lastLoadData) < minChange_)) {
    lastNotifyData = lastLoadData;
    BrowserThread::PostTask(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &DeviceSubscribeCpuThread::runCallback, base::Unretained(this)));
  } else {
    lastNotifyData = -1;
  }
}

void DeviceSubscribeCpuThread::runCallback() {
  if (connection_id_ > -1) {
    std::unique_ptr<base::DictionaryValue> result(new base::DictionaryValue());

    result->SetString("subscriptionId", base::Uint64ToString((uint64_t)this));
    DeviceWebsocketHandler::setValue(result.get(), "path", &path_);
    if (resultCode_ == 0) {
      std::unique_ptr<base::DictionaryValue> value(new base::DictionaryValue());
      value->SetInteger("resultCode", 0);
      value->SetDouble("load", lastNotifyData);
      result->Set("value", std::move(value));
    } else {
      std::unique_ptr<base::DictionaryValue> tempFilters = filters_->CreateDeepCopy();
      result->Set("filters", std::move(tempFilters));
      std::unique_ptr<base::DictionaryValue> error(new base::DictionaryValue());
      std::string errorMessage = "DeviceCpu is " + base::IntToString(resultCode_);
      DeviceWebsocketHandler::setErrorMessage(error.get(), "500", "devicecpu_internal_error", errorMessage);
      result->Set("error", std::move(error));
    }
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(result.get(), "timestamp", &timestamp);
    callback_.Run(result.release(), connection_id_);
  }
}

}  // namespace content
