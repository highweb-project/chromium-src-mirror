// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorDeviceSound.cpp
 *
 *  Created on: 2015. 12. 17.
 *      Author: Jeseon Park
 */
#include "platform/wtf/build_config.h"
#include "modules/device_sound/NavigatorDeviceSound.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorDeviceSound& NavigatorDeviceSound::From(Navigator& navigator)
{
    NavigatorDeviceSound* supplement = static_cast<NavigatorDeviceSound*>(Supplement<Navigator>::From(navigator, SupplementName()));
    if (!supplement) {
        supplement = new NavigatorDeviceSound(navigator);
        ProvideTo(navigator, SupplementName(), supplement);
    }
    return *supplement;
}

DeviceSound* NavigatorDeviceSound::devicesound(Navigator& navigator)
{
    return NavigatorDeviceSound::From(navigator).devicesound();
}

DeviceSound* NavigatorDeviceSound::devicesound()
{
    return mDeviceSound;
}

DEFINE_TRACE(NavigatorDeviceSound)
{
    visitor->Trace(mDeviceSound);
    Supplement<Navigator>::Trace(visitor);
}

NavigatorDeviceSound::NavigatorDeviceSound(Navigator& navigator)
{
    if(navigator.GetFrame())
        mDeviceSound = DeviceSound::Create(*navigator.GetFrame());
}

const char* NavigatorDeviceSound::SupplementName()
{
    return "NavigatorDeviceSound";
}

}
