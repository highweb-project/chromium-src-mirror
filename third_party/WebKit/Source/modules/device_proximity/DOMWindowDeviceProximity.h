// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOMWindowDeviceProximity_h
#define DOMWindowDeviceProximity_h

#include "modules/EventTargetModules.h"

namespace blink {

class DOMWindowDeviceProximity {
public:
    DEFINE_STATIC_ATTRIBUTE_EVENT_LISTENER(deviceproximity);
};

} // namespace blink

#endif // DOMWindowDeviceProximity_h
