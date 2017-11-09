// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "modules/calendar/DOMWindowCalendar.h"

#include "core/frame/LocalDOMWindow.h"

namespace blink {

DOMWindowCalendar::DOMWindowCalendar(LocalDOMWindow& window)
{
}

const char* DOMWindowCalendar::SupplementName()
{
    return "DOMWindowCalendar";
}

DOMWindowCalendar& DOMWindowCalendar::From(LocalDOMWindow& window)
{
	DOMWindowCalendar* supplement = static_cast<DOMWindowCalendar*>(Supplement<LocalDOMWindow>::From(window, SupplementName()));
    if (!supplement) {
        supplement = new DOMWindowCalendar(window);
        ProvideTo(window, SupplementName(), supplement);
    }
    return *supplement;
}

Calendar* DOMWindowCalendar::calendar(DOMWindow& window)
{
	return DOMWindowCalendar::From(ToLocalDOMWindow(window)).calendarCreate(window);
}

Calendar* DOMWindowCalendar::calendarCreate(DOMWindow& window) const
{
    if (!m_calendar && window.GetFrame() && window.IsLocalDOMWindow())
    	m_calendar = Calendar::create((LocalFrame*)window.GetFrame());
    return m_calendar.Get();
}

DEFINE_TRACE(DOMWindowCalendar)
{
	visitor->Trace(m_calendar);
	Supplement<LocalDOMWindow>::Trace(visitor);
}

} // namespace blink
