// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "AppLauncherListener.h"
#include "AppStatus.h"
#include "public/platform/Platform.h"
#include "AppLauncher.h"
#include "ApplicationInfo.h"
#include "wtf/text/WTFString.h"
#include "platform/applauncher/platform_appstatus.h"

namespace blink {

AppLauncherListener* AppLauncherListener::instance(AppLauncher* launcher)
{
	AppLauncherListener* callback = new AppLauncherListener(launcher);
	return callback;
}

AppLauncherListener::AppLauncherListener(AppLauncher* launcher)
{
	mAppLauncher = launcher;
	mAppStatus = AppStatus::create();
}

AppLauncherListener::~AppLauncherListener()
{
	mAppStatus.get()->appList().clear();
	mAppStatus = NULL;
	mAppLauncher = NULL;
}

DEFINE_TRACE(AppLauncherListener)
{
	visitor->trace(mAppStatus);
	visitor->trace(mAppLauncher);
}

void AppLauncherListener::resultAppLauncher()
{
	mAppLauncher->resultCodeCallback();
}

AppStatus* AppLauncherListener::lastData()
{
	if (mAppStatus != nullptr)
		return mAppStatus.get();
	else
		return nullptr;
}

void AppLauncherListener::appendAppList(PlatformApplicationInfo* data) {
	if (mAppStatus != nullptr) {
		mAppStatus->appList().append((ApplicationInfo*)data);
	}
}

void AppLauncherListener::setApplicationInfo(PlatformApplicationInfo* info) {
	if (mAppStatus != nullptr) {
		mAppStatus->setAppInfo((ApplicationInfo*)info);
	}
}

PlatformApplicationInfo* AppLauncherListener::getPlatformApplicationInfo() {
	return (PlatformApplicationInfo*)(new ApplicationInfo());
}

} // namespace blink
