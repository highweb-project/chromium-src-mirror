// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "devicesound_manager_linux.h"
#include <iostream>
#include <stdio.h>


namespace device {

void DeviceSoundManagerLinux::outputDeviceType(const outputDeviceTypeCallback& callback) {
  std::string execResult = exec("pacmd list-sinks");
  DeviceSound_ResultCodePtr result(DeviceSound_ResultCode::New());
  if (execResult.empty()) {
    result->resultCode = int32_t(device::devicesound_ErrorCodeList::FAILURE);
    result->outputType = -1;
  } else {
    result->outputType = getOutputDeviceLinux(execResult);
    if (result->outputType == -1) {
      result->resultCode = int32_t(device::devicesound_ErrorCodeList::FAILURE);
    } else {
      result->resultCode = int32_t(device::devicesound_ErrorCodeList::SUCCESS);
    }
  }
  result->functionCode = int32_t(device::devicesound_function::FUNC_OUTPUT_DEVICE_TYPE);
  result->volume = DeviceSound_Volume::New();
  callback.Run(result.Clone());
}

void DeviceSoundManagerLinux::deviceVolume(const deviceVolumeCallback& callback) {
  std::string execResult = exec("pacmd list-sinks");
  getDeviceVolume(execResult);
  DeviceSound_ResultCodePtr result(DeviceSound_ResultCode::New());
  result->volume = DeviceSound_Volume::New();
  result->functionCode = int32_t(device::devicesound_function::FUNC_DEVICE_VOLUME);
  if (execResult.empty()) {
    result->resultCode = int32_t(device::devicesound_ErrorCodeList::FAILURE);
  } else {
    float volume = getDeviceVolume(execResult);
    if (volume < 0) {
      result->resultCode = int32_t(device::devicesound_ErrorCodeList::FAILURE);
    } else {
      result->resultCode = int32_t(device::devicesound_ErrorCodeList::SUCCESS);
      result->volume->MediaVolume = volume;
      result->volume->SystemVolume = volume;
    }
  }
  result->outputType = -1;
  callback.Run(result.Clone());
}

DeviceSoundManagerLinux::DeviceSoundManagerLinux(mojo::InterfaceRequest<DeviceSoundManager> request)
  : binding_(this, std::move(request)) {
}

DeviceSoundManagerLinux::~DeviceSoundManagerLinux() {
}

std::string DeviceSoundManagerLinux::exec(const char* cmd) {
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

int32_t DeviceSoundManagerLinux::getOutputDeviceLinux(std::string& execResult) {
  size_t selectedDeviceIndex = execResult.find("* index");
  if (selectedDeviceIndex != std::string::npos) {
    size_t afterDeviceIndex = execResult.find("index", selectedDeviceIndex + 5);
    std::string subData = execResult.substr(selectedDeviceIndex, afterDeviceIndex);
    DLOG(INFO) << "data : " << subData;
    size_t activePortIndex = subData.find("active port:");
    if (activePortIndex == std::string::npos) {
      return -1;
    } else {
      return int32_t(device::device_type::DEVICE_DEFAULT);
    }
  } else {
    DLOG(INFO) << "not found : " << selectedDeviceIndex;
  }
  return -1;
}

float DeviceSoundManagerLinux::getDeviceVolume(std::string& execResult) {
  size_t selectedDeviceIndex = execResult.find("* index");
  if (selectedDeviceIndex != std::string::npos) {
    size_t afterDeviceIndex = execResult.find("index", selectedDeviceIndex + 5);
    std::string subData = execResult.substr(selectedDeviceIndex, afterDeviceIndex);
    DLOG(INFO) << "data : " << subData;
    size_t activePortIndex = subData.find("active port:");
    if (activePortIndex == std::string::npos) {
      return -1.0f;
    } else {
      size_t volumeIndex = subData.find("volume:");
      if (volumeIndex == std::string::npos) {
        return -1.0f;
      } else {
        size_t leftVolumeIndex = subData.find("%", volumeIndex);
        if (leftVolumeIndex == std::string::npos) {
          return -1.0f;
        }
        size_t leftVolumeStartIndex = subData.rfind(" ", leftVolumeIndex);
        return std::stof(subData.substr(leftVolumeStartIndex + 1, leftVolumeIndex - leftVolumeStartIndex - 1));
      }
    }
  } else {
    DLOG(INFO) << "not found : " << selectedDeviceIndex;
  }
  return -1.0f;
}

} // namespace device
