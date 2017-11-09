// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef CalendarStatus_h
#define CalendarStatus_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "modules/calendar/CalendarInfo.h"

namespace blink {

class CalendarInfo;

class CalendarStatus final : public GarbageCollectedFinalized<CalendarStatus>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarStatus* create() {return new CalendarStatus();};
	~CalendarStatus();

	int resultCode();
	void setResultCode(int code);

	HeapVector<Member<CalendarInfo>>& calendarList();

	DEFINE_INLINE_TRACE() {
		visitor->Trace(mCalendarList);
	};

private:
	CalendarStatus();
	int mResultCode = -1;
	HeapVector<Member<CalendarInfo>> mCalendarList;
};

} // namespace blink

#endif // CalendarStatus_h
