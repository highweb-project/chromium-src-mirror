// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_IPC_SERVICE_GPU_CHANNEL_OPENCL_PROXY_H_
#define GPU_IPC_SERVICE_GPU_CHANNEL_OPENCL_PROXY_H_

#include "gpu/ipc/service/gpu_config.h"
#include "gpu/opencl/opencl_gpu_manager_proxy.h"

namespace gpu {
  class GpuChannel;

	class OpenCLProxy : public gpu::CLGpuManagerProxy {
	public :
		OpenCLProxy(GpuChannel* channel);
		~OpenCLProxy();

    bool Send(cl_point, cl_point, unsigned, unsigned) override;
    unsigned int LookupGLServiceId(unsigned int resource_id, GLResourceType glResourceType) override;

  private :
    GpuChannel* channel_;
	};

}

#endif  // GPU_IPC_SERVICE_GPU_CHANNEL_OPENCL_PROXY_H_
