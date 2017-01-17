// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CalendarEventErrorCallback_h
#define CalendarEventErrorCallback_h

#include "wtf/text/WTFString.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "CalendarError.h"

namespace blink {

class CalendarEventErrorCallback : public GarbageCollectedFinalized<CalendarEventErrorCallback>{
public:
	virtual ~CalendarEventErrorCallback() { }
	virtual void handleEvent(CalendarError* error) = 0;
	DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}

#endif // CalendarEventErrorCallback_h
