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
#include "wtf/build_config.h"
#include "modules/device_sound/NavigatorDeviceSound.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorDeviceSound& NavigatorDeviceSound::from(Navigator& navigator)
{
    NavigatorDeviceSound* supplement = static_cast<NavigatorDeviceSound*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        supplement = new NavigatorDeviceSound(navigator);
        provideTo(navigator, supplementName(), supplement);
    }
    return *supplement;
}

DeviceSound* NavigatorDeviceSound::devicesound(Navigator& navigator)
{
    return NavigatorDeviceSound::from(navigator).devicesound();
}

DeviceSound* NavigatorDeviceSound::devicesound()
{
    return mDeviceSound;
}

DEFINE_TRACE(NavigatorDeviceSound)
{
    visitor->trace(mDeviceSound);
    Supplement<Navigator>::trace(visitor);
}

NavigatorDeviceSound::NavigatorDeviceSound(Navigator& navigator)
{
    if(navigator.frame())
        mDeviceSound = DeviceSound::create(*navigator.frame());
}

const char* NavigatorDeviceSound::supplementName()
{
    return "NavigatorDeviceSound";
}

}
