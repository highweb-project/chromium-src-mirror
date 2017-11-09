// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "modules/device_api/DOMWindowThirdPartyDeviceApi.h"
#include "core/dom/Document.h"
#include "core/frame/LocalDOMWindow.h"
#include "SendAndroidBroadcastCallback.h"

namespace blink {

DOMWindowThirdPartyDeviceApi::DOMWindowThirdPartyDeviceApi(LocalDOMWindow& window)
{
}

const char* DOMWindowThirdPartyDeviceApi::SupplementName()
{
    return "DOMWindowThirdPartyDeviceApi";
}

DOMWindowThirdPartyDeviceApi& DOMWindowThirdPartyDeviceApi::From(LocalDOMWindow& window)
{
	DOMWindowThirdPartyDeviceApi* supplement = static_cast<DOMWindowThirdPartyDeviceApi*>(Supplement<LocalDOMWindow>::From(window, SupplementName()));
    if (!supplement) {
        supplement = new DOMWindowThirdPartyDeviceApi(window);
        ProvideTo(window, SupplementName(), supplement);
    }
    return *supplement;
}

void DOMWindowThirdPartyDeviceApi::createDeviceApi(DOMWindow& window)
{
    return DOMWindowThirdPartyDeviceApi::From((LocalDOMWindow&)window).createDeviceApi(*((LocalDOMWindow&)window).document());
}

void DOMWindowThirdPartyDeviceApi::createDeviceApi(Document& document)
{
    if (mDeviceApi == NULL) {
        mDeviceApi = ThirdPartyDeviceApi::create(document);
    }
}

void DOMWindowThirdPartyDeviceApi::sendAndroidBroadcast(DOMWindow& window, String action, SendAndroidBroadcastCallback* callback) {
  createDeviceApi(window);
  DOMWindowThirdPartyDeviceApi::From((LocalDOMWindow&)window).sendAndroidBroadcast(action, callback);
}

void DOMWindowThirdPartyDeviceApi::sendAndroidBroadcast(String action, SendAndroidBroadcastCallback* callback) {
  mDeviceApi->sendAndroidBroadcast(action, callback);
}

DEFINE_TRACE(DOMWindowThirdPartyDeviceApi)
{
  visitor->Trace(mDeviceApi);
  Supplement<LocalDOMWindow>::Trace(visitor);
}

} // namespace blink
