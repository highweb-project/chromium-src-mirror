// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_SOUND_DEVICESOUND_MANAGER_IMPL_H_
#define DEVICE_SOUND_DEVICESOUND_MANAGER_IMPL_H_

#include "services/device/sound/devicesound_export.h"
#include "services/device/public/interfaces/devicesound_manager.mojom.h"

namespace device {

class DeviceSoundManagerImpl {
 public:
  DEVICE_SOUND_EXPORT static void Create(
      mojom::DeviceSoundManagerRequest request);
};

}  // namespace device

#endif  // DEVICE_SOUND_DEVICESOUND_MANAGER_IMPL_H_
