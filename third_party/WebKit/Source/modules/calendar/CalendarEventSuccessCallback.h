// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarEventSuccessCallback_h
#define CalendarEventSuccessCallback_h

#include "wtf/text/WTFString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "CalendarStatus.h"

namespace blink {

class CalendarEventSuccessCallback : public GarbageCollectedFinalized<CalendarEventSuccessCallback>{
public:
	virtual ~CalendarEventSuccessCallback() { }
	virtual void handleEvent(CalendarStatus* status) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif // CalendarEventSuccessCallback_h
