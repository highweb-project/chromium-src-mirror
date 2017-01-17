// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorDeviceCpu.cpp
 *
 *  Created on: 2016. 01. 04.
 *      Author: Jeseon Park
 */
#include "wtf/build_config.h"
#include "modules/device_cpu/NavigatorDeviceCpu.h"

#include "core/frame/Navigator.h"
#include "core/frame/LocalFrame.h"

namespace blink {

NavigatorDeviceCpu& NavigatorDeviceCpu::from(Navigator& navigator)
{
    NavigatorDeviceCpu* supplement = static_cast<NavigatorDeviceCpu*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        supplement = new NavigatorDeviceCpu(navigator);
        provideTo(navigator, supplementName(), supplement);
    }
    return *supplement;
}

DeviceCpu* NavigatorDeviceCpu::devicecpu(Navigator& navigator)
{
    return NavigatorDeviceCpu::from(navigator).devicecpu(*(navigator.frame()->document()));
}

DeviceCpu* NavigatorDeviceCpu::devicecpu(Document& document)
{
    if (mDeviceCpu == NULL) {
        mDeviceCpu = DeviceCpu::create(document);
    }
    return mDeviceCpu;
}

DEFINE_TRACE(NavigatorDeviceCpu)
{
    visitor->trace(mDeviceCpu);
    Supplement<Navigator>::trace(visitor);
}

NavigatorDeviceCpu::NavigatorDeviceCpu(Navigator& navigator)
{
}

const char* NavigatorDeviceCpu::supplementName()
{
    return "NavigatorDeviceCpu";
}

}
