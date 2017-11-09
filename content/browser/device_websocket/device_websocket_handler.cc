// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_websocket/device_websocket_handler.h"
#include "content/browser/device_websocket/device_function_manager.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "device/sensors/device_sensor_host.h"
#include "net/server/http_server.h"
#include "net/server/http_server_request_info.h"
#include "net/server/http_server_response_info.h"
#include "net/socket/server_socket.h"

#include "content/browser/device_websocket/device_subscribe_cpu_thread.h"
#include "content/browser/device_websocket/device_subscribe_message_thread.h"
#include "content/browser/device_websocket/device_subscribe_proximity_thread.h"
#include "content/public/common/applauncher_api_handler.h"
#if defined(OS_ANDROID)
#include "content/browser/android/java_interfaces_impl.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#elif defined(OS_LINUX)
#include "content/public/common/service_names.mojom.h"
#include "services/device/sound/devicesound_manager_impl.h"
#include "services/device/storage/devicestorage_manager_impl.h"
#include "services/device/cpu/devicecpu_manager_impl.h"
#include "content/public/common/service_manager_connection.h"
#include "services/service_manager/public/cpp/connector.h"
#include "services/device/public/interfaces/constants.mojom.h"
#endif

namespace content {

namespace {

const int32_t kSendBufferSizeForDeviceWebServer = 256 * 1024 * 1024;  // 256Mb

}  // namespace

class DeviceWebServerWrapper : net::HttpServer::Delegate {
 public:
  DeviceWebServerWrapper(base::WeakPtr<DeviceWebsocketHandler> handler,
                std::unique_ptr<net::ServerSocket> socket);

  void AcceptWebSocket(int connection_id,
                       const net::HttpServerRequestInfo& request);
  void SendOverWebSocket(int connection_id, const std::string& message);
  void Send404(int connection_id);
  void Close(int connection_id);

  bool server_enabled() {
    return server_enabled_;
  }

 private:
  void OnConnect(int connection_id) override;
  void OnHttpRequest(int connection_id,
                     const net::HttpServerRequestInfo& info) override;
  void OnWebSocketRequest(int connection_id,
                          const net::HttpServerRequestInfo& info) override;
  void OnWebSocketMessage(int connection_id,
                          const std::string& data) override;
  void OnClose(int connection_id) override;

  base::WeakPtr<DeviceWebsocketHandler> handler_;
  std::unique_ptr<net::HttpServer> server_;

  bool server_enabled_ = false;
};

DeviceWebServerWrapper::DeviceWebServerWrapper(base::WeakPtr<DeviceWebsocketHandler> handler,
                             std::unique_ptr<net::ServerSocket> socket)
    : handler_(handler) {
  if (socket.get()) {
    server_.reset(new net::HttpServer(std::move(socket), this));
    server_enabled_ = true;
  }
}

void DeviceWebServerWrapper::OnConnect(int connection_id) {
}

void DeviceWebServerWrapper::AcceptWebSocket(int connection_id,
                                    const net::HttpServerRequestInfo& request) {
  server_->SetSendBufferSize(connection_id, kSendBufferSizeForDeviceWebServer);
  server_->AcceptWebSocket(connection_id, request);
}

void DeviceWebServerWrapper::SendOverWebSocket(int connection_id,
                                      const std::string& message) {
  if (server_enabled_) {
    server_->SendOverWebSocket(connection_id, message);
  }
}

void DeviceWebServerWrapper::Close(int connection_id) {
  server_->Close(connection_id);
}

void DeviceWebServerWrapper::Send404(int connection_id) {
  server_->Send404(connection_id);
}

void DeviceWebServerWrapper::OnHttpRequest(int connection_id,
                                  const net::HttpServerRequestInfo& info) {
  server_->Send404(connection_id);
}

void DeviceWebServerWrapper::OnWebSocketRequest(
    int connection_id,
    const net::HttpServerRequestInfo& request) {
  BrowserThread::PostTask(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &DeviceWebsocketHandler::OnWebSocketRequest,
          handler_,
          connection_id,
          request));
}

void DeviceWebServerWrapper::OnWebSocketMessage(int connection_id,
                                       const std::string& data) {
  BrowserThread::PostTask(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &DeviceWebsocketHandler::handleSocketMessage,
          handler_,
          connection_id,
          data));
}

void DeviceWebServerWrapper::OnClose(int connection_id) {
  BrowserThread::PostTask(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &DeviceWebsocketHandler::OnClose,
          handler_,
          connection_id));
}

// DeviceWebsocketHandler -------------------------------------------------------

DeviceWebsocketHandler::~DeviceWebsocketHandler() {
  StopHandler();
  requestId_map_.clear();
  connectionId_map_.clear();
  subscribeId_map_.clear();

  deviceCpuThreadSet.clear();
  messageThreadSet.clear();
  proximityThreadSet.clear();

  if (deviceSoundManager.is_bound()) {
		deviceSoundManager.reset();
	}
  if (deviceStorageManager.is_bound()) {
    deviceStorageManager.reset();
  }
  if (deviceCpuManager.is_bound()) {
    deviceCpuManager->stopCpuLoad();
    deviceCpuManager.reset();
  }
  if (calendarManager.is_bound()) {
    calendarManager.reset();
  }
  if (contactManager.is_bound()) {
    contactManager.reset();
  }
  if (messagingManager.is_bound()) {
    messagingManager.reset();
  }
  if (deviceGalleryManager.is_bound()) {
    deviceGalleryManager.reset();
  }
}

void DeviceWebsocketHandler::StopHandler() {
  if (!connectionId_map_.empty()) {
    closeAllConnection();
  }
  if (!managerSet.empty()) {
    stopAllFunctionManager();
  }
  stopAllObserver();
  profile_ = nullptr;
  thread_ = nullptr;
  applauncher_handler_ = nullptr;
}

void DeviceWebsocketHandler::closeAllConnection() {
  for(ConnectionIdMap::iterator it = connectionId_map_.begin();
      it != connectionId_map_.end(); ++it) {
    if (thread_) {
      thread_->task_runner()->PostTask(
        FROM_HERE,
        base::Bind(&DeviceWebServerWrapper::Close,
                  base::Unretained(server_wrapper_), it->first));
    }
  }
}

void DeviceWebsocketHandler::stopAllFunctionManager() {
  for(std::set<DeviceFunctionManager*>::iterator it = managerSet.begin();
      it != managerSet.end(); ++it) {
    (*it)->stopManager();
  }
  managerSet.clear();
}

void DeviceWebsocketHandler::stopAllObserver() {
  for(std::multimap<int, uint64_t>::iterator it = subscribeId_map_.begin();
      it != subscribeId_map_.end(); ++it) {
    DeviceSubscribeThread* thread = (DeviceSubscribeThread*)(it->second);
    if (thread) {
      delete thread;
    }
  }
  subscribeId_map_.clear();
  deviceCpuThreadSet.clear();
  messageThreadSet.clear();
  proximityThreadSet.clear();
  if (messageFunctionManager.get()) {
    messageFunctionManager->removeOnMessageReceived(messagingManager.get());
    messageFunctionManager.reset();
  }
}

void DeviceWebsocketHandler::OnWebSocketRequest(
    int connection_id,
    const net::HttpServerRequestInfo& request) {
  if (!(thread_ && thread_->IsRunning())) {
    return;
  }
  if (connectionId_map_.find(connection_id) == connectionId_map_.end()) {
    connectionId_map_[connection_id] = std::string();
  }
  AcceptWebSocket(connection_id, request);
}

void DeviceWebsocketHandler::OnClose(int connection_id) {
  if (connectionId_map_.find(connection_id) != connectionId_map_.end()) {
    std::string value = connectionId_map_[connection_id];
    connectionId_map_.erase(connection_id);
    if (requestId_map_.find(value) != requestId_map_.end()) {
      requestId_map_.erase(value);
    }
  }
  std::multimap<int, uint64_t>::iterator it = subscribeId_map_.find(connection_id);
  while(it != subscribeId_map_.end()) {
    if (removeObserver(it->second)) {
      subscribeId_map_.erase(it);
    }
    it = subscribeId_map_.find(connection_id);
  }
}

DeviceWebsocketHandler::DeviceWebsocketHandler(
    std::unique_ptr<net::ServerSocket> socket,
    base::Thread* thread,
    Profile* profile)
      : profile_(profile), applauncher_handler_(nullptr), weak_factory_(this) {
  server_wrapper_ = new DeviceWebServerWrapper(weak_factory_.GetWeakPtr(), std::move(socket));
  if (!server_wrapper_->server_enabled()) {
    StopHandler();
  }
  thread_ = thread;
  requestId_map_.clear();
  connectionId_map_.clear();
  managerSet.clear();
  functionMap.clear();
  functionMap["authorize"] = DeviceWebsocketConstants::FunctionCode::AUTHORIZE;
  functionMap["get"] = DeviceWebsocketConstants::FunctionCode::GET;
  functionMap["set"] = DeviceWebsocketConstants::FunctionCode::SET;
  functionMap["subscribe"] = DeviceWebsocketConstants::FunctionCode::SUBSCRIBE;
  functionMap["unsubscribe"] = DeviceWebsocketConstants::FunctionCode::UNSUBSCRIBE;
  functionMap["unsubscribeAll"] = DeviceWebsocketConstants::FunctionCode::UNSUBSCRIBEALL;
  functionMap["unsubscribeall"] = DeviceWebsocketConstants::FunctionCode::UNSUBSCRIBEALL;

  pathMap.clear();
  pathMap["highweb"] = DeviceWebsocketConstants::PathCode::HIGHWEB;
  pathMap["applauncher"] = DeviceWebsocketConstants::PathCode::APPLAUNCHER;
  pathMap["launchapp"] = DeviceWebsocketConstants::PathCode::LAUNCHAPP;
  pathMap["removeapp"] = DeviceWebsocketConstants::PathCode::REMOVEAPP;
  pathMap["getapplist"] = DeviceWebsocketConstants::PathCode::GETAPPLIST;
  pathMap["getapplicationinfo"] = DeviceWebsocketConstants::PathCode::GETAPPLICATIONINFO;
  pathMap["systeminfo"] = DeviceWebsocketConstants::PathCode::SYSTEMINFO;
  pathMap["devicesound"] = DeviceWebsocketConstants::PathCode::DEVICESOUND;
  pathMap["outputdevicetype"] = DeviceWebsocketConstants::PathCode::OUTPUTDEVICETYPE;
  pathMap["devicevolume"] = DeviceWebsocketConstants::PathCode::DEVICEVOLUME;
  pathMap["devicestorage"] = DeviceWebsocketConstants::PathCode::DEVICESTORAGE;
  pathMap["getdevicestorage"] = DeviceWebsocketConstants::PathCode::GETDEVICESTORAGE;
  pathMap["devicecpu"] = DeviceWebsocketConstants::PathCode::DEVICECPU;
  pathMap["load"] = DeviceWebsocketConstants::PathCode::LOAD;
  pathMap["calendar"] = DeviceWebsocketConstants::PathCode::CALENDAR;
  pathMap["findevent"] = DeviceWebsocketConstants::PathCode::FINDEVENT;
  pathMap["addevent"] = DeviceWebsocketConstants::PathCode::ADDEVENT;
  pathMap["updateevent"] = DeviceWebsocketConstants::PathCode::UPDATEEVENT;
  pathMap["deleteevent"] = DeviceWebsocketConstants::PathCode::DELETEEVENT;
  pathMap["contact"] = DeviceWebsocketConstants::PathCode::CONTACT;
  pathMap["findcontact"] = DeviceWebsocketConstants::PathCode::FINDCONTACT;
  pathMap["addcontact"] = DeviceWebsocketConstants::PathCode::ADDCONTACT;
  pathMap["deletecontact"] = DeviceWebsocketConstants::PathCode::DELETECONTACT;
  pathMap["updatecontact"] = DeviceWebsocketConstants::PathCode::UPDATECONTACT;
  pathMap["messaging"] = DeviceWebsocketConstants::PathCode::MESSAGING;
  pathMap["onmessagereceived"] = DeviceWebsocketConstants::PathCode::ONMESSAGERECEIVED;
  pathMap["sendmessage"] = DeviceWebsocketConstants::PathCode::SENDMESSAGE;
  pathMap["findmessage"] = DeviceWebsocketConstants::PathCode::FINDMESSAGE;
  pathMap["devicegallery"] = DeviceWebsocketConstants::PathCode::GALLERY;
  pathMap["findmedia"] = DeviceWebsocketConstants::PathCode::FINDMEDIA;
  pathMap["getmedia"] = DeviceWebsocketConstants::PathCode::GETMEDIA;
  pathMap["deletemedia"] = DeviceWebsocketConstants::PathCode::DELETEMEDIA;
  pathMap["sensor"] = DeviceWebsocketConstants::PathCode::SENSOR;
  pathMap["ondeviceproximity"] = DeviceWebsocketConstants::PathCode::ONDEVICEPROXIMITY;
  
  BrowserThread::PostTask(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &DeviceWebsocketHandler::initDeviceManager,
          weak_factory_.GetWeakPtr()));
}

void DeviceWebsocketHandler::initDeviceManager() {
#if defined(OS_ANDROID)
  service_manager::InterfaceProvider* interfaceProvider = GetGlobalJavaInterfaces();
  interfaceProvider->GetInterface(&deviceSoundManager);
  interfaceProvider->GetInterface(&deviceStorageManager);
  interfaceProvider->GetInterface(&deviceCpuManager);
  interfaceProvider->GetInterface(&calendarManager);
  interfaceProvider->GetInterface(&contactManager);
  interfaceProvider->GetInterface(&messagingManager);
  interfaceProvider->GetInterface(&deviceGalleryManager);
#elif defined(OS_LINUX) 
  if (ServiceManagerConnection::GetForProcess()) {
    service_manager::Connector* connector =
        ServiceManagerConnection::GetForProcess()->GetConnector();
    connector->BindInterface(device::mojom::kServiceName, mojo::MakeRequest(&deviceSoundManager));
    connector->BindInterface(device::mojom::kServiceName, mojo::MakeRequest(&deviceStorageManager));
    connector->BindInterface(device::mojom::kServiceName, mojo::MakeRequest(&deviceCpuManager));
  }
#endif
}

void DeviceWebsocketHandler::handleSocketMessage(int connection_id, const std::string& data) {
  if (!(thread_ && thread_->IsRunning()))
    return;
  int reader_error_code = 0;
  std::string reader_error_message;
  std::unique_ptr<DictionaryValue> json_value = DictionaryValue::From(base::JSONReader::ReadAndReturnError(
                                        data, base::JSON_ALLOW_TRAILING_COMMAS || base::JSON_PARSE_RFC || base::JSON_REPLACE_INVALID_CHARACTERS, 
                                        &reader_error_code, &reader_error_message));
  std::unique_ptr<DictionaryValue> response_value(new DictionaryValue());
  if (reader_error_code == base::JSONReader::JsonParseError::JSON_NO_ERROR && json_value && json_value->HasKey("action")) {
    std::string action;
    json_value->GetString("action", &action);
    std::string path;
    std::string requestId;
    if (json_value->HasKey("path")) {
      json_value->GetString("path", &path);
    }
    if (json_value->HasKey("requestId")) {
      json_value->GetString("requestId", &requestId);
    }
    setValue(response_value.get(), "action", &action);

    base::DictionaryValue* extra_value = nullptr;
    switch(functionMap[action]) {
      case DeviceWebsocketConstants::FunctionCode::AUTHORIZE: {
        setValue(response_value.get(), "requestId", &requestId);
        handleAuthorize(response_value.get(), connection_id, requestId);
        break;
      }
      case DeviceWebsocketConstants::FunctionCode::UNSUBSCRIBE:
      case DeviceWebsocketConstants::FunctionCode::UNSUBSCRIBEALL: {
        if (!checkAuthorize(connection_id, requestId)) {
          setErrorMessage(response_value.get(), DeviceWebsocketConstants::InternalErrorCode::UNAUTHORIZED);
          break;
        }
        uint64_t subscriptionId = 0;
        if (json_value->HasKey("subscriptionId")) {
          base::Value* subscriptionValue;
          json_value->Get("subscriptionId", &subscriptionValue);
          if (subscriptionValue) {
            switch(subscriptionValue->GetType()) {
              case base::Value::Type::STRING: {
                std::string tempString;
                subscriptionValue->GetAsString(&tempString);
                base::StringToUint64(tempString, &subscriptionId);
                break;
              }
              case base::Value::Type::INTEGER: {
                int32_t tempInt = 0;
                subscriptionValue->GetAsInteger(&tempInt);
                subscriptionId = tempInt;
                break;
              }
              case base::Value::Type::DOUBLE: {
                double tempDouble;
                subscriptionValue->GetAsDouble(&tempDouble);
                subscriptionId = (uint64_t)tempDouble;
                break;
              }
              default: {
                subscriptionId = 0;
                break;
              }
            }
          }
        }
        setValue(response_value.get(), "requestId", &requestId);
        handleUnsubscribe(response_value.get(), connection_id, action, subscriptionId);
        break;
      }
      default: {
        std::vector<std::string> path_vector;
        if (!checkAuthorize(connection_id, requestId)) {
          setErrorMessage(response_value.get(), DeviceWebsocketConstants::InternalErrorCode::UNAUTHORIZED);
          break;
        }

        if (path.empty() || !splitPath(&path_vector, path)) {
          setErrorMessage(response_value.get(), DeviceWebsocketConstants::InternalErrorCode::INVALID_PATH);
          break;
        }
        setValue(response_value.get(), "path", &path);
        switch(functionMap[action]) {
          case DeviceWebsocketConstants::FunctionCode::GET: {
            if (json_value->HasKey("extra")) {
              json_value->GetDictionary("extra", &extra_value);
            }
            handleGetFunction(response_value.release(), connection_id, path_vector, extra_value);
            return;
          }
          case DeviceWebsocketConstants::FunctionCode::SET: {
            if (json_value->HasKey("extra")) {
              json_value->GetDictionary("extra", &extra_value);
            }
            handleSetFunction(response_value.release(), connection_id, path_vector, extra_value);
            return;
          }
          case DeviceWebsocketConstants::FunctionCode::SUBSCRIBE: {
            if (json_value->HasKey("filters")) {
              json_value->GetDictionary("filters", &extra_value);
            }
            handleSubscribeFunction(response_value.release(), connection_id, path_vector, extra_value);
            return;
          }
          default: {
            LOG(ERROR) << "action is not available " << action;
            setErrorMessage(response_value.get(), DeviceWebsocketConstants::InternalErrorCode::INVALID_ACTION);
            break;
          }
        }
      }
      break;
    }
  } else {
    setValue(response_value.get(), "path", nullptr);
    setValue(response_value.get(), "action", nullptr);
    setErrorMessage(response_value.get(), DeviceWebsocketConstants::InternalErrorCode::BAD_REQUEST);
  }
  sendOverWebsocket(response_value.release(), connection_id);
}

void DeviceWebsocketHandler::setErrorMessage(DictionaryValue* value, DeviceWebsocketConstants::InternalErrorCode errorCode) {
  std::string error_number, error_reason, error_message;
  switch(errorCode) {
    case DeviceWebsocketConstants::InternalErrorCode::INVALID_PATH: {
      error_number = "404";
      error_reason = "invalid_path";
      error_message = "The specified data path does not exist";
      break;
    }
    case DeviceWebsocketConstants::InternalErrorCode::INVALID_ACTION: {
      error_number = "404";
      error_reason = "invalid_action";
      error_message = "The specified action does not exist";
      break;
    }
    case DeviceWebsocketConstants::InternalErrorCode::BAD_REQUEST: {
      error_number = "400";
      error_reason = "bad_request";
      error_message = "invalid request message";
      break;
    }
    case DeviceWebsocketConstants::InternalErrorCode::INVALID_REGISTER_CLIENT: {
      error_number = "401";
      error_reason = "invalid_requestId_or_client";
      error_message = "already registered or invalid RequsetId";
      break;
    }
    case DeviceWebsocketConstants::InternalErrorCode::UNAUTHORIZED: {
      error_number = "401";
      error_reason = "unregistered_client";
      error_message = "Invalid RequestId or unregistered client";
      break;
    }
    case DeviceWebsocketConstants::InternalErrorCode::INVALID_SUBSCRIPTIONID: {
      error_number = "404";
      error_reason = "invalid_subscriptionId";
      error_message = "The specified subscription was not found";
      break;
    }
  }
  setErrorMessage(value, error_number, error_reason, error_message);
}

void DeviceWebsocketHandler::setErrorMessage(DictionaryValue* value, 
                                            std::string number, 
                                            std::string reason, 
                                            std::string message) {
  setValue(value, "error.number", &number);
  setValue(value, "error.reason", &reason);
  setValue(value, "error.message", &message);
}

void DeviceWebsocketHandler::setValue(base::Value* value, std::string path, std::string* data) {
  if (value && value->GetType() == base::Value::Type::DICTIONARY && !path.empty()) {
    if (data && !data->empty()) {
      ((DictionaryValue*)value)->Set(path, base::MakeUnique<base::Value>(*data));
    } else {
      ((DictionaryValue*)value)->Set(path, base::MakeUnique<base::Value>());
    }
  }
}

void DeviceWebsocketHandler::OnFunctionCallback(DictionaryValue* value, int connection_id, DeviceFunctionManager* manager) {
  if (managerSet.find(manager) != managerSet.end()) {
    sendOverWebsocket(value, connection_id);
    managerSet.erase(manager);
    delete manager;
  }
}

void DeviceWebsocketHandler::OnCpuCallback(int32_t resultCode, double load) {
  if (!deviceCpuThreadSet.empty()) {
    for(std::set<uint64_t>::iterator it = deviceCpuThreadSet.begin();
        it != deviceCpuThreadSet.end(); ++it) {
      DeviceSubscribeCpuThread* thread = (DeviceSubscribeCpuThread*)*it;
      thread->notifyFromHandler(resultCode, load);
    }
  }
  
  if (deviceCpuThreadSet.empty()) {
    cpuFunctionManager.reset();
    if (deviceCpuManager.is_bound()) {
      deviceCpuManager->stopCpuLoad();
    }
  } else if (!deviceCpuThreadSet.empty() && cpuFunctionManager.get() && resultCode == 0) {
    cpuFunctionManager->cpuLoad(deviceCpuManager.get(), base::Bind(&DeviceWebsocketHandler::OnCpuCallback, weak_factory_.GetWeakPtr()));
  }
}

void DeviceWebsocketHandler::OnMessageCallback(device::mojom::MessageObjectPtr message) {
  
  if (!messageThreadSet.empty()) {
    for(std::set<uint64_t>::iterator it = messageThreadSet.begin();
        it != messageThreadSet.end(); ++it) {
      DeviceSubscribeMessageThread* thread = (DeviceSubscribeMessageThread*)*it;
      thread->notifyFromHandler(std::move(message));
    }
  }

  if (messageThreadSet.empty()) {
    messageFunctionManager->removeOnMessageReceived(messagingManager.get());
    messageFunctionManager.reset();
  } else if (!messageThreadSet.empty() && messageFunctionManager.get()) {
    messageFunctionManager->setOnMessageReceived(messagingManager.get(), base::Bind(&DeviceWebsocketHandler::OnMessageCallback, weak_factory_.GetWeakPtr()));
  }
}

void DeviceWebsocketHandler::observerNotiy(DictionaryValue* value, int connection_id) {
  sendOverWebsocket(value, connection_id);
}

void DeviceWebsocketHandler::sendOverWebsocket(DictionaryValue* value, int connection_id) {
  if (!thread_ || connection_id < 0) {
    return;
  }
  std::unique_ptr<DictionaryValue> value_ptr(value);
  std::string response_data;
  if (value_ptr.get()) {
    base::JSONWriter::WriteWithOptions(*value_ptr.get(), base::JSONWriter::Options::OPTIONS_OMIT_BINARY_VALUES, &response_data);
  }
  thread_->task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&DeviceWebServerWrapper::SendOverWebSocket,
                 base::Unretained(server_wrapper_), connection_id, std::string(response_data)));
}

bool DeviceWebsocketHandler::splitPath(std::vector<std::string>* path_vector, std::string path) {
  if (path_vector && !path.empty()) {
    size_t prevPos = 0;
    size_t pos = path.find(".", prevPos);
    while(pos != std::string::npos) {
      path_vector->push_back(path.substr(prevPos, pos - prevPos));
      prevPos = pos + 1;
      pos = path.find(".", prevPos);
    }
    if (prevPos < path.length()) {
      path_vector->push_back(path.substr(prevPos));
    } else {
      return false;
    }
    for(std::string data : *path_vector) {
    }
    return true;
  }
  return false;
}

bool DeviceWebsocketHandler::checkAuthorize(int connection_id, std::string& requestId) {
  if (!requestId.empty() && requestId_map_.find(requestId) != requestId_map_.end() &&
      connectionId_map_.find(connection_id) != connectionId_map_.end()) {
    int registed_conId = requestId_map_[requestId];
    std::string registed_reqId = connectionId_map_[connection_id];
    if (registed_conId == connection_id && registed_reqId == requestId) {
      return true;
    }
  }
  return false;
}

bool DeviceWebsocketHandler::removeObserver(uint64_t subscriptionId) {
  bool enableDelete = false;
  if (deviceCpuThreadSet.find(subscriptionId) != deviceCpuThreadSet.end()) {
    enableDelete = true;
    deviceCpuThreadSet.erase(subscriptionId);
  } else if(messageThreadSet.find(subscriptionId) != messageThreadSet.end()) {
    enableDelete = true;
    messageThreadSet.erase(subscriptionId);
    if (messageThreadSet.empty() && messageFunctionManager.get()) {
      messageFunctionManager->removeOnMessageReceived(messagingManager.get());
      messageFunctionManager.reset();
    }
  } else if(proximityThreadSet.find(subscriptionId) != proximityThreadSet.end()) {
    enableDelete = true;
    proximityThreadSet.erase(subscriptionId);
  }
  if (enableDelete) {
    DeviceSubscribeThread* thread = (DeviceSubscribeThread*)subscriptionId;
    if (thread) {
      delete thread;
    }
    return true;
  } else {
    return false;
  }
}

void DeviceWebsocketHandler::handleAuthorize(DictionaryValue* data, int connection_id, std::string requestId) {
  if (!requestId.empty() && requestId_map_.find(requestId) == requestId_map_.end()) {
    requestId_map_[requestId] = connection_id;
    connectionId_map_[connection_id] = requestId;
  } else {
    setErrorMessage(data, DeviceWebsocketConstants::InternalErrorCode::INVALID_REGISTER_CLIENT);
  }
}

void DeviceWebsocketHandler::handleUnsubscribe(DictionaryValue* data,
                                              int connection_id,
                                              std::string action,
                                              uint64_t subscriptionId) {
  if (functionMap[action] == DeviceWebsocketConstants::FunctionCode::UNSUBSCRIBEALL) {
    std::multimap<int, uint64_t>::iterator it = subscribeId_map_.find(connection_id);
    while(it != subscribeId_map_.end()) {
      if (removeObserver(it->second)) {
        subscribeId_map_.erase(it);
      }
      it = subscribeId_map_.find(connection_id);
    }
  } else if(functionMap[action] == DeviceWebsocketConstants::FunctionCode::UNSUBSCRIBE) {
    for(std::multimap<int, uint64_t>::iterator it = subscribeId_map_.begin();
        it != subscribeId_map_.end(); ++it) {
      if (it->second == subscriptionId && removeObserver(it->second)) {
        subscribeId_map_.erase(it);
        std::string subscriptionIdString = base::Uint64ToString(subscriptionId);
        DeviceWebsocketHandler::setValue(data, "subscriptionId", &subscriptionIdString);
        break;
      }
    }
    if (!data->HasKey("subscriptionId")) {
      LOG(ERROR) << "invalid subscriptionId";
      setErrorMessage(data, DeviceWebsocketConstants::InternalErrorCode::INVALID_SUBSCRIPTIONID);
    }
    std::string subscriptionIdString = base::Uint64ToString(subscriptionId);
    DeviceWebsocketHandler::setValue(data, "subscriptionId", &subscriptionIdString);
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(data, "timestamp", &timestamp);
}

void DeviceWebsocketHandler::handleGetFunction(DictionaryValue* data, 
                                              int connection_id, 
                                              std::vector<std::string>& path_vector, 
                                              DictionaryValue* extra) {
  if (path_vector.size() > 2 && path_vector[0] == "highweb") {
    switch(pathMap[path_vector[1]]) {
      case DeviceWebsocketConstants::PathCode::APPLAUNCHER: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::GETAPPLIST: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->applauncherGetAppList(data, connection_id, applauncher_handler_, profile_, 
                                          base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
          case DeviceWebsocketConstants::PathCode::GETAPPLICATIONINFO: {
            std::string appId;
            if (extra && extra->HasKey("appId")) {
              extra->GetString("appId", &appId);
            }
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->applauncherGetAppplicationInfo(data, connection_id, applauncher_handler_, profile_, appId,
                                          base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::SYSTEMINFO: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::DEVICESOUND: {
            if (path_vector.size() > 3 && path_vector[3] == "outputdevicetype") {
              DeviceFunctionManager* manager = new DeviceFunctionManager();
              managerSet.insert(manager);
              manager->soundGetOutputDeviceType(data, connection_id, deviceSoundManager.get(),
                                                base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
              return;
            } else if(path_vector.size() > 3 && path_vector[3] == "devicevolume") {
              DeviceFunctionManager* manager = new DeviceFunctionManager();
              managerSet.insert(manager);
              manager->soundGetDeviceVolume(data, connection_id, deviceSoundManager.get(),
                                                base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
              return;
            }
            break;
          }
          case DeviceWebsocketConstants::PathCode::DEVICESTORAGE: {
            if (path_vector.size() > 3 && path_vector[3] == "getdevicestorage") {
              DeviceFunctionManager* manager = new DeviceFunctionManager();
              managerSet.insert(manager);
              manager->getDeviceStorage(data, connection_id, deviceStorageManager.get(),
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
              return;
            }
            break;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::CALENDAR: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::FINDEVENT: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->calendarFindEvent(data, connection_id, calendarManager.get(), extra,
                                      base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::CONTACT: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::FINDCONTACT: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->contactFindContact(data, connection_id, contactManager.get(), extra,
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::MESSAGING: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::FINDMESSAGE: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->findMessage(data, connection_id, messagingManager.get(), extra,
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::GALLERY: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::FINDMEDIA: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->galleryFindMedia(data, connection_id, deviceGalleryManager.get(), extra,
                                      base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
          case DeviceWebsocketConstants::PathCode::GETMEDIA: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->galleryGetMedia(data, connection_id, deviceGalleryManager.get(), extra,
                                    base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
    }
  }
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(data, "timestamp", &timestamp);
  setErrorMessage(data, DeviceWebsocketConstants::InternalErrorCode::INVALID_PATH);
  sendOverWebsocket(data, connection_id);
}

void DeviceWebsocketHandler::handleSetFunction(DictionaryValue* data, 
                                              int connection_id, 
                                              std::vector<std::string>& path_vector, 
                                              DictionaryValue* extra) {
  if (path_vector.size() > 2 && path_vector[0] == "highweb") {
    switch(pathMap[path_vector[1]]) {
      case DeviceWebsocketConstants::PathCode::APPLAUNCHER: {
        std::string appId;
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::LAUNCHAPP: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->applauncherLaunchApp(data, connection_id, applauncher_handler_, profile_, extra,
                                          base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
          case DeviceWebsocketConstants::PathCode::REMOVEAPP: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->applauncherRemoveApp(data, connection_id, applauncher_handler_, profile_, extra,
                                          base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
          default: {
            break;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::CALENDAR: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::ADDEVENT: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->calendarAddEvent(data, connection_id, calendarManager.get(), extra,
                                      base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
          case DeviceWebsocketConstants::PathCode::UPDATEEVENT: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->calendarUpdateEvent(data, connection_id, calendarManager.get(), extra,
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
          case DeviceWebsocketConstants::PathCode::DELETEEVENT: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->calendarDeleteEvent(data, connection_id, calendarManager.get(), extra,
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::CONTACT: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::ADDCONTACT: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->contactAddContact(data, connection_id, contactManager.get(), extra,
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
          case DeviceWebsocketConstants::PathCode::UPDATECONTACT: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->contactUpdateContact(data, connection_id, contactManager.get(), extra,
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
          case DeviceWebsocketConstants::PathCode::DELETECONTACT: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->contactDeleteContact(data, connection_id, contactManager.get(), extra,
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::MESSAGING: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::SENDMESSAGE: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->sendMessage(data, connection_id, messagingManager.get(), extra,
                                base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::GALLERY: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::DELETEMEDIA: {
            DeviceFunctionManager* manager = new DeviceFunctionManager();
            managerSet.insert(manager);
            manager->galleryDeleteMedia(data, connection_id, deviceGalleryManager.get(), extra,
                                        base::Bind(&DeviceWebsocketHandler::OnFunctionCallback, weak_factory_.GetWeakPtr()));
            return;
          }
        }
        break;
      }
    }
  } 
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(data, "timestamp", &timestamp);
  setErrorMessage(data, DeviceWebsocketConstants::InternalErrorCode::INVALID_PATH);
  sendOverWebsocket(data, connection_id);
}


void DeviceWebsocketHandler::handleSubscribeFunction(DictionaryValue* data, 
                                                    int connection_id, 
                                                    std::vector<std::string>& path_vector, 
                                                    DictionaryValue* filters) {
  DeviceWebsocketHandler::setValue(data, "requestId", &connectionId_map_[connection_id]);
  std::string timestamp = base::Uint64ToString((uint64_t)base::Time::Now().ToJsTime());
  DeviceWebsocketHandler::setValue(data, "timestamp", &timestamp);
  if (path_vector.size() > 2 && path_vector[0] == "highweb") {
    switch(pathMap[path_vector[1]]) {
      case DeviceWebsocketConstants::PathCode::SYSTEMINFO: {
        switch(pathMap[path_vector[2]]) {
          case DeviceWebsocketConstants::PathCode::DEVICECPU: {
            if (path_vector.size() > 3 && pathMap[path_vector[3]] == DeviceWebsocketConstants::PathCode::LOAD) {
              if (deviceCpuManager.get() && deviceCpuManager.is_bound()) {
                deviceCpuManager->startCpuLoad();
                std::string path;
                if (data->HasKey("path")) {
                  data->GetString("path", &path);
                  data->Remove("path", nullptr);
                }
                DeviceSubscribeCpuThread* cpuThread = new DeviceSubscribeCpuThread(path, connection_id, 
                    base::Bind(&DeviceWebsocketHandler::observerNotiy, weak_factory_.GetWeakPtr()));
                subscribeId_map_.insert(std::make_pair(connection_id, (uint64_t)cpuThread));
                deviceCpuThreadSet.insert((uint64_t)cpuThread);
                
                data->SetString("subscriptionId", base::Uint64ToString((uint64_t)cpuThread));
                
                if (!cpuFunctionManager.get()) {
                  cpuFunctionManager.reset(new DeviceFunctionManager());
                  cpuFunctionManager->cpuLoad(deviceCpuManager.get(), 
                      base::Bind(&DeviceWebsocketHandler::OnCpuCallback, weak_factory_.GetWeakPtr()));
                }

                cpuThread->setFilter(filters);
                cpuThread->StartThread();
              } else {
                std::string errorMessage = "DeviceApiManager is null";
                DeviceWebsocketHandler::setErrorMessage(data, "500", "devicecpu_internal_error", errorMessage);
              }
              sendOverWebsocket(data, connection_id);
              return;
            }
            break;
          }
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::MESSAGING: {
        if (pathMap[path_vector[2]] == DeviceWebsocketConstants::PathCode::ONMESSAGERECEIVED) {
          if (messagingManager.get() && messagingManager.is_bound()) {
            std::string path;
            if (data->HasKey("path")) {
              data->GetString("path", &path);
              data->Remove("path", nullptr);
            }
            DeviceSubscribeMessageThread* messageThread = new DeviceSubscribeMessageThread(path, connection_id, 
                base::Bind(&DeviceWebsocketHandler::observerNotiy, weak_factory_.GetWeakPtr()));
            subscribeId_map_.insert(std::make_pair(connection_id, (uint64_t)messageThread));
            messageThreadSet.insert((uint64_t)messageThread);
            
            data->SetString("subscriptionId", base::Uint64ToString((uint64_t)messageThread));
            
            if (!messageFunctionManager.get()) {
              messageFunctionManager.reset(new DeviceFunctionManager());
              messageFunctionManager->setOnMessageReceived(messagingManager.get(), 
                  base::Bind(&DeviceWebsocketHandler::OnMessageCallback, weak_factory_.GetWeakPtr()));
            }

            messageThread->setFilter(filters);
            messageThread->StartThread();
          } else {
            std::string errorMessage = "DeviceApiManager is null";
            DeviceWebsocketHandler::setErrorMessage(data, "500", "messaging_internal_error", errorMessage);
          }
          sendOverWebsocket(data, connection_id);
          return;
        }
        break;
      }
      case DeviceWebsocketConstants::PathCode::SENSOR: {
        if (pathMap[path_vector[2]] == DeviceWebsocketConstants::PathCode::ONDEVICEPROXIMITY) {
          DeviceSubscribeProximitySensorThread* proximityThread = nullptr;
          #if defined(OS_ANDROID)
            service_manager::InterfaceProvider* interfaceProvider = GetGlobalJavaInterfaces();
            device::mojom::ProximitySensorPtr proximitySensor;
            interfaceProvider->GetInterface(&proximitySensor);
            if (proximitySensor.get() && proximitySensor.is_bound()) {
              device::mojom::ProximitySensorRequest request = mojo::MakeRequest(&proximitySensor);
              device::DeviceProximityHost::Create(std::move(request));
            }
            std::string path;
            if (data->HasKey("path")) {
              data->GetString("path", &path);
              data->Remove("path", nullptr);
            }
            proximityThread = new DeviceSubscribeProximitySensorThread(path, connection_id, 
                base::Bind(&DeviceWebsocketHandler::observerNotiy, weak_factory_.GetWeakPtr()));
            subscribeId_map_.insert(std::make_pair(connection_id, (uint64_t)proximityThread));
            proximityThreadSet.insert((uint64_t)proximityThread);
            
            data->SetString("subscriptionId", base::Uint64ToString((uint64_t)proximityThread));

            proximityThread->setFilter(filters);
            proximityThread->StartThread(std::move(proximitySensor));

          #endif
          if (!proximityThread) {
            std::string errorMessage = "DeviceApiManager is null";
            DeviceWebsocketHandler::setErrorMessage(data, "500", "devicesensor_internal_error", errorMessage);
          }
          sendOverWebsocket(data, connection_id);
          return;
        }
        break;
      }
    }
  }
  if (data->HasKey("action")) {
    data->Remove("action", nullptr);
  }
  setErrorMessage(data, DeviceWebsocketConstants::InternalErrorCode::INVALID_PATH);
  sendOverWebsocket(data, connection_id);
}

void DeviceWebsocketHandler::AcceptWebSocket(
    int connection_id,
    const net::HttpServerRequestInfo& request) {
  if (!(thread_ && thread_->IsRunning()))
    return;
  thread_->task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&DeviceWebServerWrapper::AcceptWebSocket,
                 base::Unretained(server_wrapper_), connection_id, request));
}

void DeviceWebsocketHandler::SetDeviceAPIHandler(ApplauncherApiHandler* handler) {
  applauncher_handler_ = handler;
}

}  // namespace content
