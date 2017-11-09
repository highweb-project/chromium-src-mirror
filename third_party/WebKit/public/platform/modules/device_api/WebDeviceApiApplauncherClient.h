// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_DEVICE_API_APPLAUNCHER_CLIENT_H_
#define WEB_DEVICE_API_APPLAUNCHER_CLIENT_H_

#include <string>

#include "base/callback.h"
#include "base/callback_forward.h"

namespace blink {

class WebDeviceApiApplauncherClient {
public:

	enum WebApplauncherCodeList{
		// Exception code
		SUCCESS = 0,
		FAILURE = -1,
		NOT_INSTALLED_APP = -2,
		INVALID_APP_ID = -3,
		NOT_ENABLED_PERMISSION = -4,
		NOT_SUPPORT_API = 9999,
	};

	enum WebApplauncherFunction {
		FUNC_LAUNCH_APP = 1,
		FUNC_REMOVE_APP,
		FUNC_GET_APP_LIST,
		FUNC_GET_APPLICATION_INFO,
	};

	struct WebAppLauncher_ApplicationInfo {
		WebAppLauncher_ApplicationInfo() {}
		std::string id = "";
		std::string name = "";
		std::string version = "";
		std::string url = "";
		std::string iconUrl = "";
	};

	struct WebDeviceApiApplauncherRequestResult {
		WebDeviceApiApplauncherRequestResult() {}
		~WebDeviceApiApplauncherRequestResult() {
			applist.clear();
		}
		int32_t resultCode;
		int32_t functionCode;
		std::vector<WebAppLauncher_ApplicationInfo> applist;
	};

	struct WebDeviceApiApplauncherRequest {
		WebDeviceApiApplauncherRequest(base::Callback<void(WebDeviceApiApplauncherRequestResult result)> callback)
			:callback_(callback) {
		}
		int32_t functionCode;
		std::string appId;
		base::Callback<void(WebDeviceApiApplauncherRequestResult result)> callback_;
};
 
  virtual void requestFunction(WebDeviceApiApplauncherRequest* request) = 0;

    virtual ~WebDeviceApiApplauncherClient() { }
};

} // namespace blink

#endif // WEB_DEVICE_API_APPLAUNCHER_CLIENT_H_
