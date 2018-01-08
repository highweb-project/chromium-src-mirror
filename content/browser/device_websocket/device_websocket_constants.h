// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_CONSTANTS_H_
#define CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_CONSTANTS_H_

namespace content {

class DeviceWebsocketConstants {
  public:
    enum FunctionCode {
      AUTHORIZE = 1,
      GET,
      SET,
      SUBSCRIBE,
      UNSUBSCRIBE,
      UNSUBSCRIBEALL,
      REQUESTMODELNAME,
    };

    enum PathCode {
      HIGHWEB = 1,
      APPLAUNCHER,
      LAUNCHAPP,
      REMOVEAPP,
      GETAPPLIST,
      GETAPPLICATIONINFO,
      SYSTEMINFO,
      DEVICESOUND,
      OUTPUTDEVICETYPE,
      DEVICEVOLUME,
      DEVICESTORAGE,
      GETDEVICESTORAGE,
      DEVICECPU,
      LOAD,
      CALENDAR,
      FINDEVENT,
      ADDEVENT,
      UPDATEEVENT,
      DELETEEVENT,
      CONTACT,
      FINDCONTACT,
      ADDCONTACT,
      DELETECONTACT,
      UPDATECONTACT,
      MESSAGING,
      ONMESSAGERECEIVED,
      SENDMESSAGE,
      FINDMESSAGE,
      GALLERY,
      FINDMEDIA,
      GETMEDIA,
      DELETEMEDIA,
      SENSOR,
      ONDEVICEPROXIMITY,
    };

    enum InternalErrorCode {
      INVALID_PATH,
      INVALID_ACTION,
      BAD_REQUEST,
      INVALID_REGISTER_CLIENT,
      UNAUTHORIZED,
      INVALID_SUBSCRIPTIONID,
    };

};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVICE_WEBSOCKET_DEVICE_WEBSOCKET_CONSTANTS_H_
