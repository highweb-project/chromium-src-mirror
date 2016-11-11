// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.Calendar;
import org.chromium.mojom.device.CalendarManager;
import org.chromium.mojom.device.CalendarResultCode;
import org.chromium.mojom.device.CalendarCalendarInfo;
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

import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import android.database.Cursor;
import android.text.TextUtils;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.provider.CalendarContract.Events;
import android.content.ContentUris;

import org.chromium.mojo.system.MojoException;

/**
 * Android implementation of the calendar manager service defined in
 * device/calendar/calendar_manager.mojom.
 */
public class CalendarManagerImpl implements CalendarManager {
	private static final String TAG = "chromium";
	private Context mContext;

	static class code_list{
	// Exception code
		static final int SUCCESS = 0;
		static final int FAILURE = -1;
		static final int NOT_INSTALLED_APP = -2;
		static final int INVALID_PACKAGE_NAME = -3;
		static final int NOT_ENABLED_PERMISSION = -4;
		static final int INVALID_FLAGS = -5;
		static final int NOT_SUPPORT_API = 9999;
	};

	static class function {
		static final int FUNC_FIND_EVENT = 1;
		static final int FUNC_ADD_EVENT = 2;
		static final int FUNC_UPDATE_EVENT = 3;
		static final int FUNC_DELETE_EVENT = 4;
	};

	public CalendarManagerImpl(Context context) {
		Log.d(TAG, "CalendarManagerImpl");
		mContext = context;
	}

	@Override
	public void close() {}

	@Override
	public void onConnectionError(MojoException e) {}

	@Override
	public void calendarDeviceApiUpdateEvent(CalendarCalendarInfo event, CalendarDeviceApiUpdateEventResponse callback) {
		Log.d(TAG, "CalendarManagerImpl, calendarDeviceApiUpdateEvent, event : " + event);
		Log.d(TAG, "id : " + event.id + ", description : " + event.description + ", location : " + event.location + ", summary : " + event.summary + ", start : " + event.start + ", end : " + event.end);

		callback.call(s_CalendarDeviceApiUpdateEvent(mContext, event.id, event.description, event.location, event.summary, event.start, event.end));
	}

	@Override
	public void calendarDeviceApiAddEvent(CalendarCalendarInfo event, CalendarDeviceApiAddEventResponse callback) {
		Log.d(TAG, "CalendarManagerImpl, calendarDeviceApiAddEvent, event : " + event);
		Log.d(TAG, "description : " + event.description + ", location : " + event.location + ", summary : " + event.summary + ", start : " + event.start + ", end : " + event.end);

		callback.call(s_CalendarDeviceApiAddEvent(mContext, event.description, event.location, event.summary, event.start, event.end));
	}

	@Override
	public void calendarDeviceApiDeleteEvent(String id, CalendarDeviceApiDeleteEventResponse callback) {
		Log.d(TAG, "CalendarManagerImpl, calendarDeviceApiDeleteEvent, id : " + id);

		callback.call(s_CalendarDeviceApiDeleteEvent(mContext, id));
	}

	@Override
	public void calendarDeviceApiFindEvent(String startBefore, String startAfter, String endBefore, String endAfter, boolean multiple, CalendarDeviceApiFindEventResponse callback) {
		Log.d(TAG, "CalendarManagerImpl, calendarDeviceApiFindEvent, startBefore : " + startBefore + ", startAfter : " + startAfter + ", endBefore : " + endBefore + ", endAfter : " + endAfter + ", multiple : " + multiple);
		callback.call(s_CalendarDeviceApiFindEvent(mContext, startBefore, startAfter, endBefore, endAfter, multiple));
	}

	public static CalendarResultCode s_CalendarDeviceApiDeleteEvent(Context context, String id) {
		long eventID = TextUtils.isEmpty(id) || id.equals("NaN")  ? 0 : Long.parseLong(id);
		Log.d(TAG, "s_CalendarDeviceApiDeleteEvent, eventID : " + eventID);

		CalendarResultCode code = new CalendarResultCode();
		code.resultCode = 0;
		code.functionCode = function.FUNC_DELETE_EVENT;

		if (eventID != 0) {
			Uri deleteUri = null;
			deleteUri = ContentUris.withAppendedId(Events.CONTENT_URI, eventID);
			int rows = context.getContentResolver().delete(deleteUri, null,
					null);
			Log.i(TAG, "Rows deleted: " + rows);

			code.resultCode = rows;
		}

		return code;
	}

	public static CalendarResultCode s_CalendarDeviceApiUpdateEvent(Context context,
			String id, String description, String location, String summary,
			String start, String end) {

		long startMillis = TextUtils.isEmpty(start) || start.equals("NaN")  ? 0 : Long.parseLong(start);
		long endMillis = TextUtils.isEmpty(end) || end.equals("NaN")  ? 0 : Long.parseLong(end);

		Log.d(TAG, "s_CalendarDeviceApiUpdateEvent, id : " + id
				+ ", description : " + description + ", location : " + location
				+ ", summary : " + summary + ", startMillis : " + startMillis
				+ ", endMillis : " + endMillis);

		long eventId = TextUtils.isEmpty(id) || id.equals("NaN") ? 0 : Long.parseLong(id);

		ContentResolver cr = context.getContentResolver();
		ContentValues values = new ContentValues();
		Uri updateUri = null;

		if (TextUtils.isEmpty(description) == false) {
			values.put(Events.DESCRIPTION, description);
		}

		if (TextUtils.isEmpty(location) == false) {
			values.put(Events.EVENT_LOCATION, location);
		}

		if (TextUtils.isEmpty(summary) == false) {
			values.put(Events.TITLE, summary);
		}

		if (startMillis != 0) {
			values.put(Events.DTSTART, startMillis);
		}

		if (endMillis != 0) {
			values.put(Events.DTEND, endMillis);
		}

		updateUri = ContentUris.withAppendedId(Events.CONTENT_URI, eventId);
		int rows = context.getContentResolver().update(updateUri, values, null,
				null);
		Log.i(TAG, "Rows updated: " + rows);

		CalendarResultCode code = new CalendarResultCode();
		code.resultCode = (int)eventId;//rows;
		code.functionCode = function.FUNC_UPDATE_EVENT;

		return code;
	}

	public static CalendarResultCode s_CalendarDeviceApiAddEvent(Context context,
			String description, String location, String summary, String start,
			String end) {
		long startMillis = TextUtils.isEmpty(start) || start.equals("NaN")  ? 0 : Long.parseLong(start);
		long endMillis = TextUtils.isEmpty(end) || end.equals("NaN")  ? 0 : Long.parseLong(end);

		Log.d(TAG, "s_CalendarDeviceApiAddEvent, description : " + description
				+ ", location : " + location + ", summary : " + summary
				+ ", startMillis : " + startMillis + ", endMillis : "
				+ endMillis);

		long calID = 1; // There is no suitable interface

		ContentResolver cr = context.getContentResolver();
		ContentValues values = new ContentValues();
		values.put(Events.DTSTART, startMillis);
		values.put(Events.DTEND, endMillis);
		values.put(Events.TITLE, summary);
		values.put(Events.DESCRIPTION, description);
		values.put(Events.EVENT_LOCATION, location);
		values.put(Events.CALENDAR_ID, calID);
		values.put(Events.EVENT_TIMEZONE, "Asia/Seoul");
		Uri uri = cr.insert(Events.CONTENT_URI, values);

		// get the event ID that is the last element in the Uri
		long eventID = Long.parseLong(uri.getLastPathSegment());
		Log.d(TAG, "eventID : " + eventID);

		CalendarResultCode code = new CalendarResultCode();
		code.resultCode = (int)eventID;
		code.functionCode = function.FUNC_ADD_EVENT;

		return code;
	}

	public static CalendarResultCode s_CalendarDeviceApiFindEvent(Context context,
			String startBefore, String startAfter, String endBefore,
			String endAfter, boolean multiple) {

		CalendarResultCode code = new CalendarResultCode();
		code.resultCode = 0;
		code.functionCode = function.FUNC_FIND_EVENT;

		long sb = TextUtils.isEmpty(startBefore) || startBefore.equals("NaN") ? 0 : Long.parseLong(startBefore);//convertDateStringToTimestamp(startBefore);
		long sa = TextUtils.isEmpty(startAfter) || startAfter.equals("NaN")  ? 0 : Long.parseLong(startAfter);//convertDateStringToTimestamp(startAfter);
		long eb = TextUtils.isEmpty(endBefore) || endBefore.equals("NaN")  ? 0 : Long.parseLong(endBefore);//convertDateStringToTimestamp(endBefore);
		long ea = TextUtils.isEmpty(endAfter) || endAfter.equals("NaN")  ? 0 : Long.parseLong(endAfter);//convertDateStringToTimestamp(endAfter);

		Log.d(TAG, "sb : " + sb + ", sa : " + sa + ", eb : " + eb + ", ea : "
				+ ea);

		String[] projection = { Events._ID, Events.DESCRIPTION, Events.EVENT_LOCATION, Events.TITLE,
				Events.DTSTART, Events.DTEND, Events.STATUS, Events.VISIBLE, Events.HAS_ALARM };

		StringBuffer selectionBuffer = new StringBuffer();

		boolean hasBeforeToken = false;
		if (sb != 0) {
			selectionBuffer.append(Events.DTSTART + " < " + sb);
			hasBeforeToken = true;
		}
		if (sa != 0) {
			if (hasBeforeToken) {
				selectionBuffer.append(" and ");
			}

			selectionBuffer.append(Events.DTSTART + " > " + sa);
			hasBeforeToken = true;
		}
		if (eb != 0) {
			if (hasBeforeToken) {
				selectionBuffer.append(" and ");
			}

			selectionBuffer.append(Events.DTEND + " < " + eb);
			hasBeforeToken = true;
		}
		if (ea != 0) {
			selectionBuffer.append(Events.DTEND + " > " + ea);
		}

		String selection = TextUtils.isEmpty(selectionBuffer.toString()) ? null
				: selectionBuffer.toString();
		Log.d(TAG, "selection : " + selection);

		String sortOrder = Events.DTSTART + " ASC, " + Events.TITLE + " DESC";

		Cursor cursor = context.getContentResolver().query(
				Events.CONTENT_URI, projection,
				selection, null, sortOrder);
		cursor.moveToFirst();

		int dataSize = 1;
		if(multiple) {
			dataSize = cursor.getCount();
		}

		code.calendarlist = new CalendarCalendarInfo[dataSize];
		//Arrays.fill(code.calendarlist, new CalendarCalendarInfo());
		Log.d(TAG, "dataSize : " + dataSize + ", code.calendarlist : " + code.calendarlist);

		for (int i = 0; i < cursor.getCount(); i++) {
			StringBuffer buffer = new StringBuffer();
			String[] columnNames = cursor.getColumnNames();

			for (String columnName : columnNames) {
				int columnIndex = cursor.getColumnIndex(columnName);
				String columnData = cursor.getString(columnIndex);
				buffer.append(columnName + "(" + columnIndex + ") : " + columnData + " / ");
			}
			Log.d(TAG, buffer.toString());
			Log.d(TAG, "code.calendarlist[" + i + "] : " + code.calendarlist[i]);

			code.calendarlist[i] = new CalendarCalendarInfo();
			code.calendarlist[i].id = TextUtils.isEmpty(cursor.getString(0)) ? "" : cursor.getString(0);
			code.calendarlist[i].description = TextUtils.isEmpty(cursor.getString(1)) ? "" : cursor.getString(1);
			code.calendarlist[i].location = TextUtils.isEmpty(cursor.getString(2)) ? "" : cursor.getString(2);
			code.calendarlist[i].summary = TextUtils.isEmpty(cursor.getString(3)) ? "" : cursor.getString(3);
			code.calendarlist[i].start = TextUtils.isEmpty(cursor.getString(4)) ? "" : cursor.getString(4);
			code.calendarlist[i].end = TextUtils.isEmpty(cursor.getString(5)) ? "" : cursor.getString(5);
			code.calendarlist[i].status = TextUtils.isEmpty(cursor.getString(6)) ? "" : cursor.getString(6);
			code.calendarlist[i].transparency = TextUtils.isEmpty(cursor.getString(7)) ? "" : cursor.getString(7);
			code.calendarlist[i].reminder = TextUtils.isEmpty(cursor.getString(8)) ? "" : cursor.getString(8);

			Log.d(TAG, "java start : " + code.calendarlist[i].start);

			cursor.moveToNext();

			if(multiple == false) {
				break;
			}
		}

		if (cursor != null) {
			cursor.close();
			cursor = null;
		}

		return code;
	}

	public static long convertDateStringToTimestamp(String dateString) {
		long ts = 0;

		try {
			if(TextUtils.isEmpty(dateString) == false) {
				SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd");
				Date parsedDate = dateFormat.parse(dateString);
				Timestamp timestamp = new java.sql.Timestamp(parsedDate.getTime());
				ts = timestamp.getTime();
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return ts;
	}

	public static String getDate(long milliSeconds) {
		SimpleDateFormat formatter = new SimpleDateFormat(
				"dd/MM/yyyy hh:mm:ss a");
		Calendar calendar = Calendar.getInstance();
		calendar.setTimeInMillis(milliSeconds);
		return formatter.format(calendar.getTime());
	}
}
