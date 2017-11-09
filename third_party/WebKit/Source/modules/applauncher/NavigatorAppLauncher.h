// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorAppLauncher.h
 *
 *  Created on: 2015. 12. 14.
 *      Author: Jeseon Park
 */

#ifndef NavigatorAppLauncher_h
#define NavigatorAppLauncher_h

#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"
#include "AppLauncher.h"

namespace blink {

class Navigator;
class AppLauncher;

class NavigatorAppLauncher final
    : public GarbageCollected<NavigatorAppLauncher>,
      public Supplement<Navigator> {
    USING_GARBAGE_COLLECTED_MIXIN(NavigatorAppLauncher);
public:
    static NavigatorAppLauncher& From(Navigator&);

    static AppLauncher* applauncher(Navigator& navigator);
    AppLauncher* applauncher();

    DECLARE_TRACE();

private:
    explicit NavigatorAppLauncher(Navigator&);
    static const char* SupplementName();

    Member<AppLauncher> mAppLauncher;
};

}

#endif  // NavigatorAppLauncher_h
