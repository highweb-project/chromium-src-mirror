// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "Calendar.h"

#include "base/sys_info.h"

#include "core/dom/ExecutionContext.h"
#include "core/events/Event.h"
#include "wtf/HashMap.h"

#include "modules/EventTargetModules.h"

#include "base/bind.h" // permission popup
#include "core/dom/Document.h" // permission popup
#include "modules/device_api/DeviceApiPermissionController.h" // permission popup
#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckRequest.h" // permission popup

#include "CalendarError.h"
#include "CalendarEventErrorCallback.h"
#include "CalendarEventSuccessCallback.h"
#include "CalendarFindOptions.h"
#include "CalendarEventFilter.h"
#include "CalendarEvent.h"

#include "public/platform/Platform.h"

namespace blink {

Calendar::Calendar(ExecutionContext* context, LocalFrame* frame)
{
  DLOG(INFO) << "Calendar::Calendar, context : " << context << ", frame : " << frame;

  mFrame = frame;
  mClient = DeviceApiPermissionController::from(*mFrame)->client(); // permission popup
  mOrigin = frame->document()->url().strippedForUseAsReferrer(); // permission popup
  mClient->SetOrigin(mOrigin.latin1().data()); // permission popup

  mCallbackQueue.clear();
}

Calendar::~Calendar() {
  mCallbackQueue.clear();
  dispatcher = nullptr;
}

void Calendar::addEvent(CalendarEvent* event, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback) {
  DLOG(INFO) << "Calendar::addEvent, event : " << event << ", successCallback : " << successCallback << ", errorCallback : " << errorCallback;

  if(hasAddPermission == false){
    DLOG(INFO) << "hasAddPermission FALSE";

    if(errorCallback != nullptr) {
      CalendarError* error = CalendarError::create();
      error->setCode(CalendarError::PERMISSION_DENIED_ERROR);
      errorCallback->handleEvent(error);
    }

    return;
  }

  if(event != nullptr && successCallback != nullptr && errorCallback != nullptr) {
    DLOG(INFO) << "event.description() : " << event->description().utf8().data();
    DLOG(INFO) << "event.location() : " << event->location().utf8().data();
    DLOG(INFO) << "event.summary() : " << event->summary().utf8().data();
    DLOG(INFO) << "event.start() : " << event->start().utf8().data();
    DLOG(INFO) << "event.end() : " << event->end().utf8().data();

    CalendarInfo* calendarInfo = new CalendarInfo();
    calendarInfo->setDescription(event->description());
    calendarInfo->setLocation(event->location());
    calendarInfo->setSummary(event->summary());
    calendarInfo->setStart(event->start());
    calendarInfo->setEnd(event->end());

    if(mCallback == nullptr) {
        mCallback = CalendarListener::instance(this);
    }

    CallbackData* data = new CallbackData(successCallback, errorCallback);
    mCallbackQueue.append(data);

    DLOG(INFO) << "mCallback : " << mCallback << ", data : " << data;
    dispatcher = new CalendarDispatcher();
    dispatcher->calendarDeviceApiAddEvent((PlatformCalendarInfo*)calendarInfo, mCallback, (PlatformCalendarStatus*)mCallback->lastData());
    calendarInfo = nullptr;
  }
}

void Calendar::updateEvent(CalendarEvent* event, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback) {
  DLOG(INFO) << "Calendar::updateEvent, event : " << event << ", successCallback : " << successCallback << ", errorCallback : " << errorCallback;

  if(hasUpdatePermission == false){
    DLOG(INFO) << "hasUpdatePermission FALSE";

    if(errorCallback != nullptr) {
      CalendarError* error = CalendarError::create();
      error->setCode(CalendarError::PERMISSION_DENIED_ERROR);
      errorCallback->handleEvent(error);
    }

    return;
  }

  if(event != nullptr && successCallback != nullptr && errorCallback != nullptr) {
    DLOG(INFO) << "event.id() : " << event->id().utf8().data();
    DLOG(INFO) << "event.description() : " << event->description().utf8().data();
    DLOG(INFO) << "event.location() : " << event->location().utf8().data();
    DLOG(INFO) << "event.summary() : " << event->summary().utf8().data();
    DLOG(INFO) << "event.start() : " << event->start().utf8().data();
    DLOG(INFO) << "event.end() : " << event->end().utf8().data();

    CalendarInfo* calendarInfo = new CalendarInfo();
    calendarInfo->setId(event->id());
    calendarInfo->setDescription(event->description());
    calendarInfo->setLocation(event->location());
    calendarInfo->setSummary(event->summary());
    calendarInfo->setStart(event->start());
    calendarInfo->setEnd(event->end());

    if(mCallback == nullptr) {
        mCallback = CalendarListener::instance(this);
    }

    CallbackData* data = new CallbackData(successCallback, errorCallback);
    mCallbackQueue.append(data);

    DLOG(INFO) << "mCallback : " << mCallback << ", data : " << data;
    dispatcher = new CalendarDispatcher();
    dispatcher->calendarDeviceApiUpdateEvent((PlatformCalendarInfo*)calendarInfo, mCallback, (PlatformCalendarStatus*)mCallback->lastData());
    calendarInfo = nullptr;
  }
}

void Calendar::deleteEvent(const String& id, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback) {
  DLOG(INFO) << "Calendar::deleteEvent, id : " << id.utf8().data() << ", successCallback : " << successCallback << ", errorCallback : " << errorCallback;

  if(hasDeletePermission == false){
    DLOG(INFO) << "hasDeletePermission FALSE";

    if(errorCallback != nullptr) {
      CalendarError* error = CalendarError::create();
      error->setCode(CalendarError::PERMISSION_DENIED_ERROR);
      errorCallback->handleEvent(error);
    }

    return;
  }

  if(mCallback == nullptr) {
      mCallback = CalendarListener::instance(this);
  }

  CallbackData* data = new CallbackData(successCallback, errorCallback);
  mCallbackQueue.append(data);

  DLOG(INFO) << "mCallback : " << mCallback << ", data : " << data;
  dispatcher = new CalendarDispatcher();
  dispatcher->calendarDeviceApiDeleteEvent(id, mCallback, (PlatformCalendarStatus*)mCallback->lastData());
}

void Calendar::findEvent(CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback, CalendarFindOptions* options) {
  DLOG(INFO) << "Calendar::findEvent, successCallback : " << successCallback << ", errorCallback : " << errorCallback << ", options : " << options;

  if(hasFindPermission == false){
    DLOG(INFO) << "hasFindPermission FALSE";

    if(errorCallback != nullptr) {
      CalendarError* error = CalendarError::create();
      error->setCode(CalendarError::PERMISSION_DENIED_ERROR);
      errorCallback->handleEvent(error);
    }

    return;
  }

  bool isNull = false;
  bool multiple = options->multiple(isNull);
  CalendarEventFilter* filter = options->filter();

  DLOG(INFO) << "======== DUMP CalendarFindOptions ========";
  DLOG(INFO) << "CalendarFindOptions  multiple : " << multiple << ", isNull : " << isNull;
  DLOG(INFO) << "CalendarFindOptions  filter : " << filter;
  DLOG(INFO) << "==========================================";

  if(filter != nullptr) {
    String startBefore = filter->startBefore();
    String startAfter = filter->startAfter();
    String endBefore = filter->endBefore();
    String endAfter = filter->endAfter();

    DLOG(INFO) << "======== DUMP CalendarEventFilter ========";
    DLOG(INFO) << "CalendarEventFilter  startBefore : " << startBefore.utf8().data() << ", startAfter : " << startAfter.utf8().data();
    DLOG(INFO) << "CalendarEventFilter  endBefore : " << endBefore.utf8().data() << ", endAfter : " << endAfter.utf8().data();
    DLOG(INFO) << "==========================================";

    if(mCallback == nullptr) {
        mCallback = CalendarListener::instance(this);
    }

    CallbackData* data = new CallbackData(successCallback, errorCallback);
    mCallbackQueue.append(data);

    DLOG(INFO) << "mCallback : " << mCallback << ", data : " << data;
    dispatcher = new CalendarDispatcher();
    dispatcher->calendarDeviceApiFindEvent(startBefore, startAfter, endBefore, endAfter, multiple, mCallback, (PlatformCalendarStatus*)mCallback->lastData());
  }
}

void Calendar::resultCodeCallback() {
  DLOG(INFO) << "Calendar::resultCodeCallback(), mCallback : " << mCallback;

	CalendarStatus* status = nullptr;
	if (mCallback != nullptr) {
		status = mCallback->lastData();
		mCallback = nullptr;

    DLOG(INFO) << "Calendar::resultCodeCallback(), status : " << status;

    if (status != nullptr) {
      CalendarEventSuccessCallback* successCallback = mCallbackQueue.first()->successCallback;
      DLOG(INFO) << "Calendar::resultCodeCallback(), successCallback : " << successCallback;

      if(successCallback != nullptr) {
        successCallback->handleEvent(status);
        mCallbackQueue.removeFirst();
      }
  	}
	}
  dispatcher = nullptr;
}

void Calendar::acquirePermission(const String& operationType, CalendarEventErrorCallback* permissionCallback) {
  DLOG(INFO) << "Calendar::acquirePermission, operationType : " << operationType.utf8().data() << ", permissionCallback : " << permissionCallback;

  if(isShowingPermissionPopup) {
    // permission popup is showing, push error exception
    if(permissionCallback != nullptr) {
      CalendarError* error = CalendarError::create();
      error->setCode(CalendarError::PENDING_OPERATION_ERROR);
      permissionCallback->handleEvent(error);
    }
  } else {
    WebDeviceApiPermissionCheckClient::OperationType operation = PermissionOptType::FIND;

    if(operationType == "find") {
      operation = PermissionOptType::FIND;
    } else if(operationType == "add") {
      operation = PermissionOptType::ADD;
    } else if(operationType == "update") {
      operation = PermissionOptType::UPDATE;
    } else if(operationType == "delete") {
      operation = PermissionOptType::DELETE;
    }

    // permission popup (find, add, update, delete)
    if(mClient) {
      mClient->CheckPermission(new WebDeviceApiPermissionCheckRequest(
          PermissionAPIType::CALENDAR,
          operation,
          base::Bind(&Calendar::onPermissionChecked, base::Unretained(this))));

      isShowingPermissionPopup = true;
      mPermissionPopupCallback = permissionCallback;

      permissionCurrentOperation = operationType;
    }
  }
}

// permission popup
void Calendar::onPermissionChecked(PermissionResult result)
{
	DLOG(INFO) << "Calendar::onPermissionChecked, result : " << result;
  isShowingPermissionPopup = false;

  if(mPermissionPopupCallback != nullptr) {
    CalendarError* error = CalendarError::create();

    // 0 : allow, 1 : deny
    if(result == 0) {
      error->setCode(CalendarError::SUCCESS);

      if(permissionCurrentOperation == "find") {
        hasFindPermission = true;
      } else if(permissionCurrentOperation == "add") {
        hasAddPermission = true;
      } else if(permissionCurrentOperation == "update") {
        hasUpdatePermission = true;
      } else if(permissionCurrentOperation == "delete") {
        hasDeletePermission = true;
      }

      permissionCurrentOperation = "";
    } else {
      error->setCode(CalendarError::PERMISSION_DENIED_ERROR);
    }

    mPermissionPopupCallback->handleEvent(error);
    mPermissionPopupCallback = nullptr;
  }
}

DEFINE_TRACE(Calendar) {
  visitor->trace(mCallback);
  visitor->trace(mPermissionPopupCallback);
  visitor->trace(mCallbackQueue);
  visitor->trace(mFrame);
  visitor->trace(dispatcher);
}

} // namespace blink
