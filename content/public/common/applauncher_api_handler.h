// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_APPLAUNCHER_API_HANDLER_H_
#define CONTENT_PUBLIC_COMMON_APPLAUNCHER_API_HANDLER_H_

#include "chrome/browser/profiles/profile.h"

namespace content {
class WebContents;
struct DeviceApiApplauncherRequest;

class ApplauncherApiHandler {
public:
  virtual void RequestFunction(Profile* profile, const content::DeviceApiApplauncherRequest& request) = 0;
  virtual void getAppList(Profile* profile, const content::DeviceApiApplauncherRequest& request) = 0;
  virtual void launchApp(Profile* profile, const content::DeviceApiApplauncherRequest& request) = 0;
  virtual void removeApp(Profile* profile, const content::DeviceApiApplauncherRequest& request) = 0;
  virtual void getApplicationInfo(Profile* profile, const content::DeviceApiApplauncherRequest& request) = 0;

  virtual void throwErrorMessage(const content::DeviceApiApplauncherRequest& request, int32_t error_message) = 0;

  virtual ~ApplauncherApiHandler() {}
};

}

#endif /* CONTENT_PUBLIC_COMMON_APPLAUNCHER_API_HANDLER_H_ */
