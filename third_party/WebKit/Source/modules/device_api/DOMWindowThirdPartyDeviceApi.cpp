// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "modules/device_api/DOMWindowThirdPartyDeviceApi.h"
#include "core/dom/Document.h"
#include "core/frame/LocalDOMWindow.h"
#include "SendAndroidBroadcastCallback.h"

namespace blink {

DOMWindowThirdPartyDeviceApi::DOMWindowThirdPartyDeviceApi(LocalDOMWindow& window)
    : DOMWindowProperty(window.frame())
{
}

const char* DOMWindowThirdPartyDeviceApi::supplementName()
{
    return "DOMWindowThirdPartyDeviceApi";
}

DOMWindowThirdPartyDeviceApi& DOMWindowThirdPartyDeviceApi::from(LocalDOMWindow& window)
{
	DOMWindowThirdPartyDeviceApi* supplement = static_cast<DOMWindowThirdPartyDeviceApi*>(Supplement<LocalDOMWindow>::from(window, supplementName()));
    if (!supplement) {
        supplement = new DOMWindowThirdPartyDeviceApi(window);
        provideTo(window, supplementName(), supplement);
    }
    return *supplement;
}

void DOMWindowThirdPartyDeviceApi::createDeviceApi(DOMWindow& window)
{
    return DOMWindowThirdPartyDeviceApi::from((LocalDOMWindow&)window).createDeviceApi(*window.document());
}

void DOMWindowThirdPartyDeviceApi::createDeviceApi(Document& document)
{
    if (mDeviceApi == NULL) {
        mDeviceApi = ThirdPartyDeviceApi::create(document);
    }
}

void DOMWindowThirdPartyDeviceApi::sendAndroidBroadcast(DOMWindow& window, String action, SendAndroidBroadcastCallback* callback) {
  createDeviceApi(window);
  DOMWindowThirdPartyDeviceApi::from((LocalDOMWindow&)window).sendAndroidBroadcast(action, callback);
}

void DOMWindowThirdPartyDeviceApi::sendAndroidBroadcast(String action, SendAndroidBroadcastCallback* callback) {
  mDeviceApi->sendAndroidBroadcast(action, callback);
}

DEFINE_TRACE(DOMWindowThirdPartyDeviceApi)
{
  visitor->trace(mDeviceApi);
	Supplement<LocalDOMWindow>::trace(visitor);
	DOMWindowProperty::trace(visitor);
}

} // namespace blink
