#include "device_api_applauncher_request.h"

namespace content {
  AppLauncher_ApplicationInfo::AppLauncher_ApplicationInfo(const AppLauncher_ApplicationInfo& other) = default;
  AppLauncher_ApplicationInfo::AppLauncher_ApplicationInfo(){
    id = "";
    name = "";
    version = "";
    url = "";
    iconUrl = "";
  }
  AppLauncher_ApplicationInfo::~AppLauncher_ApplicationInfo() {}

  DeviceApiApplauncherRequestResult::DeviceApiApplauncherRequestResult() {}
  DeviceApiApplauncherRequestResult::DeviceApiApplauncherRequestResult(const DeviceApiApplauncherRequestResult& other) = default;
  DeviceApiApplauncherRequestResult::~DeviceApiApplauncherRequestResult() {
    applist.clear();
  }
  
  DeviceApiApplauncherRequest::DeviceApiApplauncherRequest(const DeviceApiApplauncherRequest& other) = default;
  DeviceApiApplauncherRequest::DeviceApiApplauncherRequest(const DeviceApiApplauncherCallback& callback) {
    callback_ = callback;
  }
  DeviceApiApplauncherRequest::~DeviceApiApplauncherRequest() {
  }
}