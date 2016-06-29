// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLINK_PLATFORM_APPLAUNCHER_APPLAUNCHER_DISPATCHER_H_
#define BLINK_PLATFORM_APPLAUNCHER_APPLAUNCHER_DISPATCHER_H_

#include "platform/PlatformExport.h"
#include "platform/heap/Handle.h"
#include "device/applauncher/applauncher_ResultCode.mojom.h"
#include "device/applauncher/applauncher_manager.mojom.h"

namespace blink {

class PlatformAppStatus;
class PlatformApplicationInfo;

class PLATFORM_EXPORT AppLauncherDispatcher : public GarbageCollectedFinalized<AppLauncherDispatcher> {
	public:
    class PLATFORM_EXPORT Listener {
    public:
      virtual ~Listener() = default;
      virtual void resultAppLauncher() = 0;
			virtual void appendAppList(PlatformApplicationInfo* data) = 0;
			virtual void setApplicationInfo(PlatformApplicationInfo* info) = 0;
			virtual PlatformApplicationInfo* getPlatformApplicationInfo() = 0;
    };

		explicit AppLauncherDispatcher();
		virtual ~AppLauncherDispatcher();

		enum function {
			FUNC_LAUNCH_APP = 1,
			FUNC_REMOVE_APP,
			FUNC_GET_APP_LIST,
			FUNC_GET_APPLICATION_INFO,
		};

		enum code_list{
			// Exception code
			SUCCESS = 0,
			FAILURE = -1,
			NOT_INSTALLED_APP = -2,
			INVALID_PACKAGE_NAME = -3,
			NOT_ENABLED_PERMISSION = -4,
			INVALID_FLAGS = -5,

			//getApplicationInfo flag
			FLAG_GET_META_DATA = 128,
			FLAG_SHARED_LIBRARY_FILES = 1024,
			FLAG_GET_UNINSTALLED_PACKAGES = 8192,
		};

		void launchApp(const String& packageName, const String& activityName, Listener* mCallback, PlatformAppStatus* status);
		void removeApp(const String& packageName, Listener* mCallback, PlatformAppStatus* status);
		void getAppList(const String& mimeType, Listener* mCallback, PlatformAppStatus* status);
		void getApplicationInfo(const String& packageName, const int flags, Listener* mCallback, PlatformAppStatus* status);
		void resultCodeCallback(device::AppLauncher_ResultCodePtr resultCode);
		DECLARE_VIRTUAL_TRACE();
	private:
		device::AppLauncherManagerPtr manager_;
		Listener* listener_;

		Member<PlatformAppStatus> status_;

		//DISALLOW_COPY_AND_ASSIGN(AppLauncherDispatcher);
};

}  // namespace content

#endif  // BLINK_PLATFORM_APPLAUNCHER_APPLAUNCHER_DISPATCHER_H_
