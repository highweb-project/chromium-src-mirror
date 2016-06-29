// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * WebDeviceStorageListener.h
 *
 *  Created on: 2015. 12. 23.
 *      Author: pjs3232
 */

#ifndef WebDeviceStorageListener_h
#define WebDeviceStorageListener_h

#include "WebDeviceStorageStatus.h"

namespace blink {

class WebDeviceStorageStatus;

class WebDeviceStorageListener {
public:
    virtual void resultDeviceStorage(const WebDeviceStorageStatus&) = 0;

    virtual ~WebDeviceStorageListener() { }
};

} // namespace blink

#endif // WebDeviceStorageListener_h
