// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "modules/applauncher/AppLauncher.h"

// #include "base/basictypes.h"
#include "base/bind.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "public/platform/WebString.h"
#include "AppStatus.h"
#include "AppLauncherScriptCallback.h"
#include "ApplicationInfo.h"

#include "modules/device_api/DeviceApiPermissionController.h"
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h"

#include "platform/mojo/MojoHelper.h"
#include "public/platform/Platform.h"
#include "public/platform/InterfaceProvider.h"

namespace blink {

AppLauncher::AppLauncher(LocalFrame& frame)
	: mClient(DeviceApiPermissionController::from(frame)->client())
{
	mOrigin = frame.document()->url().strippedForUseAsReferrer();
	mClient->SetOrigin(mOrigin.latin1().data());
	d_functionData.clear();
}

AppLauncher::~AppLauncher()
{
	mCallbackList.clear();
	d_functionData.clear();

	if (appManager.is_bound()) {
		appManager.reset();
	}
}

void AppLauncher::launchApp(const String packageName, const String activityName, AppLauncherScriptCallback* callback) {
	functionData* data = new functionData(function::FUNC_LAUNCH_APP);
	//20160419-jphong
	//data->scriptCallback = callback;
	mCallbackList.append(callback);
	data->setString(data->str1, packageName);
	data->setString(data->str2, activityName);
	d_functionData.append(data);
	data = nullptr;

	if (packageName.isEmpty()) {
		notifyError(code_list::kInvalidPackageName, callback);
		return;
	}
	if (d_functionData.size() == 1) {
		launchApp();
	}
}

void AppLauncher::launchApp() {
	if (!ViewPermissionState) {
		requestPermission(PermissionOptType::VIEW);
		return;
	}
	functionData* data = d_functionData.first();

	if (!appManager.is_bound()) {
		Platform::current()->interfaceProvider()->getInterface(mojo::GetProxy(&appManager));
	}
	appManager->LaunchApp(data->str1, data->str2,
		convertToBaseCallback(WTF::bind(&AppLauncher::mojoResultCallback, wrapPersistent(this))));
	data = nullptr;
}

void AppLauncher::removeApp(const String packageName, AppLauncherScriptCallback* callback) {
	functionData* data = new functionData(function::FUNC_REMOVE_APP);
	//20160419-jphong
	//data->scriptCallback = callback;
	mCallbackList.append(callback);
	data->setString(data->str1, packageName);
	d_functionData.append(data);
	data = nullptr;

	if (packageName.isEmpty()) {
		notifyError(code_list::kInvalidPackageName, callback);
		return;
	}
	if (d_functionData.size() == 1) {
		removeApp();
	}
}

void AppLauncher::removeApp() {
	if (!DeletePermissionState) {
		requestPermission(PermissionOptType::DELETE);
		return;
	}
	functionData* data = d_functionData.first();
	if (!appManager.is_bound()) {
		Platform::current()->interfaceProvider()->getInterface(mojo::GetProxy(&appManager));
	}
	appManager->RemoveApp(data->str1,
		convertToBaseCallback(WTF::bind(&AppLauncher::mojoResultCallback, wrapPersistent(this))));
	data = nullptr;
}

void AppLauncher::getAppList(const String mimeType, AppLauncherScriptCallback* callback) {
	functionData* data = new functionData(function::FUNC_GET_APP_LIST);
	//20160419-jphong
	//data->scriptCallback = callback;
	mCallbackList.append(callback);
	data->setString(data->str1, mimeType);
	d_functionData.append(data);
	data = nullptr;
	if (d_functionData.size() == 1) {
		getAppList();
	}
}

void AppLauncher::getAppList() {
	if (!ViewPermissionState) {
		requestPermission(PermissionOptType::VIEW);
		return;
	}
	functionData* data = d_functionData.first();

	if (!appManager.is_bound()) {
		Platform::current()->interfaceProvider()->getInterface(mojo::GetProxy(&appManager));
	}
	appManager->GetAppList(data->str1,
		convertToBaseCallback(WTF::bind(&AppLauncher::mojoResultCallback, wrapPersistent(this))));
	data = nullptr;
}

void AppLauncher::getApplicationInfo(const String packageName, const int flags, AppLauncherScriptCallback* callback) {
	functionData* data = new functionData(function::FUNC_GET_APPLICATION_INFO);
	//20160419-jphong
	//data->scriptCallback = callback;
	mCallbackList.append(callback);
	data->setString(data->str1, packageName);
	data->int1 = flags;
	d_functionData.append(data);
	data = nullptr;
	if (packageName.isEmpty()) {
		notifyError(code_list::kInvalidPackageName, callback);
		return;
	}
	if (flags != 0) {
		int checkFlag = flags & code_list::kFlagGetMetaData;
		checkFlag |= flags & code_list::kFlagSharedLibraryFiles;
		checkFlag |= flags & code_list::kFlagGetUninstalledPackages;
		if (checkFlag <= 0) {
			notifyError(code_list::kInvalidFlags, callback);
		}
	}
	if (d_functionData.size() == 1) {
		getApplicationInfo();
	}
}

void AppLauncher::getApplicationInfo() {
	if (!ViewPermissionState) {
		requestPermission(PermissionOptType::VIEW);
		return;
	}
	functionData* data = d_functionData.first();

	if (!appManager.is_bound()) {
		Platform::current()->interfaceProvider()->getInterface(mojo::GetProxy(&appManager));
	}
	appManager->GetApplicationInfo(data->str1, data->int1,
		convertToBaseCallback(WTF::bind(&AppLauncher::mojoResultCallback, wrapPersistent(this))));
	data = nullptr;
}

void AppLauncher::mojoResultCallback(device::blink::AppLauncher_ResultCodePtr result) {
	DCHECK(result);
	AppStatus* status = AppStatus::create();
	if (!result.is_null()) {
		status->setResultCode(result->resultCode);
		switch(result->functionCode) {
			case function::FUNC_GET_APP_LIST:
			{
				if (result->resultCode != code_list::kSuccess){
					break;
				}
				if (!result->applist.has_value()) {
					break;
				}
				unsigned listSize = result->applist->size();
				for(unsigned i = 0; i < listSize; i++) {
					ApplicationInfo* data = new ApplicationInfo();
					data->setName(result->applist.value()[i]->name);
					data->setClassName(result->applist.value()[i]->className);
					data->setDataDir(result->applist.value()[i]->dataDir);
					data->setEnabled(result->applist.value()[i]->enabled);
					data->setFlags(result->applist.value()[i]->flags);
					data->setPermission(result->applist.value()[i]->permission);
					data->setProcessName(result->applist.value()[i]->processName);
					data->setTargetSdkVersion(result->applist.value()[i]->targetSdkVersion);
					data->setTheme(result->applist.value()[i]->theme);
					data->setUid(result->applist.value()[i]->uid);
					data->setPackageName(result->applist.value()[i]->packageName);
					status->appList().append(data);
				}
				break;
			}
			case function::FUNC_GET_APPLICATION_INFO:
			{
				if (result->resultCode != code_list::kSuccess) {
					break;
				}
				if (!result->applist.has_value()) {
					break;
				}
				unsigned listSize = result->applist.value().size();
				if (listSize >= 1) {
					ApplicationInfo* data = new ApplicationInfo();
					data->setName(result->applist.value()[0]->name);
					data->setClassName(result->applist.value()[0]->className);
					data->setDataDir(result->applist.value()[0]->dataDir);
					data->setEnabled(result->applist.value()[0]->enabled);
					data->setFlags(result->applist.value()[0]->flags);
					data->setPermission(result->applist.value()[0]->permission);
					data->setProcessName(result->applist.value()[0]->processName);
					data->setTargetSdkVersion(result->applist.value()[0]->targetSdkVersion);
					data->setTheme(result->applist.value()[0]->theme);
					data->setUid(result->applist.value()[0]->uid);
					data->setPackageName(result->applist.value()[0]->packageName);
					status->setAppInfo(data);
				}
				break;
			}
			default: {
				break;
			}
		}
	}
	notifyCallback(status, mCallbackList.at(0).get());
}

void AppLauncher::notifyCallback(AppStatus* status, AppLauncherScriptCallback* callback) {
	if (callback != NULL) {
		callback->handleEvent(status);
		callback = nullptr;
	}
	if (d_functionData.size() > 0)
		d_functionData.removeFirst();
	//20160419-jphong
	if(mCallbackList.size() > 0)
		mCallbackList.remove(0);
	continueFunction();
}

void AppLauncher::notifyError(int errorCode, AppLauncherScriptCallback* callback) {
	AppStatus* status = AppStatus::create();
	status->setResultCode(errorCode);
	notifyCallback(status, callback);
}

void AppLauncher::continueFunction() {
	if (d_functionData.size() > 0) {
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_LAUNCH_APP: {
				launchApp();
				break;
			}
			case function::FUNC_REMOVE_APP: {
				removeApp();
				break;
			}
			case function::FUNC_GET_APP_LIST: {
				getAppList();
				break;
			}
			case function::FUNC_GET_APPLICATION_INFO: {
				getApplicationInfo();
				break;
			}
			default: {
				break;
			}
		}
	}
}

void AppLauncher::requestPermission(int operationType) {
	if(mClient) {
		switch(operationType) {
			case PermissionOptType::VIEW: {
				mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
						PermissionAPIType::APPLICATION_LAUNCHER,
						PermissionOptType::VIEW,
						base::Bind(&AppLauncher::onPermissionChecked, base::Unretained(this))));
				break;
			}
			case PermissionOptType::DELETE: {
				mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
						PermissionAPIType::APPLICATION_LAUNCHER,
						PermissionOptType::DELETE,
						base::Bind(&AppLauncher::onPermissionChecked, base::Unretained(this))));
				break;
			}
		}
	}
}

void AppLauncher::onPermissionChecked(PermissionResult result)
{
	if (result == WebDeviceApiPermissionCheckClient::DeviceApiPermissionRequestResult::RESULT_OK && d_functionData.size() > 0) {
		switch(d_functionData.first()->functionCode) {
			case function::FUNC_LAUNCH_APP:
			case function::FUNC_GET_APP_LIST:
			case function::FUNC_GET_APPLICATION_INFO: {
				ViewPermissionState = true;
				if (ViewPermissionState) {
					continueFunction();
					return;
				}
				break;
			}
			case function::FUNC_REMOVE_APP: {
				DeletePermissionState = true;
				if (DeletePermissionState) {
					continueFunction();
					return;
				}
				break;
			}
		}
	}
	notifyError(code_list::kNotEnabledPermission, mCallbackList.at(0).get());
}

DEFINE_TRACE(AppLauncher)
{
	visitor->trace(mCallbackList);
}

} // namespace blink
