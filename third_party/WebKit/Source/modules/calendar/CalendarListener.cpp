// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "CalendarListener.h"
#include "CalendarStatus.h"
#include "public/platform/Platform.h"
#include "Calendar.h"
#include "CalendarInfo.h"
#include "wtf/text/WTFString.h"
#include "base/logging.h"

namespace blink {

CalendarListener* CalendarListener::instance(Calendar* calendar)
{
	CalendarListener* callback = new CalendarListener(calendar);
	return callback;
}

CalendarListener::CalendarListener(Calendar* calendar)
{
	mCalendar = calendar;
	mCalendarStatus = CalendarStatus::create();
}

CalendarListener::~CalendarListener()
{
	mCalendarStatus = NULL;
	mCalendar = NULL;
}

DEFINE_TRACE(CalendarListener)
{
	visitor->trace(mCalendarStatus);
	visitor->trace(mCalendar);
}

void CalendarListener::resultCalendar()
{
	DLOG(INFO) << "CalendarListener::resultCalendar";
	mCalendar->resultCodeCallback();
}

void CalendarListener::appendCalendarList(PlatformCalendarInfo* data) {
	if (mCalendarStatus != nullptr) {
		mCalendarStatus->calendarList().append((CalendarInfo*)data);
	}
}

PlatformCalendarInfo* CalendarListener::getPlatformCalendarInfo() {
	return (PlatformCalendarInfo*)(new CalendarInfo());
}


CalendarStatus* CalendarListener::lastData()
{
	if (mCalendarStatus != nullptr)
		return mCalendarStatus.get();
	else
		return nullptr;
}

} // namespace blink
