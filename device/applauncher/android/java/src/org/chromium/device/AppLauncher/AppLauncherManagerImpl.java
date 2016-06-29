// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.AppLauncher;
import org.chromium.mojom.device.AppLauncherManager;
import org.chromium.mojom.device.AppLauncherResultCode;
import org.chromium.mojom.device.AppLauncherApplicationInfo;
import android.content.Context;
import android.content.pm.PackageManager;
import android.util.Log;
import android.content.Intent;
import android.content.ComponentName;
import java.lang.Exception;
import android.net.Uri;
import android.content.pm.ApplicationInfo;
import java.util.List;
import java.util.Arrays;
import android.content.pm.ResolveInfo;
import java.util.ArrayList;

import org.chromium.mojo.system.MojoException;


/**
 * Android implementation of the applauncher manager service defined in
 * device/applauncher/applauncher_manager.mojom.
 */
public class AppLauncherManagerImpl implements AppLauncherManager {
	private static final String TAG = "chromium";
	private Context mContext;

	static class applauncher_code_list{
	// Exception code
		static final int SUCCESS = 0;
		static final int FAILURE = -1;
		static final int NOT_INSTALLED_APP = -2;
		static final int INVALID_PACKAGE_NAME = -3;
		static final int NOT_ENABLED_PERMISSION = -4;
		static final int INVALID_FLAGS = -5;
	};

	static class applauncher_function {
		static final int FUNC_LAUNCH_APP = 1;
		static final int FUNC_REMOVE_APP = 2;
		static final int FUNC_GET_APP_LIST = 3;
		static final int FUNC_GET_APPLICATION_INFO = 4;
	};

	public AppLauncherManagerImpl(Context context) {
		mContext = context;
	}

	@Override
	public void close() {}

	@Override
	public void onConnectionError(MojoException e) {}

	@Override
	public void launchApp(String packageName, String activityName, LaunchAppResponse callback) {
		AppLauncherResultCode code = new AppLauncherResultCode();
		code.resultCode = applauncher_code_list.FAILURE;
		code.functionCode = applauncher_function.FUNC_LAUNCH_APP;
		try {
			PackageManager pm = mContext.getPackageManager();
			Intent intent;
			if (activityName.isEmpty()) {
				intent = pm.getLaunchIntentForPackage(packageName);
			} else {
				intent = new Intent();
				intent.setComponent(new ComponentName(packageName, activityName));
				intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			}
			if(intent == null) {
				code.resultCode = applauncher_code_list.NOT_INSTALLED_APP;
			}
			else {
				mContext.startActivity(intent);
				code.resultCode = applauncher_code_list.SUCCESS;	
			}
		} catch(Exception e) {
			e.printStackTrace();
		}
		callback.call(code);
		code = null;
	}

	@Override
	public void removeApp(String packageName, RemoveAppResponse callback) {
		AppLauncherResultCode code = new AppLauncherResultCode();
		code.resultCode = applauncher_code_list.FAILURE;
		code.functionCode = applauncher_function.FUNC_REMOVE_APP;
		try {
			Intent intent = new Intent(Intent.ACTION_DELETE);
			intent.setData(Uri.parse("package:" + packageName));
			intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			mContext.startActivity(intent);
			code.resultCode = applauncher_code_list.SUCCESS;
		} catch(Exception e) {
			e.printStackTrace();
		}
		callback.call(code);
		code = null;
	}

	@Override
	public void getAppList(String mimeType, GetAppListResponse callback) {
		AppLauncherResultCode code = new AppLauncherResultCode();
		code.resultCode = applauncher_code_list.FAILURE;
		code.functionCode = applauncher_function.FUNC_GET_APP_LIST;
		try {
			PackageManager pm = mContext.getPackageManager();
			List<ApplicationInfo> data = new ArrayList<ApplicationInfo>();
			if (!mimeType.isEmpty()) {
				Intent intent = new Intent(Intent.ACTION_DEFAULT);
				intent.addCategory(Intent.CATEGORY_LAUNCHER);
				intent.setType(mimeType);
				List<ResolveInfo> list = pm.queryIntentActivities(intent, 0);
				for(int i = 0; i < list.size(); i++) {
					data.add(list.get(i).activityInfo.applicationInfo);
				}
			}
			else {
				List<ApplicationInfo> packages = pm.getInstalledApplications(PackageManager.GET_META_DATA);
				for(int i = 0; i < packages.size(); i++) {
					if (packages.get(i) != null && pm.getLaunchIntentForPackage(packages.get(i).packageName) != null) {
						data.add(packages.get(i));
					}
				}
			}
			code.applist = new AppLauncherApplicationInfo[data.size()];
			for(int i = 0; i < data.size(); i++) {
				code.applist[i] = setData(data.get(i), pm);
			}
			code.resultCode = applauncher_code_list.SUCCESS;
			data.clear();
			data = null;
		}catch(Exception e) {
			e.printStackTrace();
		}
		callback.call(code);
		if (code.applist != null)
			Arrays.fill(code.applist, null);
		code.applist = null;
		code = null;
	}

	@Override
	public void getApplicationInfo(String packageName, int flags, GetApplicationInfoResponse callback) {
		AppLauncherResultCode code = new AppLauncherResultCode();
		code.resultCode = applauncher_code_list.FAILURE;
		code.functionCode = applauncher_function.FUNC_GET_APPLICATION_INFO;
		try {
			PackageManager pm = mContext.getPackageManager();
			List<ApplicationInfo> data = new ArrayList<ApplicationInfo>();

			data.add(pm.getApplicationInfo(packageName, flags));

			code.applist = new AppLauncherApplicationInfo[data.size()];
			for(int i = 0; i < data.size(); i++) {
				code.applist[i] = setData(data.get(i), pm);
			}
			code.resultCode = applauncher_code_list.SUCCESS;
			data.clear();
			data = null;
		}catch(PackageManager.NameNotFoundException e) {
			e.printStackTrace();
			code.resultCode = applauncher_code_list.NOT_INSTALLED_APP;
		} catch(Exception e) {
			e.printStackTrace();
			code.resultCode = applauncher_code_list.FAILURE;
		}
		callback.call(code);
		if (code.applist != null)
			Arrays.fill(code.applist, null);
		code.applist = null;
		code = null;
	}

	public AppLauncherApplicationInfo setData(ApplicationInfo info, PackageManager pm) {
		AppLauncherApplicationInfo appinfo = new AppLauncherApplicationInfo();
		appinfo.name = pm.getApplicationLabel(info).toString();
		appinfo.className = info.className;
		appinfo.dataDir = info.dataDir;
		appinfo.enabled = info.enabled;
		appinfo.flags = info.flags;
		appinfo.permission = info.permission;
		appinfo.processName = info.processName;
		appinfo.targetSdkVersion = info.targetSdkVersion;
		appinfo.theme = info.theme;
		appinfo.uid = info.uid;
		appinfo.packageName = info.packageName;
		if (!(appinfo.name != null && !appinfo.name.isEmpty())) {
			appinfo.name = "";
		}
		if (!(appinfo.className != null && !appinfo.className.isEmpty())) {
			appinfo.className = "";
		}
		if (!(appinfo.dataDir != null && !appinfo.dataDir.isEmpty())) {
			appinfo.dataDir = "";
		}
		if (!(appinfo.permission != null && !appinfo.permission.isEmpty())) {
			appinfo.permission = "";
		}
		if (!(appinfo.processName != null && !appinfo.processName.isEmpty())) {
			appinfo.processName = "";
		}
		if (!(appinfo.packageName != null && !appinfo.packageName.isEmpty())) {
			appinfo.packageName = "";
		}
		return appinfo;
	}

}
