// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AppLauncher_h
#define AppLauncher_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/WTFString.h"
#include "platform/wtf/Deque.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h"
#include "public/platform/modules/device_api/WebDeviceApiApplauncherClient.h"

namespace content {
	struct DeviceApiApplauncherRequestResult;
}

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
		kInvalidAppId = -3,
		kNotEnabledPermission = -4,
		kNotSupportApi = 9999,
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
		functionData(int code) {
			functionCode = code;
		}
		void setString(String& str, String data) {
			if (data.IsEmpty()) {
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

	void launchApp(const String appId, AppLauncherScriptCallback* callback);
	void launchApp();

	void removeApp(const String appId, AppLauncherScriptCallback* callback);
	void removeApp();

	void getAppList(AppLauncherScriptCallback* callback);
	void getAppList();

	void getApplicationInfo(const String appId, AppLauncherScriptCallback* callback);
	void getApplicationInfo();

	void notifyCallback(AppStatus*, AppLauncherScriptCallback*);
	void notifyError(int, AppLauncherScriptCallback*);
	void continueFunction();

	void requestPermission(int operationType);
	void onPermissionChecked(PermissionResult result);

	void onRequestResult(WebDeviceApiApplauncherClient::WebDeviceApiApplauncherRequestResult result);

	DECLARE_TRACE();

private:
	AppLauncher(LocalFrame& frame);

	Deque<functionData*> d_functionData;
	//20160419-jphong
	HeapVector<Member<AppLauncherScriptCallback>> mCallbackList;

	WTF::String mOrigin;
	WebDeviceApiPermissionCheckClient* mClient;
	WebDeviceApiApplauncherClient* mApplauncherClient;

	bool ViewPermissionState = false;
	bool DeletePermissionState = false;
};

} // namespace blink

#endif // AppLauncher_h
