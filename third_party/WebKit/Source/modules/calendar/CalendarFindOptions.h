// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarFindOptions_h
#define CalendarFindOptions_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "CalendarEventFilter.h"
#include "bindings/modules/v8/V8CalendarEventFilter.h"

namespace blink {

class CalendarFindOptions final : public GarbageCollectedFinalized<CalendarFindOptions>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static CalendarFindOptions* Create() { return new CalendarFindOptions(); };
	~CalendarFindOptions() { mFilter = nullptr; };

	bool multiple(bool& isNull) const { return mMultiple; };
	void setMultiple(bool multiple, bool isNull) { 
		if (isNull) {
			multiple = false;
		} else {
			mMultiple = multiple; 
		}
	};

	CalendarEventFilter* filter() const { return mFilter; };
	void setFilter(CalendarEventFilter* filter) { mFilter = filter; };

	DEFINE_INLINE_TRACE() {
		visitor->Trace(mFilter);
	}

private:
	CalendarFindOptions() {};

	bool mMultiple = false;
	Member<CalendarEventFilter> mFilter = CalendarEventFilter::create();
};

} // namespace blink

#endif // CalendarFindOptions_h
