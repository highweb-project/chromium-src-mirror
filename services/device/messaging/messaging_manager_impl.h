// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGING_MANAGER_IMPL_H_
#define MESSAGING_MESSAGING_MANAGER_IMPL_H_

#include "services/device/messaging/messaging_export.h"
#include "services/device/public/interfaces/messaging_manager.mojom.h"

namespace device {

class MessagingManagerImpl {
 public:
  DEVICE_MESSAGING_EXPORT static void Create(mojom::MessagingManagerRequest request);
};

}  // namespace device

#endif  // MESSAGING_MESSAGING_MANAGER_IMPL_H_
