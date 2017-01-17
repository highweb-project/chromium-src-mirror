// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorDeviceStorage.cpp
 *
 *  Created on: 2015. 12. 23.
 *      Author: Jeseon Park
 */
#include "wtf/build_config.h"
#include "modules/device_storage/NavigatorDeviceStorage.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorDeviceStorage& NavigatorDeviceStorage::from(Navigator& navigator)
{
    NavigatorDeviceStorage* supplement = static_cast<NavigatorDeviceStorage*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        supplement = new NavigatorDeviceStorage(navigator);
        provideTo(navigator, supplementName(), supplement);
    }
    return *supplement;
}

DeviceStorage* NavigatorDeviceStorage::devicestorage(Navigator& navigator)
{
    return NavigatorDeviceStorage::from(navigator).devicestorage();
}

DeviceStorage* NavigatorDeviceStorage::devicestorage()
{
    return mDeviceStorage;
}

DEFINE_TRACE(NavigatorDeviceStorage)
{
    visitor->trace(mDeviceStorage);
    Supplement<Navigator>::trace(visitor);
}

NavigatorDeviceStorage::NavigatorDeviceStorage(Navigator& navigator)
{
    if(navigator.frame())
        mDeviceStorage = DeviceStorage::create(*navigator.frame());
}

const char* NavigatorDeviceStorage::supplementName()
{
    return "NavigatorDeviceStorage";
}

}
