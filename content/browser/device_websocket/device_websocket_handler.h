// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_HANDLER_H_
#define CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_HANDLER_H_

#include <map>
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread.h"
#include "net/http/http_status_code.h"

#include "chrome/browser/profiles/profile.h"
#include "content/browser/device_websocket/device_websocket_constants.h"
#include "content/browser/device_websocket/device_function_manager.h"

namespace net {
class HttpServerRequestInfo;
}

#include "services/device/public/interfaces/calendar_manager.mojom.h"
#include "services/device/public/interfaces/contact_manager.mojom.h"
#include "services/device/public/interfaces/devicecpu_manager.mojom.h"
#include "services/device/public/interfaces/devicegallery_manager.mojom.h"
#include "services/device/public/interfaces/messaging_manager.mojom.h"
#include "device/sensors/public/interfaces/proximity.mojom.h"
#include "services/device/public/interfaces/devicesound_manager.mojom.h"
#include "services/device/public/interfaces/devicestorage_manager.mojom.h"

#include "net/socket/server_socket.h"
#include "content/public/common/device_api_applauncher_request.h"

using base::DictionaryValue;

namespace content {
class ApplauncherApiHandler;
class DeviceWebServerWrapper;
class DeviceFunctionManager;
class DeviceSubscribeThread;

class DeviceWebsocketHandler {
  public:

    DeviceWebsocketHandler(std::unique_ptr<net::ServerSocket> server_socket, base::Thread* thread, Profile* profile);

    ~DeviceWebsocketHandler();

    void OnWebSocketRequest(int connection_id,
                      const net::HttpServerRequestInfo& info);
    void AcceptWebSocket(int connection_id,
                      const net::HttpServerRequestInfo& request);
    void OnClose(int connection_id);
    void StopHandler();
    void stopAllFunctionManager();
    void stopAllObserver();
    void closeAllConnection();
    void SetDeviceAPIHandler(ApplauncherApiHandler* handler);
    void initDeviceManager();

    void handleSocketMessage(int connection_id, const std::string& data);
    
    void sendOverWebsocket(DictionaryValue* value, int connection_id);
    static void setErrorMessage(DictionaryValue* value, 
                                std::string number, 
                                std::string reason, 
                                std::string message);
    static void setErrorMessage(DictionaryValue* value, DeviceWebsocketConstants::InternalErrorCode errorCode);
    static void setValue(base::Value* value, std::string path, std::string* data);

    void OnFunctionCallback(DictionaryValue* value, int connection_id, DeviceFunctionManager* manager);
    void OnCpuCallback(int32_t resultCode, double load);
    void OnMessageCallback(device::mojom::MessageObjectPtr message);
    void observerNotiy(DictionaryValue* value, int connection_id);
  private:
    void setFiltersValue(DictionaryValue* filters, DeviceSubscribeThread* thread);
    bool splitPath(std::vector<std::string>* path_vector, std::string path);
    bool checkAuthorize(int connection_id, std::string& requestId);
    bool removeObserver(uint64_t subscriptionId);

    void handleAuthorize(DictionaryValue* data, int connection_id, std::string requestId);
    void handleUnsubscribe(DictionaryValue* data, int connection_id, std::string action, uint64_t subscriptionId);
    void handleGetFunction(DictionaryValue* data, int connection_id, std::vector<std::string>& path_vector, DictionaryValue* extra);
    void handleSetFunction(DictionaryValue* data, int connection_id, std::vector<std::string>& path_vector, DictionaryValue* extra);
    void handleSubscribeFunction(DictionaryValue* data, int connection_id, std::vector<std::string>& path_vector, DictionaryValue* extra);

    DeviceWebServerWrapper* server_wrapper_;

    Profile* profile_;
    ApplauncherApiHandler* applauncher_handler_;
    device::mojom::DeviceSoundManagerPtr deviceSoundManager;
    device::mojom::DeviceStorageManagerPtr deviceStorageManager;
    device::mojom::DeviceCpuManagerPtr deviceCpuManager;
    device::mojom::CalendarManagerPtr calendarManager;
    device::mojom::ContactManagerPtr contactManager;
    device::mojom::MessagingManagerPtr messagingManager;
    device::mojom::DeviceGalleryManagerPtr deviceGalleryManager;
    
    base::Thread* thread_;

    using RequestIdMap = std::map<std::string, int>;
    using ConnectionIdMap = std::map<int, std::string>;
    RequestIdMap requestId_map_;
    ConnectionIdMap connectionId_map_;
    std::map<std::string, int> functionMap;
    std::map<std::string, int> pathMap;

    std::multimap<int, uint64_t> subscribeId_map_;

    std::set<DeviceFunctionManager*> managerSet;
    std::set<uint64_t> deviceCpuThreadSet;
    std::set<uint64_t> messageThreadSet;
    std::set<uint64_t> proximityThreadSet;

    std::unique_ptr<DeviceFunctionManager> cpuFunctionManager;
    std::unique_ptr<DeviceFunctionManager> messageFunctionManager;

    base::WeakPtrFactory<DeviceWebsocketHandler> weak_factory_;
    DISALLOW_COPY_AND_ASSIGN(DeviceWebsocketHandler);
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_HANDLER_H_
