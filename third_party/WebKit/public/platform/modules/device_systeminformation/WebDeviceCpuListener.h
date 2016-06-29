// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * WebDeviceCpuListener.h
 *
 *  Created on: 2016. 01. 04.
 *      Author: pjs3232
 */

#ifndef WebDeviceCpuListener_h
#define WebDeviceCpuListener_h

#include "WebDeviceCpuStatus.h"

namespace blink {

class WebDeviceCpuStatus;

class WebDeviceCpuListener{
public:
    virtual void resultDeviceCpu(const WebDeviceCpuStatus&) = 0;

    virtual ~WebDeviceCpuListener() { }
};

} // namespace blink

#endif // WebDeviceCpuListener_h
