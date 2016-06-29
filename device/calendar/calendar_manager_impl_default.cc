// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/calendar/calendar_manager_impl.h"

//#include "base/basictypes.h"
#include <stddef.h>
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace device {

namespace {

class CalendarManagerEmptyImpl : public CalendarManager {
 public:
  void CalendarDeviceApiFindEvent(const mojo::String& startBefore, const mojo::String& startAfter, const mojo::String& endBefore, const mojo::String& endAfter, bool mutiple, const CalendarDeviceApiFindEventCallback& callback) override {}  
  void CalendarDeviceApiAddEvent(device::Calendar_CalendarInfoPtr event, const CalendarDeviceApiAddEventCallback& callback) override {}
  void CalendarDeviceApiUpdateEvent(device::Calendar_CalendarInfoPtr event, const CalendarDeviceApiUpdateEventCallback& callback) override {}
  void CalendarDeviceApiDeleteEvent(const mojo::String& id, const CalendarDeviceApiDeleteEventCallback& callback) override {}

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
