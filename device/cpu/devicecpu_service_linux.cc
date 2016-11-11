// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "devicecpu_service_linux.h"
#include <iostream>
#include <fstream>
#include <string>

namespace device {

DeviceCpuCallbackThread::DeviceCpuCallbackThread()
  :base::Thread(DeviceCpuCallbackThread::threadName()) {
  base::ThreadRestrictions::SetIOAllowed(true);
  isAlive = true;
}
DeviceCpuCallbackThread::~DeviceCpuCallbackThread() {
  innerStop();
}

void DeviceCpuCallbackThread::innerStart() {
  if (!this->IsRunning()) {
    this->Start();
  }
}
void DeviceCpuCallbackThread::innerStop() {
  isAlive = false;
  if (this->IsRunning()) {
    this->Stop();
  }
}

void DeviceCpuCallbackThread::startCallbackThread(const DeviceCpuCallback& callback) {
  if (stillRunning) {
    return;
  }
  stillRunning = true;
  innerStart();
  task_runner()->PostTask(FROM_HERE, base::Bind(&DeviceCpuCallbackThread::callbackThread, base::Unretained(this), callback));
}

void DeviceCpuCallbackThread::callbackThread(const DeviceCpuCallback& callback) {
  for(int i = 0; i < 5 && isAlive; i++) {
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(200));
  }
  if (IsRunning()) {
    callback.Run();
  }
  stillRunning = false;
}

DeviceCpuMonitorThread::DeviceCpuMonitorThread()
  :base::Thread(DeviceCpuMonitorThread::threadName()) {
  base::ThreadRestrictions::SetIOAllowed(true);
  isAlive = true;
  callback_ = nullptr;
}

DeviceCpuMonitorThread::~DeviceCpuMonitorThread() {
  stopMonitor();
}

void DeviceCpuMonitorThread::startMonitor(const DeviceCpuMonitorCallback& callback) {
  if (!IsRunning()) {
    Start();
  }
  if (!stillRunning) {
    task_runner()->PostTask(FROM_HERE, base::Bind(&DeviceCpuMonitorThread::readUsage, base::Unretained(this), callback));
  }
}

void DeviceCpuMonitorThread::stopMonitor() {
  isAlive = false;
  stillRunning = false;
  callback_ = nullptr;
  if (IsRunning()) {
    Stop();
  }
}

void DeviceCpuMonitorThread::readUsage(const DeviceCpuMonitorCallback& callback) {
  float usage = -1.0f;
  std::string data;
  std::ifstream inFile;
  inFile.open("/proc/stat", std::ios::in);
  if (inFile.fail() || !isAlive) {
    return;
  } else {
    if(!inFile.eof()) {
      std::getline(inFile, data);
    } else {
      callback.Run(usage);
      inFile.close();
      return;
    }
    std::vector<std::string> splitData;
    std::string sData;
    std::istringstream streamString(data);
    while(std::getline(streamString, sData, ' ') && isAlive) {
      if (!sData.empty()) {
        splitData.push_back(sData);
      }
    }
    if (!isAlive) {
      splitData.clear();
      streamString.clear();
      inFile.close();
      return;
    }

    long idle1 = 0;
    if (splitData.size() > 5) {
      //add idle and iowait = Total CPU idle time since boot
      idle1 = std::stol(splitData[4]) + std::stol(splitData[5]);
    } else if (splitData.size() > 4) {
      idle1 = std::stol(splitData[4]);
    } else {
      splitData.clear();
      streamString.clear();
      inFile.close();
      return;
    }
    long cpu1 = 0;
    for(unsigned i = 1; i < splitData.size(); i++) {
      cpu1 += std::stol(splitData[i]);
    }
    splitData.clear();
    streamString.clear();

    for(int i = 0; i < 5 && isAlive; i++) {
      base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(200));
    }

    if (!isAlive) {
      inFile.close();
      return;
    }

    if (!inFile.eof() && isAlive) {
      inFile.seekg(std::ios::beg);
      std::getline(inFile, data);
    } else if (isAlive){
      inFile.clear();
      inFile.seekg(std::ios::beg);
      std::getline(inFile, data);
    } else {
      splitData.clear();
      streamString.clear();
      inFile.close();
      return;
    }
    streamString.str(data);

    while(std::getline(streamString, sData, ' ') && isAlive) {
      if (!sData.empty()) {
        splitData.push_back(sData);
      }
    }
    if (!isAlive) {
      splitData.clear();
      streamString.clear();
      inFile.close();
      return;
    }

    long idle2 = 0;
    if (splitData.size() > 5) {
      //add idle and iowait = Total CPU idle time since boot
      idle2 = std::stol(splitData[4]) + std::stol(splitData[5]);
    } else if (splitData.size() > 4) {
      idle2 = std::stol(splitData[4]);
    } else {
      splitData.clear();
      streamString.clear();
      inFile.close();
      return;
    }
    long cpu2 = 0;
    //cpu2 = Total CPU time since boot
    for(unsigned i = 1; i < splitData.size(); i++) {
      cpu2 += std::stol(splitData[i]);
    }

    usage = 1 - (float)(idle2 - idle1) / (cpu2 - cpu1);
    splitData.clear();
    streamString.clear();
    inFile.close();
  }
  if (isAlive) {
    callback.Run(usage);
  }
  stillRunning = false;
}

} //namepsace device
