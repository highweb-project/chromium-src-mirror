// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AppLauncherListener_h
#define AppLauncherListener_h

#include "wtf/text/WTFString.h"
#include "platform/applauncher/applauncher_dispatcher.h"
#include "platform/applauncher/platform_appstatus.h"

namespace blink {

class AppStatus;
class AppLauncher;

class AppLauncherListener final : public GarbageCollectedFinalized<AppLauncherListener>, public AppLauncherDispatcher::Listener {
public:
	static AppLauncherListener* instance(AppLauncher*);
	virtual ~AppLauncherListener();

	void resultAppLauncher() override;
	void appendAppList(PlatformApplicationInfo* data) override;
	void setApplicationInfo(PlatformApplicationInfo* info) override;
	PlatformApplicationInfo* getPlatformApplicationInfo() override;

	AppStatus* lastData();

	DECLARE_VIRTUAL_TRACE();

private:
	AppLauncherListener(AppLauncher*);
	Member<AppStatus> mAppStatus;
	Member<AppLauncher> mAppLauncher;
};

} // namespace blink

#endif // AppLauncherListener_h
