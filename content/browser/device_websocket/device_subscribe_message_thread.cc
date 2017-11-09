// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_websocket/device_subscribe_message_thread.h"

#include "base/message_loop/message_loop.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/strings/string_number_conversions.h"

#include "content/public/browser/browser_thread.h"

namespace content {

DeviceSubscribeMessageThread::DeviceSubscribeMessageThread(std::string path, int connection_id, HandlerCallback callback) : 
    DeviceSubscribeThread(path, "devicesubscribemessagethread") {
  connection_id_ = connection_id;
  callback_ = callback;
}

DeviceSubscribeMessageThread::~DeviceSubscribeMessageThread() {
  connection_id_ = -1;
}

void DeviceSubscribeMessageThread::notifyFromHandler(device::mojom::MessageObjectPtr message) {
  messageQueue.push(std::move(message));
  task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&DeviceSubscribeMessageThread::notifyFromHandlerInternal,
                 base::Unretained(this)));
}

void DeviceSubscribeMessageThread::notifyFromHandlerInternal() {
  if (!IsRunning()) {
    notify();
  }
}

void DeviceSubscribeMessageThread::setFilter(base::DictionaryValue* filters) {
  if (filters) {
    filters_ = filters->CreateDeepCopy();
    int32_t interval = 0;
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
  }
}

void DeviceSubscribeMessageThread::notify() {
  BrowserThread::PostTask(
    BrowserThread::UI,
    FROM_HERE,
    base::Bind(
        &DeviceSubscribeMessageThread::runCallback, base::Unretained(this)));
}

void DeviceSubscribeMessageThread::runCallback() {
  if (connection_id_ > -1) {
    std::unique_ptr<base::DictionaryValue> result(new base::DictionaryValue());

    result->SetString("subscriptionId", base::Uint64ToString((uint64_t)this));
    DeviceWebsocketHandler::setValue(result.get(), "path", &path_);
    std::unique_ptr<base::ListValue> listValue(new base::ListValue());
    while(!messageQueue.empty()) {
      if (!messageQueue.front().is_null()) {
        std::unique_ptr<base::DictionaryValue> value(new base::DictionaryValue());
        DeviceWebsocketHandler::setValue(value.get(), "id", &messageQueue.front()->id);
        DeviceWebsocketHandler::setValue(value.get(), "type", &messageQueue.front()->type);
        DeviceWebsocketHandler::setValue(value.get(), "to", &messageQueue.front()->to);
        DeviceWebsocketHandler::setValue(value.get(), "from", &messageQueue.front()->from);
        if (messageQueue.front()->title.has_value()) {
          DeviceWebsocketHandler::setValue(value.get(), "title", &messageQueue.front()->title.value());
        } else {
          DeviceWebsocketHandler::setValue(value.get(), "title", nullptr);
        }
        DeviceWebsocketHandler::setValue(value.get(), "body", &messageQueue.front()->body);
        if (messageQueue.front()->date.has_value()) {
          DeviceWebsocketHandler::setValue(value.get(), "date", &messageQueue.front()->date.value());
        } else {
          DeviceWebsocketHandler::setValue(value.get(), "date", nullptr);
        }
        
        listValue->Append(std::move(value));
        messageQueue.pop();
      }
    }
    if (listValue->empty()) {
      result->Set("value", base::WrapUnique<base::Value>(new base::Value()));
    } else {
      result->Set("value", std::move(listValue));
    }
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(result.get(), "timestamp", &timestamp);
    callback_.Run(result.release(), connection_id_);
  }
}

}  // namespace content
