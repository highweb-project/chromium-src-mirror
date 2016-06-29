// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * WebDeviceGalleryListener.h
 *
 *  Created on: 2016. 01. 25.
 *      Author: pjs3232
 */

#ifndef WebDeviceGalleryListener_h
#define WebDeviceGalleryListener_h

#include "WebDeviceGalleryStatus.h"

namespace blink {

class WebDeviceGalleryStatus;

class WebDeviceGalleryListener{
public:
    virtual void resultDeviceGallery(const WebDeviceGalleryStatus&) = 0;

    virtual ~WebDeviceGalleryListener() { }
};

} // namespace blink

#endif // WebDeviceGalleryListener_h
