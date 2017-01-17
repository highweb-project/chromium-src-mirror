// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef CalendarInfo_h
#define CalendarInfo_h

#include "platform/heap/Handle.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "wtf/text/WTFString.h"
#include "base/logging.h"

namespace blink {

class CalendarInfo final : public GarbageCollectedFinalized<CalendarInfo>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	CalendarInfo() {}
	~CalendarInfo() {}

	String id() {
		return mId;
	}
	String description() {
		return mDescription;
	}
	String location() {
		return mLocation;
	}
	String summary() {
		return mSummary;
	}
	String start() {
		return mStart;
	}
	String end() {
		return mEnd;
	}
	String status() {
		return mStatus;
	}
	String transparency() {
		return mTransparency;
	}
	String reminder() {
		return mReminder;
	}

	void setId(String value) {
		mId = value;
	}
	void setDescription(String value) {
		mDescription = value;
	}
	void setLocation(String value) {
		mLocation = value;
	}
	void setSummary(String value) {
		mSummary = value;
	}
	void setStart(String value) {
		mStart = value;
	}
	void setEnd(String value) {
		mEnd = value;
	}
	void setStatus(String value) {
		mStatus = value;
	}
	void setTransparency(String value) {
		mTransparency = value;
	}
	void setReminder(String value) {
		mReminder = value;
	}

	DEFINE_INLINE_TRACE() {};

private:
	String mId = "";
	String mDescription = "";
	String mLocation = "";
	String mSummary = "";
	String mStart = "";
	String mEnd = "";
	String mStatus = "";
	String mTransparency = "";
	String mReminder = "";
};

} // namespace blink

#endif // CalendarInfo_h
