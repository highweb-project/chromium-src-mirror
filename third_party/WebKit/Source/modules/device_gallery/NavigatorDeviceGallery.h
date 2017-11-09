// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorDeviceGallery.h
 *
 *  Created on: 2016. 01. 21.
 *      Author: Jeseon Park
 */

#ifndef NavigatorDeviceGallery_h
#define NavigatorDeviceGallery_h

#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"
#include "DeviceGallery.h"

namespace blink {

class Navigator;
class DeviceGallery;

class NavigatorDeviceGallery final
    : public GarbageCollected<NavigatorDeviceGallery>,
      public Supplement<Navigator> {
    USING_GARBAGE_COLLECTED_MIXIN(NavigatorDeviceGallery);
public:
    static NavigatorDeviceGallery& From(Navigator&);

    static DeviceGallery* devicegallery(Navigator& navigator);
    DeviceGallery* devicegallery();

    DECLARE_TRACE();

private:
    explicit NavigatorDeviceGallery(Navigator&);
    static const char* SupplementName();
    Member<DeviceGallery> mDeviceGallery;
};

}

#endif  // NavigatorDeviceGallery_h
