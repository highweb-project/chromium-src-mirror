// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "devicestorage_manager_linux.h"
#include "base/json/json_reader.h"
#include "base/values.h"

namespace device {

DeviceStorageManagerLinux::DeviceStorageManagerLinux(mojom::DeviceStorageManagerRequest request)
  : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {
}

DeviceStorageManagerLinux::~DeviceStorageManagerLinux(){
}

void DeviceStorageManagerLinux::getDeviceStorage(getDeviceStorageCallback callback) {
  mojom::DeviceStorage_ResultCodePtr result(mojom::DeviceStorage_ResultCode::New());
  result->resultCode = int32_t(mojom::device_storage_ErrorCodeList::NOT_SUPPORT_API);
  result->functionCode = int32_t(mojom::device_storage_function::FUNC_GET_DEVICE_STORAGE);
  std::string execResult = exec("lsblk -b -J");

  if (!execResult.empty()) {
    std::unique_ptr<base::Value> data = base::JSONReader::Read(execResult);
    if (data.get() && data.get()->GetType() == base::Value::Type::DICTIONARY) {
      base::DictionaryValue* blockdevicesValue = nullptr;
      data->GetAsDictionary(&blockdevicesValue);

      if (blockdevicesValue && blockdevicesValue->size() > 0) {
        base::ListValue* deviceList = nullptr;
        blockdevicesValue->GetList("blockdevices", &deviceList);
        if (deviceList && deviceList->GetSize() > 0) {
          result->storageList = std::vector<mojom::DeviceStorage_StorageInfoPtr>();
          for(size_t listIndex = 0; listIndex < deviceList->GetSize(); listIndex++) {
            mojom::DeviceStorage_StorageInfoPtr info(mojom::DeviceStorage_StorageInfo::New());
            base::DictionaryValue* device = nullptr;
            deviceList->GetDictionary(listIndex, &device);
            if (!device) {
              DLOG(INFO) << "not device " << listIndex;
              continue;
            } else if (device->size() <= 0) {
              DLOG(INFO) << "device hasn't data " << listIndex;
              continue;
            }
            std::string stringValue;
            device->GetString("size", &stringValue);
            if (!stringValue.empty()) {
              info->capacity = uint64_t(std::stoll(stringValue));
            } else {
              info->capacity = 0;
            }
            device->GetString("rm", &stringValue);
            if (!stringValue.empty() && stringValue != "0") {
              info->isRemovable = true;
            } else {
              info->isRemovable = false;
            }
            if (info->isRemovable) {
              info->type = uint64_t(mojom::device_storage_type::DEVICE_USB);
            } else {
              info->type = uint64_t(mojom::device_storage_type::DEVICE_HARDDISK);
            }
            info->availableCapacity = 0;
            base::ListValue* childList = nullptr;
            device->GetList("children", &childList);
            if (childList && childList->GetSize() > 0) {
              std::string childrenPath;
              std::string dfExecResult;
              base::DictionaryValue* child = nullptr;
              for(size_t childIndex = 0; childIndex < childList->GetSize(); childIndex++) {
                childList->GetDictionary(childIndex, &child);
                if (!child) {
                  continue;
                }
                child->GetString("mountpoint", &childrenPath);
                if (childrenPath.empty()) {
                  continue;
                }
                size_t blankIndex = childrenPath.find(" ");
                while(blankIndex != std::string::npos) {
                  childrenPath.replace(blankIndex, 1, "\\ ");
                  blankIndex = childrenPath.find(" ", blankIndex + 2);
                }
                childrenPath = "df -B1 --output=avail " + childrenPath;
                dfExecResult = exec(childrenPath.data());
                size_t dataIndex = dfExecResult.find("\n");
                if (dataIndex != std::string::npos && dfExecResult.length() > dataIndex + 1) {
                  info->availableCapacity += std::stoll(dfExecResult.substr(dataIndex + 1));
                }
              }
            }
            childList = nullptr;
            device = nullptr;
            DLOG(INFO) << "info " << info->type << ", " << info->capacity << ", " << info->isRemovable << " , " << info->availableCapacity;
            result->storageList->push_back(std::move(info));
          }
        }
        deviceList = nullptr;
      }
      blockdevicesValue = nullptr;
    }
    data.release();
  }
  result->resultCode = int32_t(mojom::device_storage_ErrorCodeList::SUCCESS);
  std::move(callback).Run(result.Clone());
}

std::string DeviceStorageManagerLinux::exec(const char* cmd) {
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

} // namespace device
