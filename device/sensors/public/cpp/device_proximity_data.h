// Copyright 2015 Infraware All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_SENSORS_PUBLIC_CPP_DEVICE_PROXIMITY_DATA_H_
#define DEVICE_SENSORS_PUBLIC_CPP_DEVICE_PROXIMITY_DATA_H_

namespace device {

struct DeviceProximityData {
	DeviceProximityData() : value(-1) {}
  double value;
};

}  // namespace content

#endif // DEVICE_SENSORS_PUBLIC_CPP_DEVICE_PROXIMITY_DATA_H_
