// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_DEVICE_API_APPLAUNCHER_REQUEST_H_
#define CONTENT_PUBLIC_COMMON_DEVICE_API_APPLAUNCHER_REQUEST_H_

#include "base/callback.h"
#include "content/common/content_export.h"
#include <vector>
#include <utility>

namespace content {

enum applauncher_code_list{
  // Exception code
  APPLAUNCHER_SUCCESS = 0,
  APPLAUNCHER_FAILURE = -1,
  APPLAUNCHER_NOT_INSTALLED_APP = -2,
  APPLAUNCHER_INVALID_APP_ID = -3,
  APPLAUNCHER_NOT_ENABLED_PERMISSION = -4,
  APPLAUNCHER_NOT_SUPPORT_API = 9999,
};

enum applauncher_function {
  APPLAUNCHER_FUNC_LAUNCH_APP = 1,
  APPLAUNCHER_FUNC_REMOVE_APP,
  APPLAUNCHER_FUNC_GET_APP_LIST,
  APPLAUNCHER_FUNC_GET_APPLICATION_INFO,
};

struct CONTENT_EXPORT AppLauncher_ApplicationInfo {
  AppLauncher_ApplicationInfo();
  AppLauncher_ApplicationInfo(const AppLauncher_ApplicationInfo& other);
  ~AppLauncher_ApplicationInfo();
  std::string id;
  std::string name;
  std::string version;
  std::string url;
  std::string iconUrl;
};

struct CONTENT_EXPORT DeviceApiApplauncherRequestResult {
  DeviceApiApplauncherRequestResult();
  DeviceApiApplauncherRequestResult(const DeviceApiApplauncherRequestResult& other);
  ~DeviceApiApplauncherRequestResult();
  int32_t resultCode;
  int32_t functionCode;
  std::vector<AppLauncher_ApplicationInfo> applist;
};

typedef base::Callback<void(content::DeviceApiApplauncherRequestResult result)> DeviceApiApplauncherCallback;

struct CONTENT_EXPORT DeviceApiApplauncherRequest {
  DeviceApiApplauncherRequest(const DeviceApiApplauncherRequest& other);
  DeviceApiApplauncherRequest(const DeviceApiApplauncherCallback& callback);
  ~DeviceApiApplauncherRequest();
  int32_t functionCode;
  std::string appId;
  DeviceApiApplauncherCallback callback_;
};

}

#endif // CONTENT_PUBLIC_COMMON_DEVICE_API_APPLAUNCHER_REQUEST_H_
