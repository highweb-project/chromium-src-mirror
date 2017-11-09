// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu_channel_opencl_proxy.h"
#include "gpu/ipc/common/gpu_messages.h"
#include "gpu_channel.h"

namespace gpu {

OpenCLProxy::OpenCLProxy(GpuChannel* channel) {
  channel_ = channel;
}

OpenCLProxy::~OpenCLProxy() {
  channel_ = nullptr;
}

bool OpenCLProxy::Send(cl_point routeId, cl_point event_key, unsigned callback_key, unsigned object_type) {
  return channel_->Send(new OpenCLChannelMsg_Callback((unsigned)routeId, event_key, callback_key, object_type));
}

unsigned int OpenCLProxy::LookupGLServiceId(unsigned int resource_id, GLResourceType glResourceType) {
  return channel_->LookupGLServiceId(resource_id, glResourceType);
}

}
