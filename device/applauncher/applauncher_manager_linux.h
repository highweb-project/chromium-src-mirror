// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/applauncher/applauncher_manager_impl.h"

#include "mojo/public/cpp/bindings/strong_binding.h"
#include "base/values.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "base/bind.h"

namespace device {
class AppLauncherExecuter;

class AppLauncherManagerLinux : public AppLauncherManager {
 public:
  void LaunchApp(const mojo::String& packageName, const mojo::String& activityName, const LaunchAppCallback& callback) override;
  void RemoveApp(const mojo::String& packageName, const RemoveAppCallback& callback) override;
  void GetAppList(const mojo::String& mimeType, const GetAppListCallback& callback) override;
  void GetApplicationInfo(const mojo::String& packageName, const int32_t flags, const GetApplicationInfoCallback& callback) override;

 private:
  friend AppLauncherManagerImpl;

  explicit AppLauncherManagerLinux(
      mojo::InterfaceRequest<AppLauncherManager> request);
  ~AppLauncherManagerLinux() override;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<AppLauncherManager> binding_;

  std::vector<std::string>* getFileList(std::string path);
  AppLauncher_ApplicationInfoPtr getAppInfo(std::string filePath, std::string* findExecString=nullptr);
  base::DictionaryValue* parsingData(std::string fileInfo);
  std::string exec(const char* cmd);

  AppLauncherExecuter* executer = nullptr;
};

// static
void AppLauncherManagerImpl::Create(
    mojo::InterfaceRequest<AppLauncherManager> request) {
  new AppLauncherManagerLinux(std::move(request));
}

//thread executer
class AppLauncherExecuter : public base::Thread {
  public:
    static std::string threadName() {
      return std::string("AppLauncherExecuter");
    }
    AppLauncherExecuter();
    ~AppLauncherExecuter() override;

    void execute(std::string processName);
  private:
    void executeInternal(std::string processName);
    std::vector<pid_t> pidList;
    DISALLOW_COPY_AND_ASSIGN(AppLauncherExecuter);
};

}  // namespace device
