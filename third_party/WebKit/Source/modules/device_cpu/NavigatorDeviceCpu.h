// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * NavigatorDeviceCpu.h
 *
 *  Created on: 2016. 01. 04.
 *      Author: Jeseon Park
 */

#ifndef NavigatorDeviceCpu_h
#define NavigatorDeviceCpu_h

#include "bindings/core/v8/ScriptPromise.h"
#include "core/frame/Navigator.h"
#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"
#include "core/dom/Document.h"
#include "DeviceCpu.h"

namespace blink {

class Navigator;
class DeviceCpu;

class NavigatorDeviceCpu final
    : public GarbageCollected<NavigatorDeviceCpu>,
      public Supplement<Navigator> {
    USING_GARBAGE_COLLECTED_MIXIN(NavigatorDeviceCpu);
public:
    static NavigatorDeviceCpu& From(Navigator&);

    static DeviceCpu* devicecpu(Navigator& navigator);
    DeviceCpu* devicecpu(Document& document);

    DECLARE_TRACE();

private:
    explicit NavigatorDeviceCpu(Navigator&);
    static const char* SupplementName();

    Member<DeviceCpu> mDeviceCpu;
};

}

#endif  // NavigatorDeviceCpu_h
