// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/opencl/opencl_include.h"

#include "base/memory/shared_memory.h"

#define CL_API_ENTRY
#define CL_API_CALL

typedef bool					CLboolean;
typedef long					CLint;      // 32-bit signed integer
typedef long long			CLlong;     // 64-bit signed integer
typedef unsigned long		CLuint;     // 32-bit unsigned integer
typedef unsigned long long	CLulong;    // 64-bit unsigned integer
typedef const unsigned long	CLenum;     // Used for enumerated types, such as WebCL.DEVICE_TYPE_GPU

#if defined(OS_ANDROID)
#define cl_obj_key uint32_t
#elif defined(OS_LINUX)
#define cl_obj_key uintptr_t
#endif

namespace gpu {
	class GpuChannelHost;

	cl_int webcl_getPlatformIDs(GpuChannelHost*, cl_uint, cl_platform_id*, cl_uint*);
	cl_int webcl_clGetPlatformInfo(GpuChannelHost*, cl_platform_id, cl_platform_info, size_t, char*, size_t*);
	cl_int webcl_clGetDeviceIDs(GpuChannelHost*, cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
	cl_int webcl_clGetDeviceInfo(GpuChannelHost*, cl_device_id, cl_device_info, size_t, void*, size_t*);
	cl_context webcl_clCreateContextFromType(GpuChannelHost*, cl_context_properties*, cl_device_type, void(CL_CALLBACK*)(const char*, const void*, size_t, void*), void*, cl_int*);
	cl_context webcl_clCreateContext(GpuChannelHost*, cl_context_properties*, cl_uint, const cl_device_id*, void(CL_CALLBACK*)(const char*, const void*, size_t, void*), void*, cl_int*);
	cl_int webcl_clWaitForEvents(GpuChannelHost*, cl_uint, const cl_event*);
	cl_int webcl_clGetMemObjectInfo(GpuChannelHost*, cl_mem, cl_mem_info, size_t, void*, size_t*);
	cl_mem webcl_clCreateSubBuffer(GpuChannelHost*, cl_mem, cl_mem_flags, cl_buffer_create_type, void*, cl_int*);
	cl_sampler webcl_clCreateSampler(GpuChannelHost*, cl_context, cl_bool, cl_addressing_mode, cl_filter_mode, cl_int*);
	cl_int webcl_clGetSamplerInfo(GpuChannelHost*, cl_sampler, cl_sampler_info, size_t, void*, size_t*);
	cl_int webcl_clReleaseSampler(GpuChannelHost*, cl_sampler);
	cl_int webcl_clGetImageInfo(GpuChannelHost*, cl_mem, cl_image_info, size_t, void*, size_t*);
	cl_int webcl_clGetEventInfo(GpuChannelHost*, cl_event, cl_event_info, size_t, void*, size_t*);
	cl_int webcl_clGetEventProfilingInfo(GpuChannelHost*, cl_event, cl_profiling_info, size_t, void*, size_t*);
	cl_int webcl_clSetEventCallback(GpuChannelHost*, cl_event, cl_int, int, int, int);
	cl_int webcl_clReleaseEvent(GpuChannelHost*, cl_event);
	cl_int webcl_clGetContextInfo(GpuChannelHost*, cl_context, cl_context_info, size_t, void*, size_t*);
	cl_int webcl_clSetUserEventStatus(GpuChannelHost*, cl_event, cl_int);
	cl_event webcl_clCreateUserEvent(GpuChannelHost*, cl_context, cl_int*);
	cl_int webcl_clGetSupportedImageFormats(GpuChannelHost*, cl_context, cl_mem_flags, cl_mem_object_type, cl_uint, cl_image_format*, cl_uint*);
	cl_int webcl_clReleaseCommon(GpuChannelHost*, OPENCL_OBJECT_TYPE, cl_point);
	cl_command_queue webcl_clCreateCommandQueue(GpuChannelHost*, cl_context, cl_device_id, cl_command_queue_properties, cl_int*);
	cl_int webcl_clGetCommandQueueInfo(GpuChannelHost*, cl_command_queue, cl_command_queue_info, size_t, void*, size_t*);
	cl_int webcl_clEnqueueMarker(GpuChannelHost*, cl_command_queue, cl_event*);
	cl_int webcl_clEnqueueBarrier(GpuChannelHost*, cl_command_queue);
	cl_int webcl_clEnqueueWaitForEvents(GpuChannelHost*, cl_command_queue, cl_uint, const cl_event*);
	cl_int webcl_clFlush(GpuChannelHost*, cl_command_queue);
	cl_int webcl_clGetKernelInfo(GpuChannelHost*, cl_kernel, cl_kernel_info, size_t, void*, size_t*);
	cl_int webcl_clGetKernelWorkGroupInfo(GpuChannelHost*, cl_kernel, cl_device_id, cl_kernel_work_group_info, size_t, void*, size_t*);
	cl_int webcl_clGetKernelArgInfo(GpuChannelHost*, cl_kernel, cl_uint, cl_kernel_arg_info, size_t, void*, size_t*);
	cl_int webcl_clReleaseKernel(GpuChannelHost*, cl_kernel);
	cl_int webcl_clGetProgramInfo(GpuChannelHost*, cl_program, cl_program_info, size_t, void*, size_t*);
	cl_program webcl_clCreateProgramWithSource(GpuChannelHost*, cl_context, cl_uint, const char**, const size_t*, cl_int*);
	cl_int webcl_clGetProgramBuildInfo(GpuChannelHost*, cl_program, cl_device_id, cl_program_build_info, size_t, void*, size_t*);
	cl_int webcl_clBuildProgram(GpuChannelHost*, cl_program, cl_uint, const cl_device_id*, const char*, cl_point, unsigned, unsigned);
	cl_kernel webcl_clCreateKernel(GpuChannelHost*, cl_program, const char*, cl_int*);
	cl_int webcl_clCreateKernelsInProgram(GpuChannelHost*, cl_program, cl_uint, cl_kernel*, cl_uint*);
	cl_int webcl_clReleaseProgram(GpuChannelHost*, cl_program);
	// gl/cl sharing
	cl_point webcl_getGLContext(GpuChannelHost*);
	cl_point webcl_getGLDisplay(GpuChannelHost*);

	//SHM control functions
	bool webcl_ctrlSetSharedHandles(GpuChannelHost*, base::SharedMemoryHandle, base::SharedMemoryHandle, base::SharedMemoryHandle, base::SharedMemoryHandle);
	bool webcl_ctrlClearSharedHandles(GpuChannelHost*);
	bool webcl_ctrlTriggerSharedOperation(GpuChannelHost*, int operation);
}
