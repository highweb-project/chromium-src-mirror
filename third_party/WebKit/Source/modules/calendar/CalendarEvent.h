// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarEvent_h
#define CalendarEvent_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "CalendarRepeatRule.h"

namespace blink {

class CalendarEvent final : public GarbageCollectedFinalized<CalendarEvent>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarEvent* Create() { return new CalendarEvent(); };
	~CalendarEvent() {};

	String id() const { return mId; };
	void setId(const String& id) { mId = id; };

	String description() const { return mDescription; };
	void setDescription(const String& description) { mDescription = description; };

	String location() const { return mLocation; };
	void setLocation(const String& location) { mLocation = location; };

	String summary() const { return mSummary; };
	void setSummary(const String& summary) { mSummary = summary; };

	String start() const { return mStart; };
	void setStart(const String& start) { mStart = start; };

	String end() const { return mEnd; };
	void setEnd(const String& end) { mEnd = end; };

	String status() const { return mStatus; };
	void setStatus(const String& status) { mStatus = status; };

	String transparency() const { return mTransparency; };
	void setTransparency(const String& transparency) { mTransparency = transparency; };

	String reminder() const { return mReminder; };
	void setReminder(const String& reminder) { mReminder = reminder; };

	CalendarRepeatRule* recurrence() const { return mRecurrence; };
	void setRecurrence(CalendarRepeatRule* recurrence) { mRecurrence = recurrence; };

	DEFINE_INLINE_TRACE() {
		visitor->Trace(mRecurrence);
	}

private:
	CalendarEvent() {};

	String mId;
	String mDescription;
	String mLocation;
	String mSummary;
	String mStart;
	String mEnd;
	String mStatus;
	String mTransparency;
	String mReminder;
	Member<CalendarRepeatRule> mRecurrence;
};

} // namespace blink

#endif // CalendarEvent_h
