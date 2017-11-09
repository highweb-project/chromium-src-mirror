// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_FUNCTION_MANAGER_H_
#define CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_FUNCTION_MANAGER_H_

#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "content/browser/device_websocket/device_websocket_constants.h"
#include "content/browser/device_websocket/device_websocket_handler.h"

#include "content/public/common/device_api_applauncher_request.h"
#include "services/device/public/interfaces/calendar_manager.mojom.h"
#include "services/device/public/interfaces/contact_manager.mojom.h"
#include "services/device/public/interfaces/devicecpu_manager.mojom.h"
#include "services/device/public/interfaces/devicegallery_manager.mojom.h"
#include "services/device/public/interfaces/messaging_manager.mojom.h"
#include "services/device/public/interfaces/devicesound_manager.mojom.h"
#include "services/device/public/interfaces/devicestorage_manager.mojom.h"

using base::DictionaryValue;

namespace content {
class ApplauncherApiHandler;

class DeviceFunctionManager {
  public:
    typedef base::Callback<void(DictionaryValue* value, int connection_id, DeviceFunctionManager* manager)> FunctionCallback;
    typedef base::Callback<void(int32_t resultCode, double load)> CpuCallback;
    typedef base::Callback<void(device::mojom::MessageObjectPtr message)> MessageCallback;
    DeviceFunctionManager();

    ~DeviceFunctionManager();
    void stopManager();

    void applauncherGetAppList(DictionaryValue* value, int connection_id, 
                              ApplauncherApiHandler* applauncher_handler,
                              Profile* profile, FunctionCallback callback);
    void applauncherGetAppplicationInfo(DictionaryValue* value, int connection_id, 
                              ApplauncherApiHandler* applauncher_handler,
                              Profile* profile, std::string appId, FunctionCallback callback);
    void applauncherLaunchApp(DictionaryValue* value, int connection_id, 
                              ApplauncherApiHandler* applauncher_handler,
                              Profile* profile, DictionaryValue* extra, FunctionCallback callback);
    void applauncherRemoveApp(DictionaryValue* value, int connection_id, 
                              ApplauncherApiHandler* applauncher_handler,
                              Profile* profile, DictionaryValue* extra, FunctionCallback callback);
                              
    void soundGetOutputDeviceType(DictionaryValue* value, int connection_id,
                              device::mojom::DeviceSoundManager* ptr, FunctionCallback callback);
    void soundGetDeviceVolume(DictionaryValue* value, int connection_id,
                              device::mojom::DeviceSoundManager* ptr, FunctionCallback callback);

    void getDeviceStorage(DictionaryValue* value, int connection_id,
                          device::mojom::DeviceStorageManager* ptr, FunctionCallback callback);

    void calendarFindEvent(DictionaryValue* value, int connection_id, device::mojom::CalendarManager* ptr, 
                          DictionaryValue* extra, FunctionCallback callback);
    void calendarAddEvent(DictionaryValue* value, int connection_id, device::mojom::CalendarManager* ptr, 
                          DictionaryValue* extra, FunctionCallback callback);
    void calendarUpdateEvent(DictionaryValue* value, int connection_id, device::mojom::CalendarManager* ptr, 
                            DictionaryValue* extra, FunctionCallback callback);
    void calendarDeleteEvent(DictionaryValue* value, int connection_id, device::mojom::CalendarManager* ptr, 
                            DictionaryValue* extra, FunctionCallback callback);

    void contactFindContact(DictionaryValue* value, int connection_id, device::mojom::ContactManager* ptr, 
                            DictionaryValue* extra, FunctionCallback callback);
    void contactAddContact(DictionaryValue* value, int connection_id, device::mojom::ContactManager* ptr, 
                            DictionaryValue* extra, FunctionCallback callback);
    void contactUpdateContact(DictionaryValue* value, int connection_id, device::mojom::ContactManager* ptr, 
                              DictionaryValue* extra, FunctionCallback callback);
    void contactDeleteContact(DictionaryValue* value, int connection_id, device::mojom::ContactManager* ptr, 
                              DictionaryValue* extra, FunctionCallback callback);

    void sendMessage(DictionaryValue* value, int connection_id, device::mojom::MessagingManager* ptr, 
                    DictionaryValue* extra, FunctionCallback callback);
    void findMessage(DictionaryValue* value, int connection_id, device::mojom::MessagingManager* ptr, 
                    DictionaryValue* extra, FunctionCallback callback);
    void setOnMessageReceived(device::mojom::MessagingManager* ptr, MessageCallback callback);
    void removeOnMessageReceived(device::mojom::MessagingManager* ptr);

    void galleryFindMedia(DictionaryValue* value, int connection_id, device::mojom::DeviceGalleryManager* ptr, 
                          DictionaryValue* extra, FunctionCallback callback);
    void galleryGetMedia(DictionaryValue* value, int connection_id, device::mojom::DeviceGalleryManager* ptr, 
                        DictionaryValue* extra, FunctionCallback callback);
    void galleryDeleteMedia(DictionaryValue* value, int connection_id, device::mojom::DeviceGalleryManager* ptr, 
                            DictionaryValue* extra, FunctionCallback callback);

    void cpuLoad(device::mojom::DeviceCpuManager* ptr, CpuCallback callback);

    void OnApplauncherRequestResult(const DeviceApiApplauncherRequestResult result);
    void OnDeviceSoundMojoCallback(device::mojom::DeviceSound_ResultCodePtr result);
    void OnDeviceStorageMojoCallback(device::mojom::DeviceStorage_ResultCodePtr result);
    void OnCalendarMojoCallback(device::mojom::Calendar_ResultCodePtr result);
    void OnContactMojoCallback(int32_t requestId, uint32_t error, 
                              base::Optional<std::vector<device::mojom::ContactObjectPtr>> result);
    void OnFindMessagingMojoCallback(uint32_t requestId, uint32_t error, 
                                    base::Optional<std::vector<device::mojom::MessageObjectPtr>> result);
    void OnDeviceGalleryMojoCallback(device::mojom::DeviceGallery_ResultCodePtr result);
    void OnDeviceCpuMojoCallback(device::mojom::DeviceCpu_ResultCodePtr result);
    void OnMessageReceivedMojoCallback(uint32_t requestId, device::mojom::MessageObjectPtr result);
  private:
    bool getStringFromDictionary(base::DictionaryValue* value, std::string path, std::string* result);
    std::string getStringFromValue(base::Value* value);
    int32_t getIntegerFromValue(base::Value* value);
    double getDoubleFromValue(base::Value* value);

    int32_t getContactTargetValue(std::string& target);
    void convertCalendarInfoValueToMojo(DictionaryValue* calendarInfo, device::mojom::Calendar_CalendarInfo* object);
    void convertContactValueToMojo(DictionaryValue* contact, device::mojom::ContactObject* object);
    void convertMojoToMessageValue(device::mojom::MessageObject* object, DictionaryValue* message);
    void convertGalleryMediaObjectToMojo(DictionaryValue* object, device::mojom::MojoDeviceGalleryMediaObject* mediaObject);

    int connection_id_;
    DictionaryValue* value_;
    std::unique_ptr<DictionaryValue> extra_;
    FunctionCallback callback_;
    CpuCallback cpu_callback_;
    MessageCallback message_callback_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_FUNCTION_MANAGER_H_
