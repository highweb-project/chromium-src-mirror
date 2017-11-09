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
#include "platform/wtf/build_config.h"
#include "modules/device_storage/NavigatorDeviceStorage.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorDeviceStorage& NavigatorDeviceStorage::From(Navigator& navigator)
{
    NavigatorDeviceStorage* supplement = static_cast<NavigatorDeviceStorage*>(Supplement<Navigator>::From(navigator, SupplementName()));
    if (!supplement) {
        supplement = new NavigatorDeviceStorage(navigator);
        ProvideTo(navigator, SupplementName(), supplement);
    }
    return *supplement;
}

DeviceStorage* NavigatorDeviceStorage::devicestorage(Navigator& navigator)
{
    return NavigatorDeviceStorage::From(navigator).devicestorage();
}

DeviceStorage* NavigatorDeviceStorage::devicestorage()
{
    return mDeviceStorage;
}

DEFINE_TRACE(NavigatorDeviceStorage)
{
    visitor->Trace(mDeviceStorage);
    Supplement<Navigator>::Trace(visitor);
}

NavigatorDeviceStorage::NavigatorDeviceStorage(Navigator& navigator)
{
    if(navigator.GetFrame())
        mDeviceStorage = DeviceStorage::Create(*navigator.GetFrame());
}

const char* NavigatorDeviceStorage::SupplementName()
{
    return "NavigatorDeviceStorage";
}

}
