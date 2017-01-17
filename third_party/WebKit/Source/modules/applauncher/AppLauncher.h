// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AppLauncher_h
#define AppLauncher_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "wtf/Vector.h"
#include "wtf/text/WTFString.h"
#include "wtf/Deque.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"

#include "device/applauncher/applauncher_manager.mojom-blink.h"
#include "device/applauncher/applauncher_ResultCode.mojom-blink.h"

namespace blink {

class LocalFrame;
class AppStatus;
class AppLauncherScriptCallback;
class ApplicationInfo;

class AppLauncher
	: public GarbageCollectedFinalized<AppLauncher>
	, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:

	enum code_list{
		// Exception code
		kSuccess = 0,
		kFailure = -1,
		kNotInstalledApp = -2,
		kInvalidPackageName = -3,
		kNotEnabledPermission = -4,
		kInvalidFlags = -5,
		kNotSupportApi = 9999,

		//getApplicationInfo flag
		kFlagGetMetaData = 128,
		kFlagSharedLibraryFiles = 1024,
		kFlagGetUninstalledPackages = 8192,
	};

	enum function {
		FUNC_LAUNCH_APP = 1,
		FUNC_REMOVE_APP,
		FUNC_GET_APP_LIST,
		FUNC_GET_APPLICATION_INFO,
	};

	struct functionData {
		int functionCode = -1;
		//20160419-jphong
		//AppLauncherScriptCallback* scriptCallback = nullptr;
		String str1;
		String str2;
		int int1;
		functionData(int code) {
			functionCode = code;
		}
		void setString(String& str, String data) {
			if (data.isEmpty()) {
				str = "";
			} else {
				str = data;
			}
		}
	};

	static AppLauncher* create(LocalFrame& frame) {
		return new AppLauncher(frame);
	}
	virtual ~AppLauncher();

	void launchApp(const String packageName, const String activityName, AppLauncherScriptCallback* callback);
	void launchApp();

	void removeApp(const String packageName, AppLauncherScriptCallback* callback);
	void removeApp();

	void getAppList(const String mimeType, AppLauncherScriptCallback* callback);
	void getAppList();

	void getApplicationInfo(const String packageName, const int flags, AppLauncherScriptCallback* callback);
	void getApplicationInfo();

	void notifyCallback(AppStatus*, AppLauncherScriptCallback*);
	void notifyError(int, AppLauncherScriptCallback*);
	void continueFunction();

	void requestPermission(int operationType);
	void onPermissionChecked(PermissionResult result);

	DECLARE_TRACE();

private:
	AppLauncher(LocalFrame& frame);

	void mojoResultCallback(device::blink::AppLauncher_ResultCodePtr result);

	Deque<functionData*> d_functionData;
	//20160419-jphong
	HeapVector<Member<AppLauncherScriptCallback>> mCallbackList;

	WTF::String mOrigin;
	WebDeviceApiPermissionCheckClient* mClient;

	bool ViewPermissionState = false;
	bool DeletePermissionState = false;

	device::blink::AppLauncherManagerPtr appManager;
};

} // namespace blink

#endif // AppLauncher_h
