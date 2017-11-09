// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "modules/applauncher/AppLauncher.h"

#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/WebString.h"
#include "AppStatus.h"
#include "AppLauncherScriptCallback.h"
#include "ApplicationInfo.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"
#include "modules/applauncher/DeviceApiApplauncherController.h"
#include "public/platform/modules/device_api/WebDeviceApiApplauncherClient.h"

#include "content/public/common/device_api_applauncher_request.h"

namespace blink {

AppLauncher::AppLauncher(LocalFrame& frame)
  : mClient(DeviceApiPermissionController::From(frame)->client())
{
  mOrigin = frame.GetDocument()->Url().StrippedForUseAsReferrer();
  mClient->SetOrigin(mOrigin.Latin1().data());
  mApplauncherClient = DeviceApiApplauncherController::From(frame)->client();
  d_functionData.clear();
}

AppLauncher::~AppLauncher()
{
  mCallbackList.clear();
  d_functionData.clear();
}

void AppLauncher::launchApp(const String appId, AppLauncherScriptCallback* callback) {
  functionData* data = new functionData(function::FUNC_LAUNCH_APP);
  //20160419-jphong
  //data->scriptCallback = callback;
  mCallbackList.push_back(callback);
  data->setString(data->str1, appId);
  d_functionData.push_back(data);
  data = nullptr;

  if (appId.IsEmpty()) {
    notifyError(code_list::kInvalidAppId, callback);
    return;
  }
  if (d_functionData.size() == 1) {
    launchApp();
  }
}

void AppLauncher::launchApp() {
  if (!ViewPermissionState) {
    requestPermission(PermissionOptType::VIEW);
    return;
  }
  functionData* data = d_functionData.front();

  WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequest* request = 
    new WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequest(
      base::Bind(&AppLauncher::onRequestResult, base::Unretained(this))
  );
  request->functionCode = data->functionCode;
  request->appId = std::string(data->str1.Utf8().data());
  mApplauncherClient->requestFunction(request);
  data = nullptr;
}

void AppLauncher::removeApp(const String appId, AppLauncherScriptCallback* callback) {
  functionData* data = new functionData(function::FUNC_REMOVE_APP);
  //20160419-jphong
  //data->scriptCallback = callback;
  mCallbackList.push_back(callback);
  data->setString(data->str1, appId);
  d_functionData.push_back(data);
  data = nullptr;

  if (appId.IsEmpty()) {
    notifyError(code_list::kInvalidAppId, callback);
    return;
  }
  if (d_functionData.size() == 1) {
    removeApp();
  }
}

void AppLauncher::removeApp() {
  if (!DeletePermissionState) {
    requestPermission(PermissionOptType::DELETE);
    return;
  }
  functionData* data = d_functionData.front();

  WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequest* request = 
    new WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequest(
      base::Bind(&AppLauncher::onRequestResult, base::Unretained(this))
  );
  request->functionCode = data->functionCode;
  request->appId = std::string(data->str1.Utf8().data());
  mApplauncherClient->requestFunction(request);

  data = nullptr;
}

void AppLauncher::getAppList(AppLauncherScriptCallback* callback) {
  functionData* data = new functionData(function::FUNC_GET_APP_LIST);
  //20160419-jphong
  //data->scriptCallback = callback;
  mCallbackList.push_back(callback);
  d_functionData.push_back(data);
  data = nullptr;
  if (d_functionData.size() == 1) {
    getAppList();
  }
}

void AppLauncher::getAppList() {
  if (!ViewPermissionState) {
    requestPermission(PermissionOptType::VIEW);
    return;
  }
  functionData* data = d_functionData.front();

  WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequest* request = 
    new WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequest(
      base::Bind(&AppLauncher::onRequestResult, base::Unretained(this))
  );
  request->functionCode = data->functionCode;
  mApplauncherClient->requestFunction(request);
  data = nullptr;
}

void AppLauncher::getApplicationInfo(const String appId, AppLauncherScriptCallback* callback) {
  functionData* data = new functionData(function::FUNC_GET_APPLICATION_INFO);
  //20160419-jphong
  //data->scriptCallback = callback;
  mCallbackList.push_back(callback);
  data->setString(data->str1, appId);
  d_functionData.push_back(data);
  data = nullptr;
  if (appId.IsEmpty()) {
    notifyError(code_list::kInvalidAppId, callback);
    return;
  }
  if (d_functionData.size() == 1) {
    getApplicationInfo();
  }
}

void AppLauncher::getApplicationInfo() {
  if (!ViewPermissionState) {
    requestPermission(PermissionOptType::VIEW);
    return;
  }
  functionData* data = d_functionData.front();

  WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequest* request = 
    new WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequest(
      base::Bind(&AppLauncher::onRequestResult, base::Unretained(this))
  );
  request->functionCode = data->functionCode;
  request->appId = std::string(data->str1.Utf8().data());
  mApplauncherClient->requestFunction(request);

  data = nullptr;
}

void AppLauncher::notifyCallback(AppStatus* status, AppLauncherScriptCallback* callback) {
  if (callback != NULL) {
    callback->handleEvent(status);
    callback = nullptr;
  }
  if (d_functionData.size() > 0)
    d_functionData.pop_front();
  //20160419-jphong
  if(mCallbackList.size() > 0)
    mCallbackList.erase(0);
  continueFunction();
}

void AppLauncher::notifyError(int errorCode, AppLauncherScriptCallback* callback) {
  AppStatus* status = AppStatus::create();
  status->setResultCode(errorCode);
  notifyCallback(status, callback);
}

void AppLauncher::continueFunction() {
  if (d_functionData.size() > 0) {
    switch(d_functionData.front()->functionCode) {
      case function::FUNC_LAUNCH_APP: {
        launchApp();
        break;
      }
      case function::FUNC_REMOVE_APP: {
        removeApp();
        break;
      }
      case function::FUNC_GET_APP_LIST: {
        getAppList();
        break;
      }
      case function::FUNC_GET_APPLICATION_INFO: {
        getApplicationInfo();
        break;
      }
      default: {
        break;
      }
    }
  }
}

void AppLauncher::requestPermission(int operationType) {
  if(mClient) {
    switch(operationType) {
      case PermissionOptType::VIEW: {
        mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
            PermissionAPIType::APPLICATION_LAUNCHER,
            PermissionOptType::VIEW,
            base::Bind(&AppLauncher::onPermissionChecked, base::Unretained(this))));
        break;
      }
      case PermissionOptType::DELETE: {
        mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
            PermissionAPIType::APPLICATION_LAUNCHER,
            PermissionOptType::DELETE,
            base::Bind(&AppLauncher::onPermissionChecked, base::Unretained(this))));
        break;
      }
    }
  }
}

void AppLauncher::onPermissionChecked(PermissionResult result)
{
  if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK && d_functionData.size() > 0) {
    switch(d_functionData.front()->functionCode) {
      case function::FUNC_LAUNCH_APP:
      case function::FUNC_GET_APP_LIST:
      case function::FUNC_GET_APPLICATION_INFO: {
        ViewPermissionState = true;
        if (ViewPermissionState) {
          continueFunction();
          return;
        }
        break;
      }
      case function::FUNC_REMOVE_APP: {
        DeletePermissionState = true;
        if (DeletePermissionState) {
          continueFunction();
          return;
        }
        break;
      }
    }
  }
  notifyError(code_list::kNotEnabledPermission, mCallbackList.at(0).Get());
}

void AppLauncher::onRequestResult(WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequestResult result) {
  DLOG(INFO) << "DeviceApiApplauncherReuqstResult : " << result.resultCode << ", " << result.functionCode;

  AppStatus* status = AppStatus::create();
  status->setResultCode(result.resultCode);
  switch(result.functionCode) {
    case function::FUNC_GET_APP_LIST:
    {
      if (result.resultCode != code_list::kSuccess){
        break;
      }
      if (result.applist.empty()) {
        break;
      }

      for(WebDeviceApiApplauncherClient::WebAppLauncher_ApplicationInfo info : result.applist) {
        ApplicationInfo* data = new ApplicationInfo();
        data->setName(String(info.name.data()));
        data->setId(String(info.id.data()));
        data->setUrl(String(info.url.data()));
        data->setVersion(String(info.version.data()));
        data->setIconUrl(String(info.iconUrl.data()));
        status->appList().push_back(data);
      }
      break;
    }
    case function::FUNC_GET_APPLICATION_INFO:
    {
      if (result.resultCode != code_list::kSuccess){
        break;
      }
      if (result.applist.empty()) {
        break;
      }

      if (result.applist.size() >= 1) {
        WebDeviceApiApplauncherClient::WebAppLauncher_ApplicationInfo info = result.applist[0];
        ApplicationInfo* data = new ApplicationInfo();
        data->setName(String(info.name.data()));
        data->setId(String(info.id.data()));
        data->setUrl(String(info.url.data()));
        data->setVersion(String(info.version.data()));
        data->setIconUrl(String(info.iconUrl.data()));
        status->setAppInfo(data);
      }
    }
    default: {
      break;
    }
  }
  notifyCallback(status, mCallbackList.at(0).Get());
}

DEFINE_TRACE(AppLauncher)
{
  visitor->Trace(mCallbackList);
}

} // namespace blink
