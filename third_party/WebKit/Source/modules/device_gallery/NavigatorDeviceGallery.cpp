// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorDeviceGallery.cpp
 *
 *  Created on: 2016. 01. 21.
 *      Author: Jeseon Park
 */
#include "wtf/build_config.h"
#include "modules/device_gallery/NavigatorDeviceGallery.h"

#include "core/frame/Navigator.h"
#include "base/logging.h"

namespace blink {

NavigatorDeviceGallery& NavigatorDeviceGallery::from(Navigator& navigator)
{
    NavigatorDeviceGallery* supplement = static_cast<NavigatorDeviceGallery*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        supplement = new NavigatorDeviceGallery(navigator);
        provideTo(navigator, supplementName(), supplement);
    }
    return *supplement;
}

DeviceGallery* NavigatorDeviceGallery::devicegallery(Navigator& navigator)
{
    return NavigatorDeviceGallery::from(navigator).devicegallery();
}

DeviceGallery* NavigatorDeviceGallery::devicegallery()
{
    return mDeviceGallery;
}

DEFINE_TRACE(NavigatorDeviceGallery)
{
    visitor->trace(mDeviceGallery);
    Supplement<Navigator>::trace(visitor);
}

NavigatorDeviceGallery::NavigatorDeviceGallery(Navigator& navigator)
{
    if(navigator.frame())
        mDeviceGallery = DeviceGallery::create(*navigator.frame());
}

const char* NavigatorDeviceGallery::supplementName()
{
    return "NavigatorDeviceGallery";
}

}
