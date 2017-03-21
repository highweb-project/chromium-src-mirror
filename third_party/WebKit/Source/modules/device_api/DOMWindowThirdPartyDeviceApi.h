// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOMWindowThirdPartyDeviceApi_h
#define DOMWindowThirdPartyDeviceApi_h

#include "core/frame/DOMWindowProperty.h"
#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"

#include "ThirdPartyDeviceApi.h"

namespace blink {

class DOMWindow;
class LocalDOMWindow;
class ExecutionContext;
class Document;
class SendAndroidBroadcastCallback;

class DOMWindowThirdPartyDeviceApi final :
      public GarbageCollected<DOMWindowThirdPartyDeviceApi>,
      public Supplement<LocalDOMWindow>,
      public DOMWindowProperty {
    USING_GARBAGE_COLLECTED_MIXIN(DOMWindowThirdPartyDeviceApi);
public:
    static DOMWindowThirdPartyDeviceApi& from(LocalDOMWindow&);
    static void createDeviceApi(DOMWindow& window);
    void createDeviceApi(Document& document);
    static void sendAndroidBroadcast(DOMWindow&, String action, SendAndroidBroadcastCallback* callback);
    void sendAndroidBroadcast(String action, SendAndroidBroadcastCallback* callback);

    DECLARE_TRACE();

private:
    explicit DOMWindowThirdPartyDeviceApi(LocalDOMWindow&);
    static const char* supplementName();

    Member<ThirdPartyDeviceApi> mDeviceApi;
};

} // namespace blink

#endif // DOMWindowThirdPartyDeviceApi_h
