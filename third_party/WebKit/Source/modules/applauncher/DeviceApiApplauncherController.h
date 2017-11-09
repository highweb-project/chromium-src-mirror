// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_API_APPLAUNCHER_CONTROLLER_H_
#define DEVICE_API_APPLAUNCHER_CONTROLLER_H_

#include "modules/ModulesExport.h"
#include "core/frame/LocalFrame.h"
#include "platform/Supplementable.h"
#include "public/platform/modules/device_api/WebDeviceApiApplauncherClient.h"

namespace blink {

class MODULES_EXPORT DeviceApiApplauncherController final : public GarbageCollectedFinalized<DeviceApiApplauncherController>, public Supplement<LocalFrame> {
	USING_GARBAGE_COLLECTED_MIXIN(DeviceApiApplauncherController);
public:
	~DeviceApiApplauncherController();// override;

    static void ProvideTo(LocalFrame&, WebDeviceApiApplauncherClient*);
    static DeviceApiApplauncherController* From(LocalFrame&);
    static const char* SupplementName();

    WebDeviceApiApplauncherClient* client() { return m_client; };

	DECLARE_VIRTUAL_TRACE();

private:
	DeviceApiApplauncherController(LocalFrame&, WebDeviceApiApplauncherClient*);

	WebDeviceApiApplauncherClient* m_client;
};

}

#endif  // DEVICE_API_APPLAUNCHER_CONTROLLER_H_
