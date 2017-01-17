// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorAppLauncher.cpp
 *
 *  Created on: 2015. 12. 14.
 *      Author: Jeseon Park
 */
#include "wtf/build_config.h"
#include "modules/applauncher/NavigatorAppLauncher.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorAppLauncher& NavigatorAppLauncher::from(Navigator& navigator)
{
    NavigatorAppLauncher* supplement = static_cast<NavigatorAppLauncher*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        supplement = new NavigatorAppLauncher(navigator);
        provideTo(navigator, supplementName(), supplement);
    }
    return *supplement;
}

AppLauncher* NavigatorAppLauncher::applauncher(Navigator& navigator)
{
    return NavigatorAppLauncher::from(navigator).applauncher();
}

AppLauncher* NavigatorAppLauncher::applauncher()
{
    return mAppLauncher;
}

DEFINE_TRACE(NavigatorAppLauncher)
{
    visitor->trace(mAppLauncher);
    Supplement<Navigator>::trace(visitor);
}

NavigatorAppLauncher::NavigatorAppLauncher(Navigator& navigator)
{
    if(navigator.frame())
        mAppLauncher = AppLauncher::create(*navigator.frame());
}

const char* NavigatorAppLauncher::supplementName()
{
    return "NavigatorAppLauncher";
}

}
