// Copyright 2015 Infraware All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_DEVICE_SENSORS_DEVICE_PROXIMITY_DATA_H_
#define CONTENT_COMMON_DEVICE_SENSORS_DEVICE_PROXIMITY_DATA_H_

#include "content/common/content_export.h"

namespace content {

struct DeviceProximityData {
	DeviceProximityData() : value(-1) {}
  double value;
};

}  // namespace content

#endif // CONTENT_COMMON_DEVICE_SENSORS_DEVICE_PROXIMITY_DATA_H_
