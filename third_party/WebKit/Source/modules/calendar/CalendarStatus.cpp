// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "wtf/build_config.h"
#include "CalendarStatus.h"

namespace blink {

int CalendarStatus::resultCode() {
	return mResultCode;
}

void CalendarStatus::setResultCode(int code) {
	mResultCode = code;
}

HeapVector<Member<CalendarInfo>>& CalendarStatus::calendarList() {
	return mCalendarList;
}

CalendarStatus::~CalendarStatus() {
	mCalendarList.clear();
}

CalendarStatus::CalendarStatus() {
}

} // namespace blink
