// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarListener_h
#define CalendarListener_h

#include "wtf/text/WTFString.h"
#include "platform/calendar/calendar_dispatcher.h"

namespace blink {

class CalendarStatus;
class Calendar;
class PlatformCalendarInfo;

class CalendarListener final : public GarbageCollectedFinalized<CalendarListener>, public CalendarDispatcher::Listener {
public:
	static CalendarListener* instance(Calendar*);
	virtual ~CalendarListener();

	void resultCalendar() override;
	void appendCalendarList(PlatformCalendarInfo* data) override;
	PlatformCalendarInfo* getPlatformCalendarInfo() override;

	CalendarStatus* lastData();

	DECLARE_VIRTUAL_TRACE();

private:
	CalendarListener(Calendar*);
	Member<CalendarStatus> mCalendarStatus;
	Member<Calendar> mCalendar;
};

} // namespace blink

#endif // CalendarListener_h
