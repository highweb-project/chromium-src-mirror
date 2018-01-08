// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_websocket/device_function_manager.h"

#include "content/public/common/applauncher_api_handler.h"

#include "base/base64.h"
#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"

#include "chrome/browser/profiles/profile.h"

namespace content {
#define CPULOAD_ROUNDING(x, dig) floor((x) * pow(float(10), dig) + 0.5f) / pow(float(10), dig)

DeviceFunctionManager::DeviceFunctionManager() {
}

DeviceFunctionManager::~DeviceFunctionManager() {
  stopManager();
  value_ = nullptr;
  extra_.reset();
}

void DeviceFunctionManager::stopManager() {
  connection_id_ = -1;
}

void DeviceFunctionManager::applauncherGetAppList(DictionaryValue* value, int connection_id, 
                              ApplauncherApiHandler* applauncher_handler,
                              Profile* profile, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;

  if (!applauncher_handler) {
    DeviceApiApplauncherRequestResult result;
    result.functionCode = applauncher_function::APPLAUNCHER_FUNC_GET_APP_LIST;
    result.resultCode = applauncher_code_list::APPLAUNCHER_FAILURE;
    OnApplauncherRequestResult(result);
    return;
  }

  DeviceApiApplauncherRequest request(base::Bind(&DeviceFunctionManager::OnApplauncherRequestResult, base::Unretained(this)));
  request.functionCode = applauncher_function::APPLAUNCHER_FUNC_GET_APP_LIST;
  applauncher_handler->RequestFunction(profile, request);
}

void DeviceFunctionManager::applauncherGetAppplicationInfo(DictionaryValue* value, int connection_id, 
                              ApplauncherApiHandler* applauncher_handler,
                              Profile* profile, std::string appId, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;

  if (!applauncher_handler) {
    DeviceApiApplauncherRequestResult result;
    result.functionCode = applauncher_function::APPLAUNCHER_FUNC_GET_APPLICATION_INFO;
    result.resultCode = applauncher_code_list::APPLAUNCHER_FAILURE;
    OnApplauncherRequestResult(result);
    return;
  }

  DeviceApiApplauncherRequest request(base::Bind(&DeviceFunctionManager::OnApplauncherRequestResult, base::Unretained(this)));
  request.functionCode = applauncher_function::APPLAUNCHER_FUNC_GET_APPLICATION_INFO;
  request.appId = appId;
  applauncher_handler->RequestFunction(profile, request);
}

void DeviceFunctionManager::applauncherLaunchApp(DictionaryValue* value, int connection_id, 
                              ApplauncherApiHandler* applauncher_handler,
                              Profile* profile, DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (!applauncher_handler) {
    DeviceApiApplauncherRequestResult result;
    result.functionCode = applauncher_function::APPLAUNCHER_FUNC_LAUNCH_APP;
    result.resultCode = applauncher_code_list::APPLAUNCHER_FAILURE;
    OnApplauncherRequestResult(result);
    return;
  }
  
  DeviceApiApplauncherRequest request(base::Bind(&DeviceFunctionManager::OnApplauncherRequestResult, base::Unretained(this)));
  request.functionCode = applauncher_function::APPLAUNCHER_FUNC_LAUNCH_APP;
  getStringFromDictionary(extra_.get(), "appId", &request.appId);
  applauncher_handler->RequestFunction(profile, request);
}

void DeviceFunctionManager::applauncherRemoveApp(DictionaryValue* value, int connection_id, 
                              ApplauncherApiHandler* applauncher_handler,
                              Profile* profile, DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (!applauncher_handler) {
    DeviceApiApplauncherRequestResult result;
    result.functionCode = applauncher_function::APPLAUNCHER_FUNC_REMOVE_APP;
    result.resultCode = applauncher_code_list::APPLAUNCHER_FAILURE;
    OnApplauncherRequestResult(result);
    return;
  }
  
  DeviceApiApplauncherRequest request(base::Bind(&DeviceFunctionManager::OnApplauncherRequestResult, base::Unretained(this)));
  request.functionCode = applauncher_function::APPLAUNCHER_FUNC_REMOVE_APP;
  getStringFromDictionary(extra_.get(), "appId", &request.appId);
  applauncher_handler->RequestFunction(profile, request);
}

void DeviceFunctionManager::soundGetOutputDeviceType(DictionaryValue* value, int connection_id,
                              device::mojom::DeviceSoundManager* ptr, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;

  if (ptr) {
    ptr->outputDeviceType(base::Bind(&DeviceFunctionManager::OnDeviceSoundMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "devicesound_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::soundGetDeviceVolume(DictionaryValue* value, int connection_id,
                              device::mojom::DeviceSoundManager* ptr, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;

  if (ptr) {
    ptr->deviceVolume(base::Bind(&DeviceFunctionManager::OnDeviceSoundMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "devicesound_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::getDeviceStorage(DictionaryValue* value, int connection_id,
                          device::mojom::DeviceStorageManager* ptr, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;

  if (ptr) {
    ptr->getDeviceStorage(base::Bind(&DeviceFunctionManager::OnDeviceStorageMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "devicestorage_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::calendarFindEvent(DictionaryValue* value, int connection_id, device::mojom::CalendarManager* ptr, 
                          DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr) {
    bool multiple = true;
    std::string startBefore, startAfter, endBefore, endAfter;

    if (extra_.get() && extra_->HasKey("options")) {
      DictionaryValue* findOptions = nullptr;
      extra_->GetDictionary("options", &findOptions);
      if (findOptions) {
        if (findOptions->HasKey("filter")) {
          DictionaryValue* eventFilter = nullptr;
          findOptions->GetDictionary("filter", &eventFilter);
          if (eventFilter) {
            base::Value* tempValue = nullptr;
            if (eventFilter->HasKey("startBefore")) {
              eventFilter->Get("startBefore", &tempValue);
              startBefore = getStringFromValue(tempValue);
              tempValue = nullptr;
            }
            if (eventFilter->HasKey("startAfter")) {
              eventFilter->Get("startAfter", &tempValue);
              startAfter = getStringFromValue(tempValue);
              tempValue = nullptr;
            }
            if (eventFilter->HasKey("endBefore")) {
              eventFilter->Get("endBefore", &tempValue);
              endBefore = getStringFromValue(tempValue);
              tempValue = nullptr;
            }
            if (eventFilter->HasKey("endAfter")) {
              eventFilter->Get("endAfter", &tempValue);
              endAfter = getStringFromValue(tempValue);
              tempValue = nullptr;
            }
          }
        }
        if (findOptions->HasKey("multiple")) {
          findOptions->GetBoolean("multiple", &multiple);
        }
      }
    }


    ptr->CalendarDeviceApiFindEvent(startBefore, startAfter, endBefore, endAfter, multiple,
                                    base::Bind(&DeviceFunctionManager::OnCalendarMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "calendar_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::calendarAddEvent(DictionaryValue* value, int connection_id, device::mojom::CalendarManager* ptr, 
                          DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr) {
    DictionaryValue* calendarEvent = nullptr;
    if (extra_.get() && extra_->HasKey("event")) {
      extra_->GetDictionary("event", &calendarEvent);
    }
    device::mojom::Calendar_CalendarInfoPtr info(device::mojom::Calendar_CalendarInfo::New());
    convertCalendarInfoValueToMojo(calendarEvent, info.get());
    ptr->CalendarDeviceApiAddEvent(std::move(info), 
                                  base::Bind(&DeviceFunctionManager::OnCalendarMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "calendar_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::calendarUpdateEvent(DictionaryValue* value, int connection_id, device::mojom::CalendarManager* ptr, 
                                                DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr) {
    DictionaryValue* calendarEvent = nullptr;
    if (extra_.get() && extra_->HasKey("event")) {
      extra_->GetDictionary("event", &calendarEvent);
    }
    device::mojom::Calendar_CalendarInfoPtr info(device::mojom::Calendar_CalendarInfo::New());
    convertCalendarInfoValueToMojo(calendarEvent, info.get());
    ptr->CalendarDeviceApiUpdateEvent(std::move(info), 
                                      base::Bind(&DeviceFunctionManager::OnCalendarMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "calendar_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::calendarDeleteEvent(DictionaryValue* value, int connection_id, device::mojom::CalendarManager* ptr, 
                                                DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr) {
    std::string id;
    if (extra_.get() && extra_->HasKey("id")) {
      base::Value* idValue = nullptr;
      extra_->Get("id", &idValue);
      id = getStringFromValue(idValue);
    }
    ptr->CalendarDeviceApiDeleteEvent(id, 
                                      base::Bind(&DeviceFunctionManager::OnCalendarMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "calendar_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::contactFindContact(DictionaryValue* value, int connection_id, device::mojom::ContactManager* ptr, 
                                              DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr) {
    int32_t findTarget = 0;
    int32_t maxItem = 0;
    std::string condition;

    if (extra_.get() && extra_->HasKey("findOptions")) {
      DictionaryValue* findOptions = nullptr;
      base::Value* tempValue = nullptr;
      extra_->GetDictionary("findOptions", &findOptions);
      std::string tempString;
      if (findOptions) {
        if (getStringFromDictionary(findOptions, "target", &tempString)) {
          findTarget = getContactTargetValue(tempString);
        }
        if (findOptions->HasKey("maxItem")) {
          findOptions->Get("maxItem", &tempValue);
          maxItem = getIntegerFromValue(tempValue);
        }
        getStringFromDictionary(findOptions, "condition", &condition);
      }
    }


    ptr->FindContact(3, findTarget, maxItem, condition,
                    base::Bind(&DeviceFunctionManager::OnContactMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "contact_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::contactAddContact(DictionaryValue* value, int connection_id, device::mojom::ContactManager* ptr, 
                                              DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr && extra_.get() && extra_->HasKey("contact")) {
    DictionaryValue* contact = nullptr;
    device::mojom::ContactObjectPtr info(device::mojom::ContactObject::New());
    extra_->GetDictionary("contact", &contact);
    convertContactValueToMojo(contact, info.get());
    ptr->AddContact(0, std::move(info),
                    base::Bind(&DeviceFunctionManager::OnContactMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "contact_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::contactUpdateContact(DictionaryValue* value, int connection_id, device::mojom::ContactManager* ptr, 
                                              DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr && extra_.get() && extra_->HasKey("contact")) {
    DictionaryValue* contact = nullptr;
    device::mojom::ContactObjectPtr info(device::mojom::ContactObject::New());
    extra_->GetDictionary("contact", &contact);
    convertContactValueToMojo(contact, info.get());
    ptr->UpdateContact(1, std::move(info),
                    base::Bind(&DeviceFunctionManager::OnContactMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "contact_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::contactDeleteContact(DictionaryValue* value, int connection_id, device::mojom::ContactManager* ptr, 
                                              DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr) {
    int32_t findTarget = 0;
    int32_t maxItem = 0;
    std::string condition;

    if (extra_.get() && extra_->HasKey("findOptions")) {
      DictionaryValue* findOptions = nullptr;
      base::Value* tempValue = nullptr;
      extra_->GetDictionary("findOptions", &findOptions);
      if (findOptions) {
        std::string tempString;
        if (getStringFromDictionary(findOptions, "target", &tempString)) {
          findTarget = getContactTargetValue(tempString);
        }
        if (findOptions->HasKey("maxItem")) {
          findOptions->Get("maxItem", &tempValue);
          maxItem = getIntegerFromValue(tempValue);
        }
        getStringFromDictionary(findOptions, "condition", &condition);
      }
    }


    ptr->DeleteContact(2, findTarget, maxItem, condition,
                    base::Bind(&DeviceFunctionManager::OnContactMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "contact_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::sendMessage(DictionaryValue* value, int connection_id, device::mojom::MessagingManager* ptr, 
                                        DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  if (ptr) {
    if (extra_.get() && extra_->HasKey("message")) {
      DictionaryValue* message = nullptr;
      extra_->GetDictionary("message", &message);
      if (message) {
        device::mojom::MessageObjectPtr messageObject(device::mojom::MessageObject::New());
        getStringFromDictionary(message, "id", &messageObject->id);
        getStringFromDictionary(message, "type", &messageObject->type);
        getStringFromDictionary(message, "to", &messageObject->to);
        getStringFromDictionary(message, "from", &messageObject->from);
        std::string optionalString;
        if (getStringFromDictionary(message, "title", &optionalString)) {
          messageObject->title = optionalString;
        }
        getStringFromDictionary(message, "body", &messageObject->body);
        if (getStringFromDictionary(message, "date", &optionalString)) {
          messageObject->date = optionalString;
        }
        ptr->SendMessage(std::move(messageObject));
        value_->Set("value", std::move(extra_));
        callback_.Run(value_, connection_id_, this);
        return;
      }
    }
  }
  std::string errorMessage = "DeviceApiManager is null";
  DeviceWebsocketHandler::setErrorMessage(value_, "500", "messaging_internal_error", errorMessage);
  callback_.Run(value_, connection_id_, this);
}


void DeviceFunctionManager::findMessage(DictionaryValue* value, int connection_id, device::mojom::MessagingManager* ptr, 
                                        DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  if (ptr) {
    int32_t findTarget = 0;
    int32_t maxItem = 0;
    std::string condition;

    if (extra_.get() && extra_->HasKey("findOptions")) {
      DictionaryValue* findOptions = nullptr;
      base::Value* tempValue = nullptr;
      extra_->GetDictionary("findOptions", &findOptions);
      if (findOptions) {
        std::string tempString;
        if (getStringFromDictionary(findOptions, "target", &tempString)) {
          if (tempString == "from") {
            findTarget = 0;
          } else if(tempString == "body") {
            findTarget = 1;
          } else {
            findTarget = 99;
          }
        }
        if (findOptions->HasKey("maxItem")) {
          findOptions->Get("maxItem", &tempValue);
          maxItem = getIntegerFromValue(tempValue);
        }
        getStringFromDictionary(findOptions, "condition", &condition);
      }
    }

    ptr->FindMessage(connection_id_, findTarget, maxItem, condition, 
                    base::Bind(&DeviceFunctionManager::OnFindMessagingMojoCallback, base::Unretained(this)));
  } else {
    std::string errorMessage = "DeviceApiManager is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "messaging_internal_error", errorMessage);
    std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
    DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
    callback_.Run(value_, connection_id_, this);
  }
}

void DeviceFunctionManager::setOnMessageReceived(device::mojom::MessagingManager* ptr, MessageCallback callback) {
  message_callback_ = callback;
  if (ptr) {
    uint32_t requestId = (uint32_t)((uint64_t)this & UINT32_MAX);
    ptr->AddMessagingListener(requestId, base::Bind(&DeviceFunctionManager::OnMessageReceivedMojoCallback, base::Unretained(this)));
  }
}

void DeviceFunctionManager::removeOnMessageReceived(device::mojom::MessagingManager* ptr) {
  if (ptr) {
    uint32_t requestId = (uint32_t)((uint64_t)this & UINT32_MAX);
    ptr->RemoveMessagingListener(requestId);
  }
}

void DeviceFunctionManager::galleryFindMedia(DictionaryValue* value, int connection_id, device::mojom::DeviceGalleryManager* ptr, 
                                            DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  std::string errorMessage;
  if (ptr) {
    if (extra_.get() && extra_->HasKey("findOptions")) {
      DictionaryValue* findOptions = nullptr;
      extra_->GetDictionary("findOptions", &findOptions);
      if (findOptions) {
        device::mojom::MojoDeviceGalleryFindOptionsPtr options(device::mojom::MojoDeviceGalleryFindOptions::New());
        options->mObject = device::mojom::MojoDeviceGalleryMediaObject::New();        
        DictionaryValue* findObject = nullptr;
        if (findOptions->HasKey("findObject")) {
          findOptions->GetDictionary("findObject", &findObject);
          if (findObject) {
            convertGalleryMediaObjectToMojo(findObject, options->mObject.get());
          }
        }
        if (findOptions->HasKey("maxItem")) {
          base::Value* tempValue = nullptr;
          findOptions->Get("maxItem", &tempValue);
          options->mMaxItem = getIntegerFromValue(tempValue);
          tempValue = nullptr;
        }
        getStringFromDictionary(findOptions, "operation", &options->mOperation);
        ptr->findMedia(std::move(options), 
                      base::Bind(&DeviceFunctionManager::OnDeviceGalleryMojoCallback, base::Unretained(this)));
        return;
      }
    }
    errorMessage = "gallery findMedia argument not invalid";
  } else {
    errorMessage = "DeviceApiManager is null";
  }
  DeviceWebsocketHandler::setErrorMessage(value_, "500", "gallery_internal_error", errorMessage);
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::galleryGetMedia(DictionaryValue* value, int connection_id, device::mojom::DeviceGalleryManager* ptr, 
                                            DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  std::string errorMessage;
  if (ptr) {
    if (extra_.get() && extra_->HasKey("media")) {
      DictionaryValue* media = nullptr;
      extra_->GetDictionary("media", &media);
      if (media) {
        device::mojom::MojoDeviceGalleryMediaObjectPtr mediaObject(device::mojom::MojoDeviceGalleryMediaObject::New());
        convertGalleryMediaObjectToMojo(media, mediaObject.get());
        ptr->getMedia(std::move(mediaObject), 
                      base::Bind(&DeviceFunctionManager::OnDeviceGalleryMojoCallback, base::Unretained(this)));
        return;
      }
    }
    errorMessage = "gallery findMedia argument not invalid";
  } else {
    errorMessage = "DeviceApiManager is null";
  }
  DeviceWebsocketHandler::setErrorMessage(value_, "500", "gallery_internal_error", errorMessage);
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::galleryDeleteMedia(DictionaryValue* value, int connection_id, device::mojom::DeviceGalleryManager* ptr, 
                                              DictionaryValue* extra, FunctionCallback callback) {
  value_ = value;
  connection_id_ = connection_id;
  callback_ = callback;
  if (extra) {
    extra_ = extra->CreateDeepCopy();
  }

  std::string errorMessage;
  if (ptr) {
    if (extra_.get() && extra_->HasKey("media")) {
      DictionaryValue* media = nullptr;
      extra_->GetDictionary("media", &media);
      if (media) {
        device::mojom::MojoDeviceGalleryMediaObjectPtr mediaObject(device::mojom::MojoDeviceGalleryMediaObject::New());
        convertGalleryMediaObjectToMojo(media, mediaObject.get());
        ptr->deleteMedia(std::move(mediaObject), 
                        base::Bind(&DeviceFunctionManager::OnDeviceGalleryMojoCallback, base::Unretained(this)));
        return;
      }
    }
    errorMessage = "gallery findMedia argument not invalid";
  } else {
    errorMessage = "DeviceApiManager is null";
  }
  DeviceWebsocketHandler::setErrorMessage(value_, "500", "gallery_internal_error", errorMessage);
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::cpuLoad(device::mojom::DeviceCpuManager* ptr, CpuCallback callback) {
  cpu_callback_ = callback;
  if (ptr) {
    ptr->getDeviceCpuLoad(base::Bind(&DeviceFunctionManager::OnDeviceCpuMojoCallback, base::Unretained(this)));
  }
}

void DeviceFunctionManager::OnApplauncherRequestResult(const DeviceApiApplauncherRequestResult result) {
  if (connection_id_ < 0) {
    return;
  }
  if (result.resultCode == applauncher_code_list::APPLAUNCHER_SUCCESS) {
    std::unique_ptr<DictionaryValue> resultValue(new DictionaryValue());
    resultValue->SetInteger("resultCode", result.resultCode);
    switch(result.functionCode) {
      case applauncher_function::APPLAUNCHER_FUNC_GET_APP_LIST: {
        if (!result.applist.empty()) {
          std::unique_ptr<base::ListValue> valueList(new base::ListValue());
          for(AppLauncher_ApplicationInfo info : result.applist) {
            std::unique_ptr<DictionaryValue> value(new DictionaryValue());
            DeviceWebsocketHandler::setValue(value.get(), "id", &info.id);
            DeviceWebsocketHandler::setValue(value.get(), "name", &info.name);
            DeviceWebsocketHandler::setValue(value.get(), "version", &info.version);
            DeviceWebsocketHandler::setValue(value.get(), "url", &info.url);
            DeviceWebsocketHandler::setValue(value.get(), "iconUrl", &info.iconUrl);
            valueList.get()->Append(std::move(value));
          }
          resultValue->Set("appList", std::move(valueList));
        }
        break;
      }
      case applauncher_function::APPLAUNCHER_FUNC_GET_APPLICATION_INFO: {
        if (!result.applist.empty()) {
          AppLauncher_ApplicationInfo info = result.applist[0];
          std::unique_ptr<DictionaryValue> value(new DictionaryValue());
          DeviceWebsocketHandler::setValue(value.get(), "id", &info.id);
          DeviceWebsocketHandler::setValue(value.get(), "name", &info.name);
          DeviceWebsocketHandler::setValue(value.get(), "version", &info.version);
          DeviceWebsocketHandler::setValue(value.get(), "url", &info.url);
          DeviceWebsocketHandler::setValue(value.get(), "iconUrl", &info.iconUrl);
          resultValue->Set("appInfo", std::move(value));
        }
        break;
      }
      case applauncher_function::APPLAUNCHER_FUNC_LAUNCH_APP:
      case applauncher_function::APPLAUNCHER_FUNC_REMOVE_APP: {
        resultValue->Set("value", std::move(extra_));
        break;
      }
    }
    value_->Set("value", std::move(resultValue));
  } else {
    std::string errorMessage = "Applauncher Internal error code " + base::IntToString(result.resultCode);
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "appluancher_internal_error", errorMessage);
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::OnDeviceSoundMojoCallback(device::mojom::DeviceSound_ResultCodePtr result) {
  if (result.get() && result->resultCode == (int32_t)device::mojom::devicesound_ErrorCodeList::SUCCESS) {
    std::unique_ptr<DictionaryValue> resultValue(new DictionaryValue());
    resultValue->SetInteger("resultCode", result->resultCode);
		switch(result->functionCode) {
			case ((int32_t)device::mojom::devicesound_function::FUNC_OUTPUT_DEVICE_TYPE): {
        std::string type = base::IntToString(result->outputType);
        DeviceWebsocketHandler::setValue(resultValue.get(), "outputType", &type);
        value_->Set("value", std::move(resultValue));
				break;
			}
			case ((int32_t)device::mojom::devicesound_function::FUNC_DEVICE_VOLUME): {
        if (result->volume.get() != nullptr) {
          std::unique_ptr<DictionaryValue> value(new DictionaryValue());
          value->SetDouble("MediaVolume", result->volume->MediaVolume);
          value->SetDouble("NotificationVolume", result->volume->NotificationVolume);
          value->SetDouble("AlarmVolume", result->volume->AlarmVolume);
          value->SetDouble("BellVolume", result->volume->BellVolume);
          value->SetDouble("CallVolume", result->volume->CallVolume);
          value->SetDouble("SystemVolume", result->volume->SystemVolume);
          value->SetDouble("DTMFVolume", result->volume->DTMFVolume);
          resultValue->Set("volume", std::move(value));
          value_->Set("value", std::move(resultValue));
        } else {
          std::string errorMessage = "result.volume object is null";
          DeviceWebsocketHandler::setErrorMessage(value_, "500", "devicesound_internal_error", errorMessage);
        }
				break;
			}
		}
  } else {
    std::string errorMessage = "devicesound Internal result ";
    if (result.get()) {
      errorMessage.append(base::IntToString(result->resultCode));
    } else {
      errorMessage.append("is null");
    }
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "devicesound_internal_error", errorMessage);
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::OnDeviceStorageMojoCallback(device::mojom::DeviceStorage_ResultCodePtr result) {
  if (result.get() && result->resultCode == (int32_t)device::mojom::device_storage_ErrorCodeList::SUCCESS) {
    std::unique_ptr<DictionaryValue> resultValue(new DictionaryValue());
    resultValue->SetInteger("resultCode", result->resultCode);
    if (result->storageList.has_value()) {
      size_t size = result->storageList.value().size();
      std::unique_ptr<base::ListValue> valueList(new base::ListValue());
      for(size_t i = 0; i < size; i++) {
        std::unique_ptr<DictionaryValue> value(new DictionaryValue());
        value->SetInteger("type", result->storageList.value()[i].get()->type);
        std::string capacity = base::Uint64ToString(result->storageList.value()[i].get()->capacity);
        std::string availableCapacity = base::Uint64ToString(result->storageList.value()[i].get()->availableCapacity);
        DeviceWebsocketHandler::setValue(value.get(), "capacity", &capacity);
        DeviceWebsocketHandler::setValue(value.get(), "availableCapacity", &availableCapacity);
        value->SetBoolean("isRemovable", result->storageList.value()[i].get()->isRemovable);
        valueList.get()->Append(std::move(value));
      }
      resultValue->Set("storageList", std::move(valueList));
    }
    value_->Set("value", std::move(resultValue));
  } else {
    std::string errorMessage = "devicestorage Internal result ";
    if (result.get()) {
      errorMessage.append(base::IntToString(result->resultCode));
    } else {
      errorMessage.append("is null");
    }
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "devicestorage_internal_error", errorMessage);
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::OnCalendarMojoCallback(device::mojom::Calendar_ResultCodePtr result) {
  if (result) {
    std::unique_ptr<DictionaryValue> resultValue(new DictionaryValue());
    resultValue->SetInteger("resultCode", result->resultCode);
    switch(result->functionCode) {
      case ((int32_t)device::mojom::calendar_function::FUNC_FIND_EVENT): {
        if (result->calendarlist.has_value()) {
          size_t size = result->calendarlist.value().size();
          std::unique_ptr<base::ListValue> valueList(new base::ListValue());
          for(size_t i = 0; i < size; i++) {
            std::unique_ptr<DictionaryValue> value(new DictionaryValue());
            value->SetString("id", result->calendarlist.value()[i].get()->id);
            value->SetString("description", result->calendarlist.value()[i].get()->description);
            value->SetString("location", result->calendarlist.value()[i].get()->location);
            value->SetString("summary", result->calendarlist.value()[i].get()->summary);
            value->SetString("start", result->calendarlist.value()[i].get()->start);
            value->SetString("end", result->calendarlist.value()[i].get()->end);
            value->SetString("status", result->calendarlist.value()[i].get()->status);
            value->SetString("transparency", result->calendarlist.value()[i].get()->transparency);
            value->SetString("reminder", result->calendarlist.value()[i].get()->reminder);
            valueList.get()->Append(std::move(value));
          }
          resultValue->Set("calendarList", std::move(valueList));
        }
        value_->Set("value", std::move(resultValue));
        break;
      }
      case ((int32_t)device::mojom::calendar_function::FUNC_ADD_EVENT):
      case ((int32_t)device::mojom::calendar_function::FUNC_UPDATE_EVENT):
      case ((int32_t)device::mojom::calendar_function::FUNC_DELETE_EVENT): {
        resultValue->Set("value", std::move(extra_));
        value_->Set("value", std::move(resultValue));
        break;
      }
      default: {
        std::string errorMessage = "calendar Internal functioncode not enabled " + base::IntToString(result->functionCode);
        DeviceWebsocketHandler::setErrorMessage(value_, "500", "calendar_internal_error", errorMessage);
        break;
      }
    }
  } else {
    std::string errorMessage = "calendar Internal result is null";
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "calendar_internal_error", errorMessage);
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::OnContactMojoCallback(int32_t requestId, uint32_t error, 
                              base::Optional<std::vector<device::mojom::ContactObjectPtr>> result) {
  if (error == (uint32_t)device::mojom::contact_ErrorCodeList::SUCCESS) {
    std::unique_ptr<DictionaryValue> resultValue(new DictionaryValue());
    if (result.has_value()){
      std::unique_ptr<base::ListValue> listValue(new base::ListValue());
      size_t resultSize = result.value().size();
      for(size_t i=0; i < resultSize; i++) {
        std::unique_ptr<DictionaryValue> value(new DictionaryValue());
        DeviceWebsocketHandler::setValue(value.get(), "id", &result.value()[i].get()->id);
        DeviceWebsocketHandler::setValue(value.get(), "displayName", &result.value()[i].get()->displayName);
        DeviceWebsocketHandler::setValue(value.get(), "phoneNumber", &result.value()[i].get()->phoneNumber);
        DeviceWebsocketHandler::setValue(value.get(), "emails", &result.value()[i].get()->emails);
        DeviceWebsocketHandler::setValue(value.get(), "address", &result.value()[i].get()->address);
        DeviceWebsocketHandler::setValue(value.get(), "accountName", &result.value()[i].get()->accountName);
        DeviceWebsocketHandler::setValue(value.get(), "accountType", &result.value()[i].get()->accountType);

        if(result.value()[i].get()->categories.has_value()) {
          std::unique_ptr<base::ListValue> categories(new base::ListValue());
          categories->AppendStrings(result.value()[i].get()->categories.value());
          value->Set("categories", std::move(categories));
        }

        std::unique_ptr<DictionaryValue> address(new DictionaryValue());
        DeviceWebsocketHandler::setValue(address.get(), "type", &result.value()[i].get()->structuredAddress->type);
        DeviceWebsocketHandler::setValue(address.get(), "streetAddress", &result.value()[i].get()->structuredAddress->streetAddress);
        DeviceWebsocketHandler::setValue(address.get(), "locality", &result.value()[i].get()->structuredAddress->locality);
        DeviceWebsocketHandler::setValue(address.get(), "region", &result.value()[i].get()->structuredAddress->region);
        DeviceWebsocketHandler::setValue(address.get(), "postalCode", &result.value()[i].get()->structuredAddress->postalCode);
        DeviceWebsocketHandler::setValue(address.get(), "country", &result.value()[i].get()->structuredAddress->country);
        value->Set("structuredAddress", std::move(address));

        std::unique_ptr<DictionaryValue> name(new DictionaryValue());
        DeviceWebsocketHandler::setValue(name.get(), "familyName", &result.value()[i].get()->structuredName->familyName);
        DeviceWebsocketHandler::setValue(name.get(), "givenName", &result.value()[i].get()->structuredName->givenName);
        DeviceWebsocketHandler::setValue(name.get(), "middleName", &result.value()[i].get()->structuredName->middleName);
        DeviceWebsocketHandler::setValue(name.get(), "honorificPrefix", &result.value()[i].get()->structuredName->honorificPrefix);
        DeviceWebsocketHandler::setValue(name.get(), "honorificSuffix", &result.value()[i].get()->structuredName->honorificSuffix);
        value->Set("structuredName", std::move(name));
        
        listValue.get()->Append(std::move(value));
      }
      resultValue->Set("contactObjs", std::move(listValue));
	  }
    if (requestId != 3) {
      resultValue->Set("value", std::move(extra_));
    }
    value_->Set("value", std::move(resultValue));
  } else {
    std::string errorMessage = "contact Internal result is error " + base::UintToString(error);
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "contact_internal_error", errorMessage);
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::OnFindMessagingMojoCallback(uint32_t requestId, uint32_t error, 
                              base::Optional<std::vector<device::mojom::MessageObjectPtr>> result) {
  if (error == (uint32_t)device::mojom::messaging_ErrorCodeList::SUCCESS) {
    std::unique_ptr<DictionaryValue> resultValue(new DictionaryValue());
    if (result.has_value()){
      std::unique_ptr<base::ListValue> listValue(new base::ListValue());
      size_t resultSize = result.value().size();
      for(size_t i=0; i < resultSize; i++) {
        std::unique_ptr<DictionaryValue> message(new DictionaryValue());
        convertMojoToMessageValue(result.value()[i].get(), message.get());
        listValue.get()->Append(std::move(message));
      }
      resultValue->Set("messageObjs", std::move(listValue));
	  }
    resultValue->Set("value", std::move(extra_));
    value_->Set("value", std::move(resultValue));
  } else {
    std::string errorMessage = "messaging Internal result is error " + base::UintToString(error);
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "messaging_internal_error", errorMessage);
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::OnDeviceGalleryMojoCallback(device::mojom::DeviceGallery_ResultCodePtr result) {
  if (result.get() && result->resultCode == (int32_t)device::mojom::device_gallery_ErrorCodeList::SUCCESS) {
    if (result->functionCode != (int32_t)device::mojom::device_gallery_function::FUNC_DELETE_MEDIA) {
      std::unique_ptr<DictionaryValue> resultValue(new DictionaryValue());
      if (result->mediaList.has_value()) {
        size_t size = result->mediaList.value().size();
        std::unique_ptr<base::ListValue> valueList(new base::ListValue());
        for(size_t i = 0; i < size; i++) {
          std::unique_ptr<DictionaryValue> value(new DictionaryValue());
          DeviceWebsocketHandler::setValue(value.get(), "type", &result->mediaList.value()[i].get()->mType);
          DeviceWebsocketHandler::setValue(value.get(), "description", &result->mediaList.value()[i].get()->mDescription);
          DeviceWebsocketHandler::setValue(value.get(), "id", &result->mediaList.value()[i].get()->mId);
          DeviceWebsocketHandler::setValue(value.get(), "title", &result->mediaList.value()[i].get()->mTitle);
          DeviceWebsocketHandler::setValue(value.get(), "fileName", &result->mediaList.value()[i].get()->mFileName);
          std::string fileSizeString = base::Int64ToString(result->mediaList.value()[i].get()->mFileSize);
          DeviceWebsocketHandler::setValue(value.get(), "fileSize", &fileSizeString);
          std::string createdDateString = base::Uint64ToString(result->mediaList.value()[i].get()->mCreatedDate);
          DeviceWebsocketHandler::setValue(value.get(), "createdDate", &createdDateString);
          if (result->mediaList.value()[i].get()->mContent.get()) {
            std::unique_ptr<DictionaryValue> content(new DictionaryValue());
            DeviceWebsocketHandler::setValue(content.get(), "uri", &result->mediaList.value()[i].get()->mContent.get()->mUri);
            if (result->mediaList.value()[i].get()->mContent.get()->mBlobSize > 0) {
              std::string tempValue(result->mediaList.value()[i].get()->mContent.get()->mBlob.begin(), 
                                          result->mediaList.value()[i].get()->mContent.get()->mBlob.end());
              std::string base64Value;
              base::Base64Encode(tempValue, &base64Value);
              content->SetString("blob", base64Value);
            } else {
              content->Set("blob", base::WrapUnique<base::Value>(new base::Value()));
            }
            value->Set("content", std::move(content));
          }
          valueList.get()->Append(std::move(value));
        }
        resultValue->Set("mediaList", std::move(valueList));
      }
      value_->Set("value", std::move(resultValue));
    } else {
      value_->Set("value", std::move(extra_));
    }
  } else {
    std::string errorMessage = "devicegallery Internal result ";
    if (result.get()) {
      errorMessage.append(base::IntToString(result->resultCode));
    } else {
      errorMessage.append("is null");
    }
    DeviceWebsocketHandler::setErrorMessage(value_, "500", "gallery_internal_error", errorMessage);
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(value_, "timestamp", &timestamp);
  callback_.Run(value_, connection_id_, this);
}

void DeviceFunctionManager::OnDeviceCpuMojoCallback(device::mojom::DeviceCpu_ResultCodePtr result) {
  int32_t resultCode = -99;
  double load = -1;
  if (result) {
    if (result->functionCode == (int32_t)device::mojom::device_cpu_function::FUNC_GET_CPU_LOAD) {
      resultCode = result->resultCode;
      load = CPULOAD_ROUNDING(result->load, 5);
    }
  }
  cpu_callback_.Run(resultCode, load);
}

void DeviceFunctionManager::OnMessageReceivedMojoCallback(uint32_t requestId, device::mojom::MessageObjectPtr result) {
  device::mojom::MessageObjectPtr clone = result.Clone();
  message_callback_.Run(std::move(clone));
}

bool DeviceFunctionManager::getStringFromDictionary(base::DictionaryValue* value, std::string path, std::string* result) {
  if (value && value->HasKey(path)) {
    value->GetString(path, result);
    return true;
  }
  return false;
}

std::string DeviceFunctionManager::getStringFromValue(base::Value* value) {
  std::string result;
  if (value) {
    switch(value->GetType()) {
      case base::Value::Type::STRING: {
        value->GetAsString(&result);
        break;
      }
      case base::Value::Type::INTEGER: {
        int tempInt = 0;
        value->GetAsInteger(&tempInt);
        result = base::IntToString(tempInt);
        break;
      }
      case base::Value::Type::DOUBLE: {
        double tempDouble = 0;
        value->GetAsDouble(&tempDouble);
        //DoubleToString(1.02e+12) result 1.02e+12
        result = base::Uint64ToString((uint64_t)tempDouble);
        break;
      }
      default: {
        result = "";
      }
    }
  }
  return result;
}

int32_t DeviceFunctionManager::getIntegerFromValue(base::Value* value) {
  int32_t result = 0;
  if (value) {
    switch(value->GetType()) {
      case base::Value::Type::STRING: {
        std::string tempString;
        value->GetAsString(&tempString);
        base::StringToInt(tempString, &result);
        break;
      }
      case base::Value::Type::INTEGER: {
        value->GetAsInteger(&result);
        break;
      }
      default: {
        result = 0;
        break;
      }
    }
  }
  return result;
}

double DeviceFunctionManager::getDoubleFromValue(base::Value* value) {
  double result = 0;
  if (value) {
    switch(value->GetType()) {
      case base::Value::Type::STRING: {
        std::string tempString;
        value->GetAsString(&tempString);
        base::StringToDouble(tempString, &result);
        break;
      }
      case base::Value::Type::INTEGER: {
        int32_t tempInt = 0;
        value->GetAsInteger(&tempInt);
        result = tempInt;
        break;
      }
      case base::Value::Type::DOUBLE: {
        value->GetAsDouble(&result);
        break;
      }
      default: {
        result = 0;
        break;
      }
    }
  }
  return result;
}

int32_t DeviceFunctionManager::getContactTargetValue(std::string& target) {
  if (target == "name") {
    return 0;
  } else if(target == "phone") {
    return 1;
  } else if(target == "email") {
    return 2;
  } else {
    return 99;
  }
}

void DeviceFunctionManager::convertCalendarInfoValueToMojo(DictionaryValue* calendarInfo, device::mojom::Calendar_CalendarInfo* object) {
  if (calendarInfo && object) {
    object->id = "";
    object->description = "";
    object->location = "";
    object->summary = "";
    object->start = "";
    object->end = "";
    base::Value* tempValue = nullptr;
    if (calendarInfo->HasKey("id")) {
      calendarInfo->Get("id", &tempValue);
      object->id = getStringFromValue(tempValue);
      tempValue = nullptr;
    }
    getStringFromDictionary(calendarInfo, "description", &object->description);
    getStringFromDictionary(calendarInfo, "location", &object->location);
    getStringFromDictionary(calendarInfo, "summary", &object->summary);
    if (calendarInfo->HasKey("start")) {
      calendarInfo->Get("start", &tempValue);
      object->start = getStringFromValue(tempValue);
      tempValue = nullptr;
    }
    if (calendarInfo->HasKey("end")) {
      calendarInfo->Get("end", &tempValue);
      object->end = getStringFromValue(tempValue);
      tempValue = nullptr;
    }
  }
}

void DeviceFunctionManager::convertContactValueToMojo(DictionaryValue* contact, device::mojom::ContactObject* object) {
  if (contact && object) {
    object->structuredAddress = device::mojom::ContactAddress::New();
    object->structuredName = device::mojom::ContactName::New();
    getStringFromDictionary(contact, "id", &object->id);
    getStringFromDictionary(contact, "displayName", &object->displayName);
    getStringFromDictionary(contact, "phoneNumber", &object->phoneNumber);
    getStringFromDictionary(contact, "emails", &object->emails);
    getStringFromDictionary(contact, "address", &object->address);
    getStringFromDictionary(contact, "accountName", &object->accountName);
    getStringFromDictionary(contact, "accountType", &object->accountType);

    base::ListValue* categories = nullptr;
    if (contact->HasKey("categories")) {
      base::Value* tempValue = nullptr;
      contact->Get("categories", &tempValue);
      if (tempValue && tempValue->IsType(base::Value::Type::LIST)) {
        tempValue->GetAsList(&categories);
      }
    }
    if (categories && !categories->empty()) {
      object->categories = std::vector<std::string>();
      for(base::ListValue::iterator it = categories->begin(); it != categories->end(); ++it) {
        std::string stringValue;
        it->GetAsString(&stringValue);
        object->categories.value().push_back(stringValue);
      }
    }

    object->structuredAddress->type = "";
    object->structuredAddress->streetAddress = "";
    object->structuredAddress->locality = "";
    object->structuredAddress->region = "";
    object->structuredAddress->country = "";
    object->structuredAddress->postalCode = "";
    if (contact->HasKey("structuredAddress")) {
      DictionaryValue* address = nullptr;
      contact->GetDictionary("structuredAddress", &address);
      if (address) {
        getStringFromDictionary(address, "type", &object->structuredAddress->type);
        getStringFromDictionary(address, "streetAddress", &object->structuredAddress->streetAddress);
        getStringFromDictionary(address, "locality", &object->structuredAddress->locality);
        getStringFromDictionary(address, "region", &object->structuredAddress->region);
        getStringFromDictionary(address, "country", &object->structuredAddress->country);
        getStringFromDictionary(address, "postalCode", &object->structuredAddress->postalCode);
      }
    } 

    object->structuredName->familyName = "";
    object->structuredName->givenName = "";
    object->structuredName->middleName = "";
    object->structuredName->honorificPrefix = "";
    object->structuredName->honorificSuffix = "";
    if (contact->HasKey("structuredName")) {
      DictionaryValue* name = nullptr;
      contact->GetDictionary("structuredName", &name);
      if (name) {
        getStringFromDictionary(name, "familyName", &object->structuredName->familyName);
        getStringFromDictionary(name, "givenName", &object->structuredName->givenName);
        getStringFromDictionary(name, "middleName", &object->structuredName->middleName);
        getStringFromDictionary(name, "honorificPrefix", &object->structuredName->honorificPrefix);
        getStringFromDictionary(name, "honorificSuffix", &object->structuredName->honorificSuffix);
      } 
    }
  }
}

void DeviceFunctionManager::convertMojoToMessageValue(device::mojom::MessageObject* object, DictionaryValue* message) {
  DeviceWebsocketHandler::setValue(message, "id", &object->id);
	DeviceWebsocketHandler::setValue(message, "type", &object->type);
	DeviceWebsocketHandler::setValue(message, "to", &object->to);
	DeviceWebsocketHandler::setValue(message, "from", &object->from);
  if (object->title.has_value()) {
    DeviceWebsocketHandler::setValue(message, "title", &object->title.value());
  } else {
    DeviceWebsocketHandler::setValue(message, "title", nullptr);
  }
	DeviceWebsocketHandler::setValue(message, "body", &object->body);
  if (object->date.has_value()) {
	  DeviceWebsocketHandler::setValue(message, "date", &object->date.value());
  } else {
    DeviceWebsocketHandler::setValue(message, "date", nullptr);
  }
}

void DeviceFunctionManager::convertGalleryMediaObjectToMojo(DictionaryValue* object, device::mojom::MojoDeviceGalleryMediaObject* mediaObject) {
  mediaObject->mContent = device::mojom::MojoDeviceGalleryMediaContent::New();
  getStringFromDictionary(object, "type", &mediaObject->mType);
  getStringFromDictionary(object, "description", &mediaObject->mDescription);
  getStringFromDictionary(object, "id", &mediaObject->mId);
  getStringFromDictionary(object, "title", &mediaObject->mTitle);
  getStringFromDictionary(object, "fileName", &mediaObject->mFileName);
  if (object->HasKey("fileSize")) {
    base::Value* tempValue = nullptr;
    object->Get("fileSize", &tempValue);
    mediaObject->mFileSize = getIntegerFromValue(tempValue);
    tempValue = nullptr;
  }
  if (object->HasKey("createdDate")) {
    base::Value* tempValue = nullptr;
    object->Get("createdDate", &tempValue);
    mediaObject->mCreatedDate = getDoubleFromValue(tempValue);
    tempValue = nullptr;
  }
  if (object->HasKey("content")) {
    DictionaryValue* content = nullptr;
    object->GetDictionary("content", &content);
    if (content) {
      if (content->HasKey("blob")) {
        base::Value* blobValue = nullptr;
        content->GetBinary("blob", &blobValue);
        if (blobValue) {
          std::vector<char> blobVector;
          blobVector.assign(blobValue->GetBlob().begin(), blobValue->GetBlob().end());
          std::vector<uint8_t> blobConvertVector(blobVector.begin(), blobVector.end());
          mediaObject->mContent->mBlob = blobConvertVector;
        }
      }
      getStringFromDictionary(content, "uri", &mediaObject->mContent->mUri);
    }
  }
}

}  // namespace content
