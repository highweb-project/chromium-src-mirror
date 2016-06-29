// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//#include "platform/threading/BindForMojo.h"
#include "platform/mojo/MojoHelper.h"
#include "public/platform/Platform.h"
#include "wtf/text/WTFString.h"

#include "calendar_dispatcher.h"
#include "platform_calendarinfo.h"
#include "platform_calendarstatus.h"
#include "public/platform/ServiceRegistry.h"

namespace blink {

CalendarDispatcher::CalendarDispatcher() {
	Platform::current()->serviceRegistry()->connectToRemoteService(mojo::GetProxy(&manager_));
}

CalendarDispatcher::~CalendarDispatcher() {
	manager_.reset();
	listener_ = nullptr;
	status_ = nullptr;
}

DEFINE_TRACE(CalendarDispatcher) {
  visitor->trace(status_);
}

void CalendarDispatcher::calendarDeviceApiAddEvent(PlatformCalendarInfo* event, Listener* callback, PlatformCalendarStatus* status) {
	DLOG(INFO) << "CalendarDispatcher::calendarDeviceApiAddEvent, manager_ : " << manager_;
	status_ = status;
	device::Calendar_CalendarInfoPtr ptr(device::Calendar_CalendarInfo::New());

  ptr.get()->id = mojo::String(event->id().utf8().data());
	ptr.get()->description = mojo::String(event->description().utf8().data());
	ptr.get()->location = mojo::String(event->location().utf8().data());
	ptr.get()->summary = mojo::String(event->summary().utf8().data());
	ptr.get()->start = mojo::String(event->start().utf8().data());
	ptr.get()->end = mojo::String(event->end().utf8().data());
	ptr.get()->status = mojo::String(event->status().utf8().data());
	ptr.get()->transparency = mojo::String(event->transparency().utf8().data());
	ptr.get()->reminder = mojo::String(event->reminder().utf8().data());

	listener_ = callback;
	manager_->CalendarDeviceApiAddEvent(std::move(ptr),
		//sameThreadBindForMojo(&CalendarDispatcher::resultCodeCallback, this));
		createBaseCallback(bind<device::Calendar_ResultCodePtr>(&CalendarDispatcher::resultCodeCallback, this)));
}

void CalendarDispatcher::calendarDeviceApiUpdateEvent(PlatformCalendarInfo* event, Listener* callback, PlatformCalendarStatus* status) {
	DLOG(INFO) << "CalendarDispatcher::calendarDeviceApiUpdateEvent, manager_ : " << manager_;
	status_ = status;
	device::Calendar_CalendarInfoPtr ptr(device::Calendar_CalendarInfo::New());

  ptr.get()->id = mojo::String(event->id().utf8().data());
	ptr.get()->description = mojo::String(event->description().utf8().data());
	ptr.get()->location = mojo::String(event->location().utf8().data());
	ptr.get()->summary = mojo::String(event->summary().utf8().data());
	ptr.get()->start = mojo::String(event->start().utf8().data());
	ptr.get()->end = mojo::String(event->end().utf8().data());
	ptr.get()->status = mojo::String(event->status().utf8().data());
	ptr.get()->transparency = mojo::String(event->transparency().utf8().data());
	ptr.get()->reminder = mojo::String(event->reminder().utf8().data());

	listener_ = callback;
	manager_->CalendarDeviceApiUpdateEvent(std::move(ptr),
    //sameThreadBindForMojo(&CalendarDispatcher::resultCodeCallback, this));
	createBaseCallback(bind<device::Calendar_ResultCodePtr>(&CalendarDispatcher::resultCodeCallback, this)));
}

void CalendarDispatcher::calendarDeviceApiFindEvent(const String& startBefore, const String& startAfter, const String& endBefore,
  const String& endAfter, bool multiple, Listener* callback, PlatformCalendarStatus* status) {
	DLOG(INFO) << "CalendarDispatcher::calendarDeviceApiFindEvent, manager_ : " << manager_;
	status_ = status;
	listener_ = callback;
	manager_->CalendarDeviceApiFindEvent(mojo::String(startBefore.utf8().data()), mojo::String(startAfter.utf8().data()),
    mojo::String(endBefore.utf8().data()), mojo::String(endAfter.utf8().data()), multiple,
    //sameThreadBindForMojo(&CalendarDispatcher::resultCodeCallback, this));
    createBaseCallback(bind<device::Calendar_ResultCodePtr>(&CalendarDispatcher::resultCodeCallback, this)));
}

void CalendarDispatcher::calendarDeviceApiDeleteEvent(const String& id, Listener* callback, PlatformCalendarStatus* status) {
	DLOG(INFO) << "CalendarDispatcher::calendarDeviceApiDeleteEvent, manager_ : " << manager_;
	status_ = status;
	listener_ = callback;
	manager_->CalendarDeviceApiDeleteEvent(mojo::String(id.utf8().data()),
    //sameThreadBindForMojo(&CalendarDispatcher::resultCodeCallback, this));
	createBaseCallback(bind<device::Calendar_ResultCodePtr>(&CalendarDispatcher::resultCodeCallback, this)));
}

void CalendarDispatcher::resultCodeCallback(
		device::Calendar_ResultCodePtr resultCodePtr) {
	DCHECK(resultCodePtr);
	DLOG(INFO) << "CalendarDispatcher::resultCodeCallback";

	if (resultCodePtr != nullptr && status_ != nullptr) {
    status_->setResultCode(resultCodePtr->resultCode);
		switch(resultCodePtr->functionCode) {
			case function::FUNC_FIND_EVENT:
			{
				unsigned listSize = resultCodePtr->calendarlist.size();
				DLOG(INFO) << "CalendarDispatcher::resultCodeCallback, listSize : " << listSize;
				for(unsigned i = 0; i < listSize; i++) {
          PlatformCalendarInfo* data = listener_->getPlatformCalendarInfo();
					data->setId(String::fromUTF8(resultCodePtr->calendarlist[i]->id.data()));
					data->setDescription(String::fromUTF8(resultCodePtr->calendarlist[i]->description.data()));
					data->setLocation(String::fromUTF8(resultCodePtr->calendarlist[i]->location.data()));
					data->setSummary(String::fromUTF8(resultCodePtr->calendarlist[i]->summary.data()));
					data->setStart(String::fromUTF8(resultCodePtr->calendarlist[i]->start.data()));
					data->setEnd(String::fromUTF8(resultCodePtr->calendarlist[i]->end.data()));
					data->setStatus(String::fromUTF8(resultCodePtr->calendarlist[i]->status.data()));
					data->setTransparency(String::fromUTF8(resultCodePtr->calendarlist[i]->transparency.data()));
					data->setReminder(String::fromUTF8(resultCodePtr->calendarlist[i]->reminder.data()));
          listener_->appendCalendarList(data);
					DLOG(INFO) << "resultCodePtr->calendarlist[" << i << "]->start.get() : " << resultCodePtr->calendarlist[i]->start.data();
				}
			}
			break;
		}
	}

	if (listener_ != nullptr) {
		listener_->resultCalendar();
    status_ = nullptr;
	}
}

}  // namespace content
