// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_STORAGE_STORAGE_MANAGER_IMPL_H_
#define DEVICE_STORAGE_STORAGE_MANAGER_IMPL_H_

#include "device/storage/devicestorage_export.h"
#include "device/storage/devicestorage_manager.mojom.h"

namespace device {

class DeviceStorageManagerImpl {
 public:
  DEVICE_STORAGE_EXPORT static void Create(
      mojo::InterfaceRequest<DeviceStorageManager> request);
};

}  // namespace device

#endif  // DEVICE_STORAGE_STORAGE_MANAGER_IMPL_H_
