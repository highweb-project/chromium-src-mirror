// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarEventFilter_h
#define CalendarEventFilter_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"

namespace blink {

class CalendarEventFilter final : public GarbageCollectedFinalized<CalendarEventFilter>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarEventFilter* create() { return new CalendarEventFilter(); };
	~CalendarEventFilter() {};

	String startBefore() const { return mStartBefore; };
	void setStartBefore(const String& startBefore) { mStartBefore = startBefore; };

	String startAfter() const { return mStartAfter; };
	void setStartAfter(const String& startAfter) { mStartAfter = startAfter; };

	String endBefore() const { return mEndBefore; };
	void setEndBefore(const String& endBefore) { mEndBefore = endBefore; };

	String endAfter() const { return mEndAfter; };
	void setEndAfter(const String& endAfter) { mEndAfter = endAfter; };

	DEFINE_INLINE_VIRTUAL_TRACE() {}

private:
	CalendarEventFilter() {};

	String mStartBefore;
	String mStartAfter;
	String mEndBefore;
	String mEndAfter;
};

} // namespace blink

#endif // CalendarEventFilter_h
