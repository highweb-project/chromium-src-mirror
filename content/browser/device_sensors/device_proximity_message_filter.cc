// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/device_sensors/device_proximity_message_filter.h"

#include "content/browser/device_sensors/device_inertial_sensor_service.h"
#include "content/common/device_sensors/device_proximity_messages.h"

namespace content {

DeviceProximityMessageFilter::DeviceProximityMessageFilter()
    : BrowserMessageFilter(DeviceProximityMsgStart), is_started_(false) {
}

DeviceProximityMessageFilter::~DeviceProximityMessageFilter() {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  if (is_started_) {
    DeviceInertialSensorService::GetInstance()->RemoveConsumer(CONSUMER_TYPE_PROXIMITY);
  }
}

bool DeviceProximityMessageFilter::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(DeviceProximityMessageFilter, message)
  IPC_MESSAGE_HANDLER(DeviceProximityHostMsg_StartPolling,
                      OnDeviceProximityStartPolling)
  IPC_MESSAGE_HANDLER(DeviceProximityHostMsg_StopPolling, OnDeviceProximityStopPolling)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void DeviceProximityMessageFilter::OnDeviceProximityStartPolling() {
  DCHECK(!is_started_);
  if (is_started_)
    return;

  is_started_ = true;

  DeviceInertialSensorService::GetInstance()->AddConsumer(CONSUMER_TYPE_PROXIMITY);
  DidStartDeviceProximityPolling();
}

void DeviceProximityMessageFilter::OnDeviceProximityStopPolling() {
  DCHECK(is_started_);
  if (!is_started_)
    return;

  is_started_ = false;
  DeviceInertialSensorService::GetInstance()->RemoveConsumer(CONSUMER_TYPE_PROXIMITY);
}

void DeviceProximityMessageFilter::DidStartDeviceProximityPolling() {
  Send(new DeviceProximityMsg_DidStartPolling(
      DeviceInertialSensorService::GetInstance()
          ->GetSharedMemoryHandleForProcess(CONSUMER_TYPE_PROXIMITY,
                                            PeerHandle())));
}

}  // namespace content
