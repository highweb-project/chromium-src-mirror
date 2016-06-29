// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.storage;
import org.chromium.mojom.device.DeviceStorageManager;
import org.chromium.mojom.device.DeviceStorageResultCode;
import org.chromium.mojom.device.DeviceStorageStorageInfo;
import android.content.Context;
import java.lang.Exception;
import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;
import android.os.Environment;
import android.os.StatFs;
import java.io.BufferedReader;
import java.util.StringTokenizer;
import java.io.FileReader;
import android.os.Build;
import android.util.Log;
import java.util.Locale;

import org.chromium.mojo.system.MojoException;


/**
 * Android implementation of the devicestorage manager service defined in
 * device/storage/devicestorage_manager.mojom.
 */
public class DeviceStorageManagerImpl implements DeviceStorageManager {
	private static final String TAG = "chromium";
	private Context mContext;

	static class device_storage_ErrorCodeList{
		// Exception code
		static final int SUCCESS = 0;
		static final int FAILURE = -1;
		static final int NOT_ENABLED_PERMISSION = -2;
	};

	static class device_storage_function {
		static final int FUNC_GET_DEVICE_STORAGE = 1;
	};

	static class device_storage_type {
		static final int DEVICE_UNKNOWN = 1;
		static final int DEVICE_HARDDISK = 2;
		static final int DEVICE_FLOPPYDISK = 3;
		static final int DEVICE_OPTICAL = 4;
		//android storage type
		static final int DEVICE_INTERNAL = 5;
		static final int DEVICE_EXTERNAL = 6;
		static final int DEVICE_SDCARD = 7;
		static final int DEVICE_USB = 8;
	};


	public DeviceStorageManagerImpl(Context context) {
		mContext = context;
	}

	@Override
	public void close() {}

	@Override
	public void onConnectionError(MojoException e) {}

	@Override
	public void getDeviceStorage(GetDeviceStorageResponse callback) {
		DeviceStorageResultCode code = new DeviceStorageResultCode();
		code.resultCode = device_storage_ErrorCodeList.FAILURE;
		code.functionCode = device_storage_function.FUNC_GET_DEVICE_STORAGE;
		try {
			code.storageList = getStorageList();
			code.resultCode = device_storage_ErrorCodeList.SUCCESS;
		} catch(Exception e) {
			e.printStackTrace();
			code.resultCode = device_storage_ErrorCodeList.FAILURE;
			code.storageList = null;
		}
		callback.call(code);
		code = null;
	}

	public DeviceStorageStorageInfo[] getStorageList() throws Exception{
		List<DeviceStorageStorageInfo> list = new ArrayList<DeviceStorageStorageInfo>();
		List<String> pathList = new ArrayList<String>();
		try {
			String externalPath = Environment.getExternalStorageDirectory().getPath();
			boolean externalRemovable = Environment.isExternalStorageRemovable();
			boolean externalAvailable = Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)
										|| Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED_READ_ONLY);
			if (externalAvailable) {
				pathList.add(externalPath);
			}
			BufferedReader reader = null;
			try {
				reader = new BufferedReader(new FileReader("/proc/mounts"));
				String mountsLine;
				while((mountsLine = reader.readLine()) != null) {
					//sdcard file system - android default(fuse), samsung(sdcardfs)
					// proc/mounts "mountPosition mountPath FileSystem Flag...
					if (mountsLine.contains("sdcardfs") || mountsLine.contains("fuse")) {
						StringTokenizer token = new StringTokenizer(mountsLine, " ");
						String temp = token.nextToken();
						String mountPath = token.nextToken();
						if (pathList.contains(mountPath)) {
							continue;
						}
						//android default externalPath contains emulated
						//externalPath has 2 path - /storage/emulated/0, /storage/emulated/legacy
						if (mountPath.startsWith("/storage") && !mountPath.contains("emulated")) {
							pathList.add(mountPath);
						}
					}
				}
			} catch(Exception e) {
				e.printStackTrace();
				throw new Exception(e);
			} finally {
				if (reader != null) {
					try {
						reader.close();
					} catch(Exception e) {}
				}
			}
			for(int i = 0; i < pathList.size(); i++) {
				StatFs fs = new StatFs(pathList.get(i));
				DeviceStorageStorageInfo info = new DeviceStorageStorageInfo();
				if (externalPath.equals(pathList.get(i))) {
					info.type = device_storage_type.DEVICE_EXTERNAL;
					info.isRemovable = false;
				} else if (pathList.get(i).toLowerCase(Locale.getDefault()).contains("ext")) {
					info.type = device_storage_type.DEVICE_SDCARD;
					info.isRemovable = true;
				} else if (pathList.get(i).toLowerCase(Locale.getDefault()).contains("usb")) {
					info.type = device_storage_type.DEVICE_USB;
					info.isRemovable = true;
				} else {
					info.type = device_storage_type.DEVICE_UNKNOWN;
					info.isRemovable = false;
				}
				if (Build.VERSION.SDK_INT < 18) {
					info.capacity = (long)fs.getBlockCount() * (long)fs.getBlockSize();
					info.availableCapacity = (long)fs.getAvailableBlocks() * (long)fs.getBlockSize();
				} else {
					info.capacity = fs.getTotalBytes();
					info.availableCapacity = fs.getAvailableBytes();	
				}
				list.add(info);
			}
		} catch(Exception e) {
			e.printStackTrace();
			list.clear();
			throw new Exception(e);
		}
		return list.toArray(new DeviceStorageStorageInfo[list.size()]);
	}

}
