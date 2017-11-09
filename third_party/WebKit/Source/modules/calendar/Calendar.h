// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef Calendar_H
#define Calendar_H

#include "core/events/EventTarget.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"
#include "CalendarEventSuccessCallback.h"
#include "CalendarEventErrorCallback.h"

#include "public/platform/modules/device_api/WebDeviceApiPermissionCheckClient.h" // permission popup

#include "services/device/public/interfaces/calendar_manager.mojom-blink.h"
#include "services/device/public/interfaces/calendar_ResultCode.mojom-blink.h"

namespace blink {

class ExecutionContext;
class LocalFrame;
class CalendarFindOptions;
class CalendarEvent;

class Calendar : public GarbageCollectedFinalized<Calendar>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    enum function {
      FUNC_FIND_EVENT = 1,
      FUNC_ADD_EVENT,
      FUNC_UPDATE_EVENT,
      FUNC_DELETE_EVENT,
    };

    static Calendar* create(LocalFrame* frame)
    {
    	Calendar* calendar = new Calendar(frame);
      return calendar;
    }
    virtual ~Calendar();

    void acquirePermission(const String& operationType, CalendarEventErrorCallback* permissionCallback);
    void onPermissionChecked(PermissionResult result); // permission popup

    void findEvent(CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback, CalendarFindOptions* options);
    void addEvent(CalendarEvent* event, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback);
    void updateEvent(CalendarEvent* event, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback);
    void deleteEvent(const String& id, CalendarEventSuccessCallback* successCallback, CalendarEventErrorCallback* errorCallback);

    struct CallbackData : public GarbageCollectedFinalized<CallbackData>{
      Member<CalendarEventSuccessCallback> successCallback = nullptr;
      Member<CalendarEventErrorCallback> errorCallback = nullptr;
      CallbackData(CalendarEventSuccessCallback* sCallback, CalendarEventErrorCallback* eCallback) {
        successCallback = sCallback;
        errorCallback = eCallback;
      }
      DEFINE_INLINE_TRACE() {
        visitor->Trace(successCallback);
        visitor->Trace(errorCallback);
      }
    };

    DECLARE_TRACE();

private:
    Calendar(LocalFrame* frame);
    Member<LocalFrame> mFrame;

    WTF::String mOrigin; // permission popup
    WebDeviceApiPermissionCheckClient* mClient; // permission popup

    bool isShowingPermissionPopup = false;
    Member<CalendarEventErrorCallback> mPermissionPopupCallback;

    HeapDeque<Member<CallbackData>> mCallbackQueue;

    String permissionCurrentOperation;
    bool hasFindPermission = false;
    bool hasAddPermission = false;
    bool hasUpdatePermission = false;
    bool hasDeletePermission = false;

    device::mojom::blink::CalendarManagerPtr calendarManager;
    void mojoResultCallback(device::mojom::blink::Calendar_ResultCodePtr result);
    void setString(String& dst, String source);
};

} // namespace blink

#endif
