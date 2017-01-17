// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "modules/calendar/DOMWindowCalendar.h"

#include "core/frame/LocalDOMWindow.h"

namespace blink {

DOMWindowCalendar::DOMWindowCalendar(LocalDOMWindow& window)
    : DOMWindowProperty(window.frame())
{
}

const char* DOMWindowCalendar::supplementName()
{
    return "DOMWindowCalendar";
}

DOMWindowCalendar& DOMWindowCalendar::from(LocalDOMWindow& window)
{
	DOMWindowCalendar* supplement = static_cast<DOMWindowCalendar*>(Supplement<LocalDOMWindow>::from(window, supplementName()));
    if (!supplement) {
        supplement = new DOMWindowCalendar(window);
        provideTo(window, supplementName(), supplement);
    }
    return *supplement;
}

Calendar* DOMWindowCalendar::calendar(DOMWindow& window)
{
	return DOMWindowCalendar::from(toLocalDOMWindow(window)).calendarCreate(window.getExecutionContext());
}

Calendar* DOMWindowCalendar::calendarCreate(ExecutionContext* context) const
{
    if (!m_calendar && frame())
    	m_calendar = Calendar::create(context, frame());
    return m_calendar.get();
}

DEFINE_TRACE(DOMWindowCalendar)
{
	visitor->trace(m_calendar);
	Supplement<LocalDOMWindow>::trace(visitor);
	DOMWindowProperty::trace(visitor);
}

} // namespace blink
