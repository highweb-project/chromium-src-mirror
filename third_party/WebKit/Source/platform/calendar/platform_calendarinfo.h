// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLINK_PLATFORM_CALENDAR_PLATFORM_CALENDARINFO_H_
#define BLINK_PLATFORM_CALENDAR_PLATFORM_CALENDARINFO_H_

#include "platform/PlatformExport.h"
#include "platform/heap/Handle.h"
#include "wtf/text/WTFString.h"

namespace blink {

class PLATFORM_EXPORT PlatformCalendarInfo : public GarbageCollectedFinalized<PlatformCalendarInfo> {
public:

  PlatformCalendarInfo();
	virtual ~PlatformCalendarInfo();

  String id();
	String description();
	String location();
	String summary();
	String start();
	String end();
	String status();
	String transparency();
	String reminder();

  void setId(String value);
  void setDescription(String value);
  void setLocation(String value);
  void setSummary(String value);
  void setStart(String value);
  void setEnd(String value);
  void setStatus(String value);
  void setTransparency(String value);
  void setReminder(String value);

	DECLARE_VIRTUAL_TRACE();

private:
  String mId = "";
	String mDescription = "";
	String mLocation = "";
	String mSummary = "";
	String mStart = "";
	String mEnd = "";
	String mStatus = "";
	String mTransparency = "";
	String mReminder = "";
};

} // namespace blink

#endif // BLINK_PLATFORM_CALENDAR_PLATFORM_CALENDARINFO_H_
