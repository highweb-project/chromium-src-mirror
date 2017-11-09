// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_CONTACT_CONTACT_MANAGER_IMPL_H_
#define DEVICE_CONTACT_CONTACT_MANAGER_IMPL_H_

#include "services/device/contact/contact_export.h"
#include "services/device/public/interfaces/contact_manager.mojom.h"

namespace device {

class ContactManagerImpl {
 public:
  DEVICE_CONTACT_EXPORT static void Create(mojom::ContactManagerRequest request);
};

}  // namespace device

#endif  // DEVICE_CONTACT_CONTACT_MANAGER_IMPL_H_
