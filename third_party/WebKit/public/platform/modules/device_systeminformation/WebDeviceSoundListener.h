// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * WebDeviceSoundListener.h
 *
 *  Created on: 2015. 12. 17.
 *      Author: pjs3232
 */

#ifndef WebDeviceSoundListener_h
#define WebDeviceSoundListener_h

#include "WebDeviceSoundStatus.h"

namespace blink {

class WebDeviceSoundStatus;

class WebDeviceSoundListener {
public:
    virtual void resultDeviceSound(const WebDeviceSoundStatus&) = 0;

    virtual ~WebDeviceSoundListener() { }
};

} // namespace blink

#endif // WebDeviceSoundListener_h
