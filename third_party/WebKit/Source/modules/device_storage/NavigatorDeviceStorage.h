// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorDeviceStorage.h
 *
 *  Created on: 2015. 12. 17.
 *      Author: Jeseon Park
 */

#ifndef NavigatorDeviceStorage_h
#define NavigatorDeviceStorage_h

#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"
#include "DeviceStorage.h"

namespace blink {

class Navigator;
class DeviceStorage;

class NavigatorDeviceStorage final
    : public GarbageCollected<NavigatorDeviceStorage>,
      public Supplement<Navigator> {
    USING_GARBAGE_COLLECTED_MIXIN(NavigatorDeviceStorage);
public:
    static NavigatorDeviceStorage& From(Navigator&);

    static DeviceStorage* devicestorage(Navigator& navigator);
    DeviceStorage* devicestorage();

    DECLARE_TRACE();

private:
    explicit NavigatorDeviceStorage(Navigator&);
    static const char* SupplementName();

    Member<DeviceStorage> mDeviceStorage;
};

}

#endif  // NavigatorDeviceStorage_h
