// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarRepeatRule_h
#define CalendarRepeatRule_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"

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
	void setInterval(bool interval, bool isNull) {
		if (isNull) {
			interval = false;
		} else {
			mInterval = interval; 
		}
	};

	Vector<String> exceptionDates() const { return mExceptionDates; };
	void setExceptionDates(Vector<String>& exceptionDates) { mExceptionDates = exceptionDates; };

	Vector<int16_t> daysInWeek() const { return mDaysInWeek; };
	void setDaysInWeek(Vector<int16_t>& daysInWeek) { mDaysInWeek = daysInWeek; };

	Vector<int16_t> daysInMonth() const { return mDaysInMonth; };
	void setDaysInMonth(Vector<int16_t>& daysInMonth) { mDaysInMonth = daysInMonth; };

	Vector<int16_t> daysInYear() const { return mDaysInYear; };
	void setDaysInYear(Vector<int16_t>& daysInYear) { mDaysInYear = daysInYear; };

	Vector<int16_t> weeksInMonth() const { return mWeeksInMonth; };
	void setWeeksInMonth(Vector<int16_t>& weeksInMonth) { mWeeksInMonth = weeksInMonth; };

	Vector<int16_t> monthsInYear() const { return mMonthsInYear; };
	void setMonthsInYear(Vector<int16_t>& monthsInYear) { mMonthsInYear = monthsInYear; };

	DEFINE_INLINE_TRACE() {
	}

private:
	CalendarRepeatRule() {};

	String mFrequency;
	String mExpires;
	unsigned int mInterval;
	Vector<String> mExceptionDates;
	Vector<int16_t> mDaysInWeek;
	Vector<int16_t> mDaysInMonth;
	Vector<int16_t> mDaysInYear;
	Vector<int16_t> mWeeksInMonth;
	Vector<int16_t> mMonthsInYear;
};

} // namespace blink

#endif // CalendarRepeatRule_h
