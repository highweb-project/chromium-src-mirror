// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/calendar/calendar_manager_impl.h"

#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "services/device/public/interfaces/calendar_ResultCode.mojom.h"

namespace device {

namespace {

class CalendarManagerEmptyImpl : public mojom::CalendarManager {
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


  explicit CalendarManagerEmptyImpl(mojom::CalendarManagerRequest request)
      : binding_(mojo::MakeStrongBinding(base::WrapUnique(this), std::move(request))) {}
  ~CalendarManagerEmptyImpl() override {}

  void CalendarDeviceApiFindEvent(
    const std::string& startBefore, const std::string& startAfter, const std::string& endBefore,
    const std::string& endAfter, bool mutiple, CalendarDeviceApiFindEventCallback callback) override {
      mojom::Calendar_ResultCodePtr result(mojom::Calendar_ResultCode::New());
      result->resultCode = NOT_SUPPORT_API;
      result->functionCode = function::FUNC_FIND_EVENT;
      std::move(callback).Run(result.Clone());
  }
  void CalendarDeviceApiAddEvent(
    mojom::Calendar_CalendarInfoPtr event, CalendarDeviceApiAddEventCallback callback) override {
    mojom::Calendar_ResultCodePtr result(mojom::Calendar_ResultCode::New());
    result->resultCode = NOT_SUPPORT_API;
    result->functionCode = function::FUNC_ADD_EVENT;
    std::move(callback).Run(result.Clone());
  }
  void CalendarDeviceApiUpdateEvent(
    mojom::Calendar_CalendarInfoPtr event, CalendarDeviceApiUpdateEventCallback callback) override {
    mojom::Calendar_ResultCodePtr result(mojom::Calendar_ResultCode::New());
    result->resultCode = NOT_SUPPORT_API;
    result->functionCode = function::FUNC_UPDATE_EVENT;
    std::move(callback).Run(result.Clone());
  }
  void CalendarDeviceApiDeleteEvent(const std::string& id, CalendarDeviceApiDeleteEventCallback callback) override {
    mojom::Calendar_ResultCodePtr result(mojom::Calendar_ResultCode::New());
    result->resultCode = NOT_SUPPORT_API;
    result->functionCode = function::FUNC_DELETE_EVENT;
    std::move(callback).Run(result.Clone());
  }

 private:
  friend CalendarManagerImpl;

  // The binding between this object and the other end of the pipe.
  mojo::StrongBindingPtr<mojom::CalendarManager> binding_;
};

}  // namespace

// static
void CalendarManagerImpl::Create(mojom::CalendarManagerRequest request) {
  new CalendarManagerEmptyImpl(std::move(request));
}

}  // namespace device
