// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_DEVICE_API_ANDROID_APPLAUNCHER_API_HANDLER_H_
#define CHROME_BROWSER_ANDROID_DEVICE_API_ANDROID_APPLAUNCHER_API_HANDLER_H_

#include "base/memory/singleton.h"
#include "content/public/common/applauncher_api_handler.h"
#include "content/public/common/device_api_applauncher_request.h"
#include "chrome/browser/profiles/profile.h"

namespace base {
  class DictionaryValue;
}

namespace content {
class WebContents;
}

class ApplauncherApiHandlerImpl : content::ApplauncherApiHandler {
public:
  static ApplauncherApiHandlerImpl* GetInstance();

  void RequestFunction(Profile* profile, const content::DeviceApiApplauncherRequest& request) override;
  void getAppList(Profile* profile, const content::DeviceApiApplauncherRequest& request) override;
  void launchApp(Profile* profile, const content::DeviceApiApplauncherRequest& request) override;
  void removeApp(Profile* profile, const content::DeviceApiApplauncherRequest& request) override;
  void getApplicationInfo(Profile* profile, const content::DeviceApiApplauncherRequest& request) override;

  void throwErrorMessage(const content::DeviceApiApplauncherRequest& request, int32_t error_message) override;

private:
  bool getAppInfo(base::DictionaryValue* value, content::AppLauncher_ApplicationInfo* info);
  friend struct base::DefaultSingletonTraits<ApplauncherApiHandlerImpl>;

  ApplauncherApiHandlerImpl();
  ~ApplauncherApiHandlerImpl() override;
};



#endif /* CHROME_BROWSER_ANDROID_DEVICE_API_ANDROID_APPLAUNCHER_API_HANDLER_H_ */
