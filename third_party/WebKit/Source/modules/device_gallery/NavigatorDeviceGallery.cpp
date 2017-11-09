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
#include "platform/wtf/build_config.h"
#include "modules/device_gallery/NavigatorDeviceGallery.h"

#include "core/frame/Navigator.h"
#include "base/logging.h"

namespace blink {

NavigatorDeviceGallery& NavigatorDeviceGallery::From(Navigator& navigator)
{
    NavigatorDeviceGallery* supplement = static_cast<NavigatorDeviceGallery*>(Supplement<Navigator>::From(navigator, SupplementName()));
    if (!supplement) {
        supplement = new NavigatorDeviceGallery(navigator);
        ProvideTo(navigator, SupplementName(), supplement);
    }
    return *supplement;
}

DeviceGallery* NavigatorDeviceGallery::devicegallery(Navigator& navigator)
{
    return NavigatorDeviceGallery::From(navigator).devicegallery();
}

DeviceGallery* NavigatorDeviceGallery::devicegallery()
{
    return mDeviceGallery;
}

DEFINE_TRACE(NavigatorDeviceGallery)
{
    visitor->Trace(mDeviceGallery);
    Supplement<Navigator>::Trace(visitor);
}

NavigatorDeviceGallery::NavigatorDeviceGallery(Navigator& navigator)
{
    if(navigator.GetFrame())
        mDeviceGallery = DeviceGallery::create(*navigator.GetFrame());
}

const char* NavigatorDeviceGallery::SupplementName()
{
    return "NavigatorDeviceGallery";
}

}
