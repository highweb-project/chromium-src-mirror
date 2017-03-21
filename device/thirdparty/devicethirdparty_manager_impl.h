// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_THIRDPARTY_THIRDPARTY_MANAGER_IMPL_H_
#define DEVICE_THIRDPARTY_THIRDPARTY_MANAGER_IMPL_H_

#include "device/thirdparty/devicethirdparty_export.h"
#include "device/thirdparty/devicethirdparty_manager.mojom.h"

namespace device {

class DeviceThirdpartyManagerImpl {
 public:
  DEVICE_THIRDPARTY_EXPORT static void Create(
      DeviceThirdpartyManagerRequest request);
};

}  // namespace device

#endif  // DEVICE_THIRDPARTY_THIRDPARTY_MANAGER_IMPL_H_
