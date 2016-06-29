// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLINK_PLATFORM_APPLAUNCHER_PLATFORM_APPSTATUS_H_
#define BLINK_PLATFORM_APPLAUNCHER_PLATFORM_APPSTATUS_H_

#include "platform/PlatformExport.h"
#include "platform/heap/Handle.h"
#include "platform/applauncher/platform_applicationinfo.h"

namespace blink {

class PLATFORM_EXPORT PlatformAppStatus : public GarbageCollectedFinalized<PlatformAppStatus>{
public:

  PlatformAppStatus();
	virtual ~PlatformAppStatus();

  void setResultCode(int code);

  int mResultCode = -1;

	DECLARE_VIRTUAL_TRACE();
};

} // namespace blink

#endif // BLINK_PLATFORM_APPLAUNCHER_PLATFORM_APPSTATUS_H_
