// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/applauncher/applauncher_manager_linux.h"

#include <dirent.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/wait.h>

namespace device {

AppLauncherManagerLinux::AppLauncherManagerLinux(AppLauncherManagerRequest request)
  :binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {
}

AppLauncherManagerLinux::~AppLauncherManagerLinux() {
  if (executer) {
    delete executer;
  }
}

void AppLauncherManagerLinux::LaunchApp(
  const std::string& packageName, const std::string& activityName, const LaunchAppCallback& callback) {
  AppLauncher_ResultCodePtr result = AppLauncher_ResultCode::New();
  result->resultCode = int32_t(device::applauncher_code_list::NOT_SUPPORT_API);
  result->functionCode = int32_t(device::applauncher_function::FUNC_LAUNCH_APP);

  std::string command = "which " + packageName;
  std::string checkValidation = exec(command.data());
  if (!checkValidation.empty()) {
    if (!executer) {
      executer = new AppLauncherExecuter();
    }
    executer->execute(packageName);
    result->resultCode = int32_t(device::applauncher_code_list::SUCCESS);
  } else {
    result->resultCode = int32_t(device::applauncher_code_list::INVALID_PACKAGE_NAME);
  }
  callback.Run(result.Clone());
}

void AppLauncherManagerLinux::RemoveApp(
  const std::string& packageName, const RemoveAppCallback& callback) {
  AppLauncher_ResultCodePtr result = AppLauncher_ResultCode::New();
  result->resultCode = int32_t(device::applauncher_code_list::NOT_SUPPORT_API);
  result->functionCode = int32_t(device::applauncher_function::FUNC_REMOVE_APP);
  callback.Run(result.Clone());
}

void AppLauncherManagerLinux::GetAppList(const std::string& mimeType, const GetAppListCallback& callback) {
  AppLauncher_ResultCodePtr result = AppLauncher_ResultCode::New();
  result->resultCode = int32_t(device::applauncher_code_list::NOT_SUPPORT_API);
  result->functionCode = int32_t(device::applauncher_function::FUNC_GET_APP_LIST);
  std::vector<std::string>* fileList = getFileList("/usr/share/applications");
  if (fileList->size() > 0) {
    result->applist = std::vector<device::AppLauncher_ApplicationInfoPtr>();
    for(size_t listIndex = 0; listIndex < fileList->size(); listIndex++) {
      AppLauncher_ApplicationInfoPtr appInfo = getAppInfo(fileList->at(listIndex));
      if (!appInfo.is_null() && !appInfo->name.empty()) {
        result->applist->push_back(std::move(appInfo));
      }
    }
    fileList->clear();
    delete fileList;
    result->resultCode = int32_t(device::applauncher_code_list::SUCCESS);
  }
  callback.Run(result.Clone());
}

void AppLauncherManagerLinux::GetApplicationInfo(
  const std::string& packageName, const int32_t flags, const GetApplicationInfoCallback& callback) {
  AppLauncher_ResultCodePtr result = AppLauncher_ResultCode::New();
  result->resultCode = int32_t(device::applauncher_code_list::NOT_SUPPORT_API);
  result->functionCode = int32_t(device::applauncher_function::FUNC_GET_APPLICATION_INFO);

  std::vector<std::string>* fileList = getFileList("/usr/share/applications");
  std::string execName = packageName;
  if (fileList->size() > 0) {
    result->applist = std::vector<device::AppLauncher_ApplicationInfoPtr>();
    for(size_t listIndex = 0; listIndex < fileList->size(); listIndex++) {
      AppLauncher_ApplicationInfoPtr appInfo = getAppInfo(fileList->at(listIndex), &execName);
      if (!appInfo.is_null() && !appInfo->name.empty()) {
        result->applist->push_back(std::move(appInfo));
      }
    }
    fileList->clear();
    delete fileList;
    if (result->applist.has_value()) {
      result->resultCode = int32_t(device::applauncher_code_list::SUCCESS);
    } else {
      result->resultCode = int32_t(device::applauncher_code_list::NOT_INSTALLED_APP);
    }
  }
  callback.Run(result.Clone());
}

std::vector<std::string>* AppLauncherManagerLinux::getFileList(std::string path) {
  DIR *d;
  struct dirent *dir;
  std::vector<std::string>* fileList = new std::vector<std::string>();
  d = opendir(path.data());
  if (d) {
    while( (dir = readdir(d)) != NULL) {
      if (std::string(dir->d_name).find(".desktop") != std::string::npos) {
        fileList->push_back(dir->d_name);
      }
    }
    closedir(d);
  }
  return fileList;
}

AppLauncher_ApplicationInfoPtr AppLauncherManagerLinux::getAppInfo(std::string filePath, std::string* findExecString) {
  AppLauncher_ApplicationInfoPtr appInfo(AppLauncher_ApplicationInfo::New());
  std::ifstream inFile;
  inFile.open("/usr/share/applications/" + filePath, std::ios::in);
  if (inFile.fail()) {
    return nullptr;
  }
  std::string fileInfo = "";
  std::string read;
  bool write = false;
  while(!inFile.eof()) {
    std::getline(inFile, read);
    if (read.find("[Desktop Entry]") != std::string::npos) {
      write = true;
      continue;
    } else if(read.find("[Desktop") != std::string::npos) {
      write = false;
    }
    if (write) {
      fileInfo.append(read + "\n");
    }
  }
  inFile.close();

  base::DictionaryValue* parsingResult = parsingData(fileInfo);
  std::string value;
  if (!parsingResult->HasKey("Name") || !parsingResult->HasKey("Exec")) {
    DLOG(INFO) << filePath << " not has information";
    return nullptr;
  }
  parsingResult->GetString("Name", &value);
  appInfo->name = value;
  appInfo->processName = value;
  parsingResult->GetString("Exec", &value);
  if (value.find("%") != std::string::npos) {
    value = value.substr(0, value.find("%"));
  }

  while(value.size() > 0 && value[0] == ' ') {
    value.erase(0, 1);
  }
  while(value.size() > 0 && value[value.size() - 1] == ' ') {
    value.erase(value.size() - 1, 1);
  }

  appInfo->packageName = value;
  appInfo->enabled = true;
  if (parsingResult->HasKey("Terminal")) {
    parsingResult->GetString("Terminal", &value);
    if (value == "true") {
      appInfo->enabled = false;
    }
  }

  delete parsingResult;
  if (findExecString != nullptr) {
    if (appInfo->packageName != *findExecString) {
      return nullptr;
    }
  }
  return appInfo.Clone();
}

base::DictionaryValue* AppLauncherManagerLinux::parsingData(std::string fileInfo) {
  base::DictionaryValue* dictionary = new base::DictionaryValue();
  std::string lineData;
  std::string key, value;
  std::istringstream stream(fileInfo);
  while(std::getline(stream, lineData, '\n')) {
    if (lineData.empty()) {
      //empty line jump
      continue;
    } else if(lineData[0] == '#') {
      //comment line jump
      continue;
    }
    size_t index = lineData.find("=");
    if (index != std::string::npos) {
      key = lineData.substr(0, index);
      value = lineData.substr(index + 1);
      dictionary->SetString(key, value);
    }
  }
  return dictionary;
}

std::string AppLauncherManagerLinux::exec(const char* cmd) {
  char buffer[128];
  std::string result = "";
  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    DLOG(INFO) << "pipe open fail";
  }
  else {
    while(!feof(pipe.get())) {
      if (fgets(buffer, 128, pipe.get()) != NULL) {
        result += buffer;
      }
    }
  }
  return result;
}

AppLauncherExecuter::AppLauncherExecuter()
  :base::Thread(AppLauncherExecuter::threadName()) {
    base::ThreadRestrictions::SetIOAllowed(true);
    pidList.clear();
}
AppLauncherExecuter::~AppLauncherExecuter() {
  for(size_t index = 0; index < pidList.size(); index++) {
    std::string program = "kill " + std::to_string(pidList[index]);
    system(program.data());
    int status = 0;
    waitpid(pidList[index], &status, 0);
  }
  if (IsRunning()) {
    Stop();
  }
}

void AppLauncherExecuter::execute(std::string processName) {
  if (!IsRunning()) {
    Start();
  }
  task_runner()->PostTask(FROM_HERE, base::Bind(&AppLauncherExecuter::executeInternal, base::Unretained(this), processName));
}

void AppLauncherExecuter::executeInternal(std::string processName) {
  pid_t pid = fork();
  if (pid == 0) {
    std::vector<std::string> splitData;
    std::vector<char*> args;
    std::string sData;
    std::istringstream streamString(processName);
    while(std::getline(streamString, sData, ' ')) {
      if (!sData.empty()) {
        splitData.push_back(sData);
      }
    }
    if (splitData.size() <= 0) {
      exit(0);
    }
    for(size_t index = 0; index < splitData.size(); index++) {
      args.push_back(const_cast<char*>(splitData[index].data()));
    }
    args.push_back(0);
    execvp(args[0], args.data());
    exit(0);
  }
  pidList.push_back(pid);
}

}  // namespace device
