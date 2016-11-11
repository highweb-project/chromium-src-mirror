// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/calendar/calendar_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "device/calendar/calendar_ResultCode.mojom.h"

namespace device {

namespace {

class CalendarManagerEmptyImpl : public CalendarManager {
 public:
  enum function {
    FUNC_FIND_EVENT = 1,
    FUNC_ADD_EVENT,
    FUNC_UPDATE_EVENT,
    FUNC_DELETE_EVENT,
  };
  const unsigned short UNKNOWN_ERROR = 0;
  const unsigned short INVALID_ARGUMENT_ERROR = 1;
  const unsigned short TIMEOUT_ERROR = 2;
  const unsigned short PENDING_OPERATION_ERROR = 3;
  const unsigned short IO_ERROR = 4;
  const unsigned short NOT_SUPPORTED_ERROR = 5;
  const unsigned short PERMISSION_DENIED_ERROR = 20;
  const unsigned short SUCCESS = 99;
  const unsigned short NOT_SUPPORT_API = 9999;

  void CalendarDeviceApiFindEvent(
    const mojo::String& startBefore, const mojo::String& startAfter, const mojo::String& endBefore,
    const mojo::String& endAfter, bool mutiple, const CalendarDeviceApiFindEventCallback& callback) override {
      Calendar_ResultCodePtr result(Calendar_ResultCode::New());
      result->resultCode = NOT_SUPPORT_API;
      result->functionCode = function::FUNC_FIND_EVENT;
      callback.Run(result.Clone());
  }
  void CalendarDeviceApiAddEvent(
    device::Calendar_CalendarInfoPtr event, const CalendarDeviceApiAddEventCallback& callback) override {
    Calendar_ResultCodePtr result(Calendar_ResultCode::New());
    result->resultCode = NOT_SUPPORT_API;
    result->functionCode = function::FUNC_ADD_EVENT;
    callback.Run(result.Clone());
  }
  void CalendarDeviceApiUpdateEvent(
    device::Calendar_CalendarInfoPtr event, const CalendarDeviceApiUpdateEventCallback& callback) override {
    Calendar_ResultCodePtr result(Calendar_ResultCode::New());
    result->resultCode = NOT_SUPPORT_API;
    result->functionCode = function::FUNC_UPDATE_EVENT;
    callback.Run(result.Clone());
  }
  void CalendarDeviceApiDeleteEvent(const mojo::String& id, const CalendarDeviceApiDeleteEventCallback& callback) override {
    Calendar_ResultCodePtr result(Calendar_ResultCode::New());
    result->resultCode = NOT_SUPPORT_API;
    result->functionCode = function::FUNC_DELETE_EVENT;
    callback.Run(result.Clone());
  }

 private:
  friend CalendarManagerImpl;

  explicit CalendarManagerEmptyImpl(
      mojo::InterfaceRequest<CalendarManager> request)
      : binding_(this, std::move(request)) {}
  ~CalendarManagerEmptyImpl() override {}

  // The binding between this object and the other end of the pipe.
  mojo::StrongBinding<CalendarManager> binding_;
};

}  // namespace

// static
void CalendarManagerImpl::Create(
    mojo::InterfaceRequest<CalendarManager> request) {
  new CalendarManagerEmptyImpl(std::move(request));
}

}  // namespace device
