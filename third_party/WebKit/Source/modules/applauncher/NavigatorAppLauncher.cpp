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
#include "platform/wtf/build_config.h"
#include "modules/applauncher/NavigatorAppLauncher.h"

#include "core/frame/Navigator.h"

namespace blink {

NavigatorAppLauncher& NavigatorAppLauncher::From(Navigator& navigator)
{
    NavigatorAppLauncher* supplement = static_cast<NavigatorAppLauncher*>(Supplement<Navigator>::From(navigator, SupplementName()));
    if (!supplement) {
        supplement = new NavigatorAppLauncher(navigator);
        ProvideTo(navigator, SupplementName(), supplement);
    }
    return *supplement;
}

AppLauncher* NavigatorAppLauncher::applauncher(Navigator& navigator)
{
    return NavigatorAppLauncher::From(navigator).applauncher();
}

AppLauncher* NavigatorAppLauncher::applauncher()
{
    return mAppLauncher;
}

DEFINE_TRACE(NavigatorAppLauncher)
{
    visitor->Trace(mAppLauncher);
    Supplement<Navigator>::Trace(visitor);
}

NavigatorAppLauncher::NavigatorAppLauncher(Navigator& navigator)
{
    if(navigator.GetFrame())
        mAppLauncher = AppLauncher::create(*navigator.GetFrame());
}

const char* NavigatorAppLauncher::SupplementName()
{
    return "NavigatorAppLauncher";
}

}
