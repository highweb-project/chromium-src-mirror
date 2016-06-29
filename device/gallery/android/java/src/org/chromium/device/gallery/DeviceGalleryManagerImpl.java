// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.gallery;
import org.chromium.mojom.device.DeviceGalleryManager;
import org.chromium.mojom.device.DeviceGalleryResultCode;
import org.chromium.mojom.device.MojoDeviceGalleryFindOptions;
import org.chromium.mojom.device.MojoDeviceGalleryMediaObject;
import org.chromium.mojom.device.MojoDeviceGalleryMediaContent;
import android.content.Context;
import java.lang.Exception;
import android.util.Log;
import java.io.IOException;
import android.content.ContentResolver;
import android.database.Cursor;
import java.util.ArrayList;
import java.util.List;
import android.provider.MediaStore;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import android.Manifest;
import android.content.pm.PackageManager;

import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;
import android.net.Uri;
import android.content.ContentUris;

import org.chromium.mojo.system.MojoException;


/**
 * Android implementation of the devicegallery manager service defined in
 * device/gallery/devicegallery_manager.mojom.
 */
public class DeviceGalleryManagerImpl implements DeviceGalleryManager {
	private static final String TAG = "chromium";
	private Context mContext;

	static class device_gallery_ErrorCodeList{
		// Exception code
		static final int SUCCESS = -1;
		static final int NOT_ENABLED_PERMISSION = -2;
		static final int UNKNOWN_ERROR = 0;
		static final int INVALID_ARGUMENT_ERROR = 1;
		static final int TIMEOUT_ERROR = 3;
		static final int PENDING_OPERATION_ERROR = 4;
		static final int IO_ERROR = 5;
		static final int NOT_SUPPORTED_ERROR = 6;
		static final int MEDIA_SIZE_EXCEEDED = 20;
	};

	static class device_gallery_function {
		static final int FUNC_FIND_MEDIA = 1;
		static final int FUNC_GET_MEDIA = 2;
		static final int FUNC_DELETE_MEDIA = 3;
	};

	public DeviceGalleryManagerImpl(Context context) {
		mContext = context;
	}

	@Override
	public void close() {}

	@Override
	public void onConnectionError(MojoException e) {}

	@Override
	public void findMedia(MojoDeviceGalleryFindOptions options, FindMediaResponse callback) {
		DeviceGalleryResultCode code = new DeviceGalleryResultCode();
		code.resultCode = device_gallery_ErrorCodeList.UNKNOWN_ERROR;
		code.functionCode = device_gallery_function.FUNC_FIND_MEDIA;

		ArrayList<String> folderList = new ArrayList<String>();
		ContentResolver resolver = mContext.getContentResolver();
		String[] folderColumn = { "distinct replace("+MediaStore.Images.Media.DATA+", "+ MediaStore.Images.Media.DISPLAY_NAME+", '')"};
		Cursor folderListCursor = resolver.query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, folderColumn, null, null, null);

		if (folderListCursor != null && folderListCursor.getCount() > 0) {
			folderListCursor.moveToFirst();
			while(!folderListCursor.isAfterLast()) {
				folderList.add(folderListCursor.getString(0));
				folderListCursor.moveToNext();
			}
		}

		String where = MediaStore.Images.Media.DATA + " like ?";
		String operationString;
		String optionString = "";
		if (options.mOperation.isEmpty()) {
			operationString = "";
		} else if (options.mOperation.compareToIgnoreCase("and") == 0 || options.mOperation.compareToIgnoreCase("or") == 0) {
			operationString = " " + options.mOperation + " ";
		} else {
			operationString = "";
		}
		try {
			if (operationString.length() > 0) {
				if (!options.mObject.mType.isEmpty()) {
					optionString += operationString + MediaStore.Images.Media.MIME_TYPE + " like \'%" + options.mObject.mType + "\'";
				}
				if (!options.mObject.mDescription.isEmpty()) {
					optionString += operationString + MediaStore.Images.Media.DESCRIPTION + " like \'" + options.mObject.mDescription + "\'";
				}
				if (!options.mObject.mId.isEmpty()) {
					optionString += operationString + MediaStore.Images.Media._ID + " like \'" + options.mObject.mId + "\'";
				}
				if (!options.mObject.mTitle.isEmpty()) {
					optionString += operationString + MediaStore.Images.Media.TITLE + " like \'%" + options.mObject.mTitle + "%\'";
				}
				if (!options.mObject.mFileName.isEmpty()) {
					optionString += operationString + MediaStore.Images.Media.DISPLAY_NAME + " like \'%" + options.mObject.mFileName + "%\'";
				}
				if (options.mObject.mFileSize != 0) {
					optionString += operationString + MediaStore.Images.Media.SIZE + " like \'" + options.mObject.mFileSize + "\'";
				}
				if (options.mObject.mCreatedDate != 0) {
					optionString += operationString + MediaStore.Images.Media.DATE_TAKEN + " like \'" + options.mObject.mCreatedDate + "\'";
				}
				if (options.mObject.mContent != null && !options.mObject.mContent.mUri.isEmpty()) {
					optionString += operationString + MediaStore.Images.Media.DATA + " like \'" + options.mObject.mContent.mUri + "\'";
				}
				optionString = optionString.replaceFirst(operationString, "");
				if (optionString.length() > 0) {
					where = "(" + where + ") and (" + optionString + ")";
				}
			}
		} catch(Exception e) {
			e.printStackTrace();
			where = MediaStore.Images.Media.DATA + " like ?";
		}

		List<MojoDeviceGalleryMediaObject> objectArray = new ArrayList<MojoDeviceGalleryMediaObject>();
		try{
			for(int i = 0; i < folderList.size(); i++) {
				String whereArg[] = {folderList.get(i) + "%"};
				Cursor imageListCursor = resolver.query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, where, whereArg, null);
				if (imageListCursor != null && imageListCursor.getCount() > 0) {
					imageListCursor.moveToFirst();
					while(!imageListCursor.isAfterLast()) {
						MojoDeviceGalleryMediaObject object = new MojoDeviceGalleryMediaObject();
						object.mType = imageListCursor.getString(imageListCursor.getColumnIndex(MediaStore.Images.Media.MIME_TYPE));
						object.mDescription = imageListCursor.getString(imageListCursor.getColumnIndex(MediaStore.Images.Media.DESCRIPTION));
						object.mId = imageListCursor.getString(imageListCursor.getColumnIndex(MediaStore.Images.Media._ID));
						object.mTitle = imageListCursor.getString(imageListCursor.getColumnIndex(MediaStore.Images.Media.TITLE));
						object.mFileName = imageListCursor.getString(imageListCursor.getColumnIndex(MediaStore.Images.Media.DISPLAY_NAME));
						object.mFileSize = imageListCursor.getInt(imageListCursor.getColumnIndex(MediaStore.Images.Media.SIZE));
						object.mCreatedDate = imageListCursor.getDouble(imageListCursor.getColumnIndex(MediaStore.Images.Media.DATE_TAKEN));
						object.mContent = new MojoDeviceGalleryMediaContent();
						object.mContent.mUri = imageListCursor.getString(imageListCursor.getColumnIndex(MediaStore.Images.Media.DATA));
						object.mContent.mBlob = new byte[0];
						object.mContent.mBlobSize = 0;

						if (object.mType == null) {
							object.mType = "";
						}
						if (object.mDescription == null) {
							object.mDescription = "";
						}
						if (object.mId == null) {
							object.mId = "";
						}
						if (object.mTitle == null) {
							object.mTitle = "";
						}
						if (object.mFileName == null) {
							object.mFileName = "";
						}
						if (object.mContent.mUri == null) {
							object.mContent.mUri = "";
						}

						objectArray.add(object);
						if (options.mMaxItem > 0 && objectArray.size() == options.mMaxItem) {
							break;
						}
						imageListCursor.moveToNext();
					}
					imageListCursor.close();
				}
				if (options.mMaxItem > 0 && objectArray.size() == options.mMaxItem) {
					break;
				}
			}
			code.resultCode = device_gallery_ErrorCodeList.SUCCESS;
			code.mediaList = new MojoDeviceGalleryMediaObject[objectArray.size()];
			code.mediaListSize = objectArray.size();
			if (objectArray.size() > 0) {
				for(int i = 0; i < objectArray.size(); i++) {
					code.mediaList[i] = objectArray.get(i);
				}
			}
		} catch(Exception e) {
			e.printStackTrace();
			objectArray.clear();
			code.mediaListSize = 0;
			code.resultCode = device_gallery_ErrorCodeList.INVALID_ARGUMENT_ERROR;
			code.mediaList = new MojoDeviceGalleryMediaObject[0];
		}
		callback.call(code);
		code = null;
	}

	@Override
	public void getMedia(MojoDeviceGalleryMediaObject object, GetMediaResponse callback) {
		DeviceGalleryResultCode code = new DeviceGalleryResultCode();
		code.resultCode = device_gallery_ErrorCodeList.TIMEOUT_ERROR;
		code.resultCode = device_gallery_ErrorCodeList.SUCCESS;
		code.functionCode = device_gallery_function.FUNC_GET_MEDIA;
		code.mediaListSize = 1;
		code.mediaList = new MojoDeviceGalleryMediaObject[1];
		code.mediaList[0] = object;
		File f = new File(object.mContent.mUri);
		try {
			ByteArrayOutputStream bout = new ByteArrayOutputStream();
			FileInputStream fin = new FileInputStream(f);
			byte[] buf = new byte[1024];
			int read = 0;
			while((read = fin.read(buf, 0, buf.length)) != -1) {
				bout.write(buf, 0, read);
			}
			fin.close();
			object.mContent.mBlob = bout.toByteArray();
			object.mContent.mBlobSize = bout.size();
		} catch(Exception e) {
			e.printStackTrace();
		}
		callback.call(code);
		code = null;
	}

	@Override
	public void deleteMedia(MojoDeviceGalleryMediaObject object, DeleteMediaResponse callback) {
		DeviceGalleryResultCode code = new DeviceGalleryResultCode();
		code.functionCode = device_gallery_function.FUNC_DELETE_MEDIA;
		code.resultCode = device_gallery_ErrorCodeList.UNKNOWN_ERROR;
		code.mediaListSize = 1;
		code.mediaList = new MojoDeviceGalleryMediaObject[1];
		code.mediaList[0] = object;
		int checkPermission = mContext.checkCallingOrSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
		if (checkPermission != PackageManager.PERMISSION_GRANTED) {
			Log.d(TAG, "permission not granted");
			code.resultCode = device_gallery_ErrorCodeList.NOT_ENABLED_PERMISSION;
		} else {
			File f = new File(object.mContent.mUri);
			boolean result = false;
			boolean deleteMSResult = false;
			if (f.exists()) {
				result = f.delete();
				if (result) {
					deleteMSResult = deleteFromMediaScanner(object.mContent.mUri);
				}
			}
			if (result && deleteMSResult) {
				code.resultCode = device_gallery_ErrorCodeList.SUCCESS;
			} else {
				code.resultCode = device_gallery_ErrorCodeList.IO_ERROR;
			}
		}
		callback.call(code);
		code = null;
	}

	private boolean deleteFromMediaScanner(String filePath) {
		boolean result = false;
		try {
			Uri fileUri = Uri.parse(filePath);
			filePath = fileUri.getPath();

			Cursor c = mContext.getContentResolver().query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, MediaStore.Images.Media.DATA + "='" + filePath + "'", null, null);
			c.moveToNext();
			int id = c.getInt(0);
			Uri uri = ContentUris.withAppendedId(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, id);

			result = mContext.getContentResolver().delete(uri, null, null) == 1;
			c.close();
		} catch(Exception e) {
			e.printStackTrace();
			result = false;
		}
		return result;
	}
}