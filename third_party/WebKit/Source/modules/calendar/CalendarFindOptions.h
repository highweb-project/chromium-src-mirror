// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarFindOptions_h
#define CalendarFindOptions_h

#include "platform/heap/Handle.h"
#include "wtf/Forward.h"
#include "wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "CalendarEventFilter.h"
#include "bindings/modules/v8/V8CalendarEventFilter.h"

namespace blink {

class CalendarFindOptions final : public GarbageCollectedFinalized<CalendarFindOptions>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarFindOptions* create() { return new CalendarFindOptions(); };
	~CalendarFindOptions() { mFilter = nullptr; };

	bool multiple(bool& isNull) const { return mMultiple; };
	void setMultiple(const bool multiple) { mMultiple = multiple; };

	CalendarEventFilter* filter() const { return mFilter; };
	void setFilter(CalendarEventFilter* filter) { mFilter = filter; };

	DEFINE_INLINE_TRACE() {
		visitor->trace(mFilter);
	}

private:
	CalendarFindOptions() {};

	bool mMultiple = false;
	Member<CalendarEventFilter> mFilter = CalendarEventFilter::create();
};

} // namespace blink

#endif // CalendarFindOptions_h
