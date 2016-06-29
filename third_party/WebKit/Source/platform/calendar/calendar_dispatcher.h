// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLINK_PLATFORM_CALENDAR_CALENDAR_DISPATCHER_H_
#define BLINK_PLATFORM_CALENDAR_CALENDAR_DISPATCHER_H_

#include "platform/PlatformExport.h"
#include "platform/heap/Handle.h"
#include "device/calendar/calendar_ResultCode.mojom.h"
#include "device/calendar/calendar_manager.mojom.h"

namespace blink {

class PlatformCalendarInfo;
class PlatformCalendarStatus;

class PLATFORM_EXPORT CalendarDispatcher : public GarbageCollectedFinalized<CalendarDispatcher> {
	public:
    class PLATFORM_EXPORT Listener {
    public:
      virtual ~Listener() = default;
      virtual void resultCalendar() = 0;
			virtual void appendCalendarList(PlatformCalendarInfo* data) = 0;
			virtual PlatformCalendarInfo* getPlatformCalendarInfo() = 0;
    };

    enum function {
  		FUNC_FIND_EVENT = 1,
  		FUNC_ADD_EVENT,
  		FUNC_UPDATE_EVENT,
  		FUNC_DELETE_EVENT,
  	};

		explicit CalendarDispatcher();
		virtual ~CalendarDispatcher();

		void resultCodeCallback(device::Calendar_ResultCodePtr resultCode);

		void calendarDeviceApiFindEvent(const String& startBefore, const String& startAfter, const String& endBefore,
      const String& endAfter, bool multiple, Listener* callback, PlatformCalendarStatus* status);
		void calendarDeviceApiAddEvent(PlatformCalendarInfo* event, Listener* callback, PlatformCalendarStatus* status);
		void calendarDeviceApiUpdateEvent(PlatformCalendarInfo* event, Listener* callback, PlatformCalendarStatus* status);
		void calendarDeviceApiDeleteEvent(const String& id, Listener* callback, PlatformCalendarStatus* status);

    DECLARE_VIRTUAL_TRACE();

	private:
		device::CalendarManagerPtr manager_;
		Listener* listener_;
		Member<PlatformCalendarStatus> status_;

		//DISALLOW_COPY_AND_ASSIGN(CalendarDispatcher);
};

}  // namespace content

#endif  // BLINK_PLATFORM_CALENDAR_CALENDAR_DISPATCHER_H_
