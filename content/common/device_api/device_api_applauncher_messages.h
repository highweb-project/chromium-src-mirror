// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "base/memory/shared_memory.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_param_traits.h"
#include "ipc/ipc_platform_file.h"
#include "content/public/common/device_api_applauncher_request.h"

#define IPC_MESSAGE_START DeviceApiApplauncherMsgStart

IPC_STRUCT_BEGIN(DeviceApiApplauncherRequestMessage)
  IPC_STRUCT_MEMBER(int32_t, functionCode)
  IPC_STRUCT_MEMBER(std::string, appId)
IPC_STRUCT_END()

IPC_STRUCT_BEGIN(DeviceApiApplauncherApplicationInfo)
  IPC_STRUCT_MEMBER(std::string, id)
  IPC_STRUCT_MEMBER(std::string, name)
  IPC_STRUCT_MEMBER(std::string, version)
  IPC_STRUCT_MEMBER(std::string, url)
  IPC_STRUCT_MEMBER(std::string, iconUrl)
IPC_STRUCT_END()

IPC_STRUCT_BEGIN(DeviceApiApplauncherResultMessage)
  IPC_STRUCT_MEMBER(int32_t, resultCode)
  IPC_STRUCT_MEMBER(int32_t, functionCode)
  IPC_STRUCT_MEMBER(std::vector<DeviceApiApplauncherApplicationInfo>, applist)
IPC_STRUCT_END()

IPC_MESSAGE_CONTROL2(DeviceApiApplauncherMsg_RequestFunction, int, DeviceApiApplauncherRequestMessage)

IPC_MESSAGE_ROUTED1(DeviceApiApplauncherMsg_RequestResult, DeviceApiApplauncherResultMessage)
