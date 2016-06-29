// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_CALENDAR_CALENDAR_MANAGER_IMPL_H_
#define DEVICE_CALENDAR_CALENDAR_MANAGER_IMPL_H_

#include "device/calendar/calendar_export.h"
#include "device/calendar/calendar_manager.mojom.h"

namespace device {

class CalendarManagerImpl {
 public:
  DEVICE_CALENDAR_EXPORT static void Create(
      mojo::InterfaceRequest<CalendarManager> request);
};

}  // namespace device

#endif  // DEVICE_CALENDAR_CALENDAR_MANAGER_IMPL_H_
