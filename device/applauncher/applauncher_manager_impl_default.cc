// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/applauncher/applauncher_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include <stdint.h>
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

namespace {

class AppLauncherManagerEmptyImpl : public AppLauncherManager {
 public:
  void LaunchApp(const mojo::String& packageName, const mojo::String& activityName, const LaunchAppCallback& callback) override {}
  void RemoveApp(const mojo::String& packageName, const RemoveAppCallback& callback) override {}
  void GetAppList(const mojo::String& mimeType, const GetAppListCallback& callback) override {}
  void GetApplicationInfo(const mojo::String& packageName, const int32_t flags, const GetApplicationInfoCallback& callback) override {}

 private:
  friend AppLauncherManagerImpl;

  explicit AppLauncherManagerEmptyImpl(
      mojo::InterfaceRequest<AppLauncherManager> request)
      : binding_(this, std::move(request)) {}
  ~AppLauncherManagerEmptyImpl() override {}

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<AppLauncherManager> binding_;
};

}  // namespace

// static
void AppLauncherManagerImpl::Create(
    mojo::InterfaceRequest<AppLauncherManager> request) {
  new AppLauncherManagerEmptyImpl(std::move(request));
}

}  // namespace device
