// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef CalendarStatus_h
#define CalendarStatus_h

#include "platform/heap/Handle.h"
#include "wtf/Forward.h"
#include "wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "modules/calendar/CalendarInfo.h"
#include "platform/calendar/platform_calendarstatus.h"

namespace blink {

class CalendarInfo;

class CalendarStatus final : public PlatformCalendarStatus, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarStatus* create() {return new CalendarStatus();};
	virtual ~CalendarStatus();

	int resultCode();

	HeapVector<Member<CalendarInfo>>& calendarList();

	DEFINE_INLINE_TRACE() {
		visitor->trace(mCalendarList);
		PlatformCalendarStatus::trace(visitor);
	};

private:
	CalendarStatus();
	HeapVector<Member<CalendarInfo>> mCalendarList;
};

} // namespace blink

#endif // CalendarStatus_h
