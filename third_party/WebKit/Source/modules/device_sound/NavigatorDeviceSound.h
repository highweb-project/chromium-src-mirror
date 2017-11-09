// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorDeviceSound.h
 *
 *  Created on: 2015. 12. 17.
 *      Author: Jeseon Park
 */

#ifndef NavigatorDeviceSound_h
#define NavigatorDeviceSound_h

#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"
#include "DeviceSound.h"

namespace blink {

class Navigator;
class DeviceSound;

class NavigatorDeviceSound final
    : public GarbageCollected<NavigatorDeviceSound>,
      public Supplement<Navigator> {
    USING_GARBAGE_COLLECTED_MIXIN(NavigatorDeviceSound);
public:
    static NavigatorDeviceSound& From(Navigator&);

    static DeviceSound* devicesound(Navigator& navigator);
    DeviceSound* devicesound();

    DECLARE_TRACE();

private:
    explicit NavigatorDeviceSound(Navigator&);
    static const char* SupplementName();

    Member<DeviceSound> mDeviceSound;
};

}

#endif  // NavigatorDeviceSound_h
