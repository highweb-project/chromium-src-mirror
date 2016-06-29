// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef Calendar_H
#define Calendar_H

#include "core/events/EventTarget.h"
#include "core/dom/ActiveDOMObject.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "wtf/text/WTFString.h"
#include "CalendarListener.h"
#include "CalendarEventSuccessCallback.h"
#include "CalendarEventErrorCallback.h"
#include "platform/calendar/calendar_dispatcher.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h" // permission popup

namespace blink {

class ExecutionContext;
class LocalFrame;
class CalendarFindOptions;
class CalendarEvent;

class Calendar : public GarbageCollectedFinalized<Calendar>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static Calendar* create(ExecutionContext* context, LocalFrame* frame)
    {
    	Calendar* calendar = new Calendar(context, frame);
      return calendar;
    }
    virtual ~Calendar();

    void acquirePermission(const String& operationType, CalendarEventErrorCallback* permissionCallback);
    void onPermissionChecked(PermissionResult result); // permission popup

    void findEvent(CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback, CalendarFindOptions* options);
    void addEvent(CalendarEvent* event, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback);
    void updateEvent(CalendarEvent* event, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback);
    void deleteEvent(const String& id, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback);
    void resultCodeCallback();

    struct CallbackData : public GarbageCollectedFinalized<CallbackData>{
      Member<CalendarEventSuccessCallback> successCallback = nullptr;
      Member<CalendarEventErrorCallback> errorCallback = nullptr;
      CallbackData(CalendarEventSuccessCallback* sCallback, CalendarEventErrorCallback* eCallback) {
        successCallback = sCallback;
        errorCallback = eCallback;
      }
      DEFINE_INLINE_TRACE() {
        visitor->trace(successCallback);
        visitor->trace(errorCallback);
      }
    };

    DECLARE_TRACE();

private:
    Calendar(ExecutionContext* context, LocalFrame* frame);
    Member<LocalFrame> mFrame;

    WTF::String mOrigin; // permission popup
    WebDeviceApiPermissionCheckClient* mClient; // permission popup

    bool isShowingPermissionPopup = false;
    Member<CalendarEventErrorCallback> mPermissionPopupCallback;

    Member<CalendarListener> mCallback;
    HeapDeque<Member<CallbackData>> mCallbackQueue;

    Member<CalendarDispatcher> dispatcher;

    String permissionCurrentOperation;
    bool hasFindPermission = false;
    bool hasAddPermission = false;
    bool hasUpdatePermission = false;
    bool hasDeletePermission = false;
};

} // namespace blink

#endif
