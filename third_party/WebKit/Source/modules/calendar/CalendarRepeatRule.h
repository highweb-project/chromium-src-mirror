// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarRepeatRule_h
#define CalendarRepeatRule_h

#include "platform/heap/Handle.h"
#include "wtf/Forward.h"
#include "wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "bindings/core/v8/ScriptWrappable.h"

namespace blink {

class CalendarRepeatRule final : public GarbageCollectedFinalized<CalendarRepeatRule>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarRepeatRule* create() { return new CalendarRepeatRule(); };
	~CalendarRepeatRule() {};

	String frequency() const { return mFrequency; };
	void setFrequency(const String& frequency) { mFrequency = frequency; };

	String expires() const { return mExpires; };
	void setExpires(const String& expires) { mExpires = expires; };

	unsigned int interval(const bool& isNull) const { return mInterval; };
	void setInterval(const bool& interval) { mInterval = interval; };

	Vector<String> exceptionDates() const { return mExceptionDates; };
	void setExceptionDates(Vector<String>& exceptionDates) { mExceptionDates = exceptionDates; };

	Vector<int> daysInWeek() const { return mDaysInWeek; };
	void setDaysInWeek(Vector<int>& daysInWeek) { mDaysInWeek = daysInWeek; };

	Vector<int> daysInMonth() const { return mDaysInMonth; };
	void setDaysInMonth(Vector<int>& daysInMonth) { mDaysInMonth = daysInMonth; };

	Vector<int> daysInYear() const { return mDaysInYear; };
	void setDaysInYear(Vector<int>& daysInYear) { mDaysInYear = daysInYear; };

	Vector<int> weeksInMonth() const { return mWeeksInMonth; };
	void setWeeksInMonth(Vector<int>& weeksInMonth) { mWeeksInMonth = weeksInMonth; };

	Vector<int> monthsInYear() const { return mMonthsInYear; };
	void setMonthsInYear(Vector<int>& monthsInYear) { mMonthsInYear = monthsInYear; };

	DEFINE_INLINE_TRACE() {
	}

private:
	CalendarRepeatRule() {};

	String mFrequency;
	String mExpires;
	unsigned int mInterval;
	Vector<String> mExceptionDates;
	Vector<int> mDaysInWeek;
	Vector<int> mDaysInMonth;
	Vector<int> mDaysInYear;
	Vector<int> mWeeksInMonth;
	Vector<int> mMonthsInYear;
};

} // namespace blink

#endif // CalendarRepeatRule_h
