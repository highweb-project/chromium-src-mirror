// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef CalendarInfo_h
#define CalendarInfo_h

#include "platform/heap/Handle.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/calendar/platform_calendarinfo.h"
#include "base/logging.h"

namespace blink {

class CalendarInfo final : public PlatformCalendarInfo, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	CalendarInfo() {}
	~CalendarInfo() {}

	DEFINE_INLINE_TRACE() {
		PlatformCalendarInfo::trace(visitor);
	};
};

} // namespace blink

#endif // CalendarInfo_h
