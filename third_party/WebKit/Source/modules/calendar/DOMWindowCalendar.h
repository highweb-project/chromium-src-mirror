// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOMWindowCalendar_h
#define DOMWindowCalendar_h

#include "core/frame/DOMWindowProperty.h"
#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"
#include "Calendar.h"

namespace blink {

class Calendar;
class DOMWindow;
class ExecutionContext;

class DOMWindowCalendar final : public GarbageCollected<DOMWindowCalendar>, public Supplement<LocalDOMWindow>, public DOMWindowProperty {
    USING_GARBAGE_COLLECTED_MIXIN(DOMWindowCalendar);
public:
    static DOMWindowCalendar& from(LocalDOMWindow&);
    static Calendar* calendar(DOMWindow&);
    Calendar* calendarCreate(ExecutionContext* context) const;

    DECLARE_TRACE();

private:
    explicit DOMWindowCalendar(LocalDOMWindow&);
    static const char* supplementName();

    mutable Member<Calendar> m_calendar;
};

} // namespace blink

#endif // DOMWindowCrypto_h
