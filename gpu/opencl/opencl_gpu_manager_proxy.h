// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_OPENCL_OPENCL_GPU_MANAGER_PROXY_H_
#define GPU_OPENCL_OPENCL_GPU_MANAGER_PROXY_H_

#include "opencl_include.h"
#include "ipc/ipc_message.h"

namespace gpu {

	class CLGpuManagerProxy {
	public :
		CLGpuManagerProxy(){};
		~CLGpuManagerProxy(){};

    virtual bool Send(cl_point, cl_point, unsigned, unsigned) = 0;
    virtual unsigned int LookupGLServiceId(unsigned int resource_id, GLResourceType glResourceType) = 0;
	};

}

#endif  // GPU_OPENCL_OPENCL_GPU_MANAGER_PROXY_H_
