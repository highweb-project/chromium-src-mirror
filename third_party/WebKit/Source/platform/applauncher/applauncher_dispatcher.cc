// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//#include "platform/threading/BindForMojo.h"
#include "platform/mojo/MojoHelper.h"
#include "public/platform/Platform.h"
#include "wtf/text/WTFString.h"

#include "applauncher_dispatcher.h"
#include "platform_appstatus.h"
#include "public/platform/ServiceRegistry.h"

namespace blink {

AppLauncherDispatcher::AppLauncherDispatcher() {
	Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&manager_));
}

AppLauncherDispatcher::~AppLauncherDispatcher() {
	manager_.reset();
	listener_ = nullptr;
	status_ = nullptr;
}

DEFINE_TRACE(AppLauncherDispatcher) {
	visitor->trace(status_);
}

void AppLauncherDispatcher::launchApp(const String& packageName,
	const String& activityName, Listener* mCallback, PlatformAppStatus* status) {
	listener_ = mCallback;
	status_ = status;
	manager_->LaunchApp(mojo::String(packageName.utf8().data()), mojo::String(activityName.utf8().data()),
		//sameThreadBindForMojo(&AppLauncherDispatcher::resultCodeCallback, this));
		createBaseCallback(bind<device::AppLauncher_ResultCodePtr>(&AppLauncherDispatcher::resultCodeCallback, this)));
}

void AppLauncherDispatcher::removeApp(const String& packageName,
	Listener* mCallback, PlatformAppStatus* status) {
	listener_ = mCallback;
	status_ = status;
	manager_->RemoveApp(mojo::String(packageName.utf8().data()),
		//sameThreadBindForMojo(&AppLauncherDispatcher::resultCodeCallback, this));
		createBaseCallback(bind<device::AppLauncher_ResultCodePtr>(&AppLauncherDispatcher::resultCodeCallback, this)));
}

void AppLauncherDispatcher::getAppList(const String& mimeType, Listener* mCallback,
	PlatformAppStatus* status) {
	listener_ = mCallback;
	status_ = status;
	manager_->GetAppList(mojo::String(mimeType.utf8().data()),
		//sameThreadBindForMojo(&AppLauncherDispatcher::resultCodeCallback, this));
		createBaseCallback(bind<device::AppLauncher_ResultCodePtr>(&AppLauncherDispatcher::resultCodeCallback, this)));
}

void AppLauncherDispatcher::getApplicationInfo(const String& packageName,
	const int flags, Listener* mCallback,	PlatformAppStatus* status) {
	listener_ = mCallback;
	status_ = status;
	manager_->GetApplicationInfo(mojo::String(packageName.utf8().data()), flags,
		//sameThreadBindForMojo(&AppLauncherDispatcher::resultCodeCallback, this));
		createBaseCallback(bind<device::AppLauncher_ResultCodePtr>(&AppLauncherDispatcher::resultCodeCallback, this)));
}

void AppLauncherDispatcher::resultCodeCallback(
		device::AppLauncher_ResultCodePtr resultCodePtr) {
	DCHECK(resultCodePtr);

	if (resultCodePtr != nullptr) {
		status_->setResultCode(resultCodePtr->resultCode);
		switch(resultCodePtr->functionCode) {
			case function::FUNC_GET_APP_LIST:
			{
				if (resultCodePtr->resultCode != code_list::SUCCESS){
					break;
				}
				unsigned listSize = resultCodePtr->applist.size();
				for(unsigned i = 0; i < listSize; i++) {
					PlatformApplicationInfo* data = listener_->getPlatformApplicationInfo();
					data->setName(String(resultCodePtr->applist[i]->name.data()));
					data->setClassName(String(resultCodePtr->applist[i]->className.data()));
					data->setDataDir(String(resultCodePtr->applist[i]->dataDir.data()));
					data->setEnabled(resultCodePtr->applist[i]->enabled);
					data->setFlags(resultCodePtr->applist[i]->flags);
					data->setPermission(String(resultCodePtr->applist[i]->permission.data()));
					data->setProcessName(String(resultCodePtr->applist[i]->processName.data()));
					data->setTargetSdkVersion(resultCodePtr->applist[i]->targetSdkVersion);
					data->setTheme(resultCodePtr->applist[i]->theme);
					data->setUid(resultCodePtr->applist[i]->uid);
					data->setPackageName(String(resultCodePtr->applist[i]->packageName.data()));
					listener_->appendAppList(data);
				}
				break;
			}
			case function::FUNC_GET_APPLICATION_INFO:
			{
				if (resultCodePtr->resultCode != code_list::SUCCESS) {
					break;
				}
				unsigned listSize = resultCodePtr->applist.size();
				if (listSize >= 1) {
					PlatformApplicationInfo* data = listener_->getPlatformApplicationInfo();
					data->setName(String(resultCodePtr->applist[0]->name.data()));
					data->setClassName(String(resultCodePtr->applist[0]->className.data()));
					data->setDataDir(String(resultCodePtr->applist[0]->dataDir.data()));
					data->setEnabled(resultCodePtr->applist[0]->enabled);
					data->setFlags(resultCodePtr->applist[0]->flags);
					data->setPermission(String(resultCodePtr->applist[0]->permission.data()));
					data->setProcessName(String(resultCodePtr->applist[0]->processName.data()));
					data->setTargetSdkVersion(resultCodePtr->applist[0]->targetSdkVersion);
					data->setTheme(resultCodePtr->applist[0]->theme);
					data->setUid(resultCodePtr->applist[0]->uid);
					data->setPackageName(String(resultCodePtr->applist[0]->packageName.data()));
					listener_->setApplicationInfo(data);
				}
				break;
			}
			default: {
				break;
			}
		}
	}
	if (listener_ != nullptr) {
		listener_->resultAppLauncher();
		status_ = nullptr;
	}
}

}  // namespace content
