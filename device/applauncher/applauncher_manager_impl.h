// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_APPLAUNCHER_APPLAUNCHER_MANAGER_IMPL_H_
#define DEVICE_APPLAUNCHER_APPLAUNCHER_MANAGER_IMPL_H_

#include "device/applauncher/applauncher_export.h"
#include "device/applauncher/applauncher_manager.mojom.h"

namespace device {

class AppLauncherManagerImpl {
 public:
  DEVICE_APPLAUNCHER_EXPORT static void Create(
      mojo::InterfaceRequest<AppLauncherManager> request);
};

}  // namespace device

#endif  // DEVICE_APPLAUNCHER_APPLAUNCHER_MANAGER_IMPL_H_
