// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_IPC_SERVICE_GPU_CHANNEL_H_
#define GPU_IPC_SERVICE_GPU_CHANNEL_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "base/containers/flat_map.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/process/process.h"
#include "base/single_thread_task_runner.h"
#include "base/trace_event/memory_dump_provider.h"
#include "build/build_config.h"
#include "gpu/command_buffer/service/sync_point_manager.h"
#include "gpu/gpu_export.h"
#include "gpu/ipc/service/gpu_command_buffer_stub.h"
#include "gpu/ipc/service/gpu_memory_manager.h"
#include "ipc/ipc_sender.h"
#include "ipc/ipc_sync_channel.h"
#include "ipc/message_router.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gl/gl_share_group.h"
#include "ui/gl/gpu_preference.h"

#if defined(ENABLE_HIGHWEB_WEBCL)
#include "gpu/opencl/opencl_include.h"
#endif
#if defined(ENABLE_HIGHWEB_WEBVKC)
#include "gpu/native_vulkan/vulkan_include.h"
#endif

struct GPUCreateCommandBufferConfig;

namespace base {
class WaitableEvent;
}

namespace gpu {

class PreemptionFlag;
class Scheduler;
class SyncPointManager;
class GpuChannelManager;
class GpuChannelMessageFilter;
class GpuChannelMessageQueue;

class GPU_EXPORT FilteredSender : public IPC::Sender {
 public:
  FilteredSender();
  ~FilteredSender() override;

  virtual void AddFilter(IPC::MessageFilter* filter) = 0;
  virtual void RemoveFilter(IPC::MessageFilter* filter) = 0;
};

class GPU_EXPORT SyncChannelFilteredSender : public FilteredSender {
 public:
  SyncChannelFilteredSender(
      IPC::ChannelHandle channel_handle,
      IPC::Listener* listener,
      scoped_refptr<base::SingleThreadTaskRunner> ipc_task_runner,
      base::WaitableEvent* shutdown_event);
  ~SyncChannelFilteredSender() override;

  bool Send(IPC::Message* msg) override;
  void AddFilter(IPC::MessageFilter* filter) override;
  void RemoveFilter(IPC::MessageFilter* filter) override;

 private:
  std::unique_ptr<IPC::SyncChannel> channel_;

  DISALLOW_COPY_AND_ASSIGN(SyncChannelFilteredSender);
};

#if defined(ENABLE_HIGHWEB_WEBCL)
class CLApi;
class OpenCLProxy;
#endif
#if defined(ENABLE_HIGHWEB_WEBVKC)
class VKCApi;
#endif

// Encapsulates an IPC channel between the GPU process and one renderer
// process. On the renderer side there's a corresponding GpuChannelHost.
class GPU_EXPORT GpuChannel : public IPC::Listener, public FilteredSender {
 public:
  // Takes ownership of the renderer process handle.
  GpuChannel(GpuChannelManager* gpu_channel_manager,
             Scheduler* scheduler,
             SyncPointManager* sync_point_manager,
             scoped_refptr<gl::GLShareGroup> share_group,
             scoped_refptr<PreemptionFlag> preempting_flag,
             scoped_refptr<PreemptionFlag> preempted_flag,
             scoped_refptr<base::SingleThreadTaskRunner> task_runner,
             scoped_refptr<base::SingleThreadTaskRunner> io_task_runner,
             int32_t client_id,
             uint64_t client_tracing_id,
             bool is_gpu_host);
  ~GpuChannel() override;

  // The IPC channel cannot be passed in the constructor because it needs a
  // listener. The listener is the GpuChannel and must be constructed first.
  void Init(std::unique_ptr<FilteredSender> channel);

  base::WeakPtr<GpuChannel> AsWeakPtr();

  void SetUnhandledMessageListener(IPC::Listener* listener);

  // Get the GpuChannelManager that owns this channel.
  GpuChannelManager* gpu_channel_manager() const {
    return gpu_channel_manager_;
  }

  Scheduler* scheduler() const { return scheduler_; }

  SyncPointManager* sync_point_manager() const { return sync_point_manager_; }

  gles2::ImageManager* image_manager() const { return image_manager_.get(); }

  const scoped_refptr<base::SingleThreadTaskRunner>& task_runner() const {
    return task_runner_;
  }

  const scoped_refptr<PreemptionFlag>& preempted_flag() const {
    return preempted_flag_;
  }

  base::ProcessId GetClientPID() const;

  int client_id() const { return client_id_; }

  uint64_t client_tracing_id() const { return client_tracing_id_; }

  const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner() const {
    return io_task_runner_;
  }

  FilteredSender* channel_for_testing() const { return channel_.get(); }

  // IPC::Listener implementation:
  bool OnMessageReceived(const IPC::Message& msg) override;
  void OnChannelConnected(int32_t peer_pid) override;
  void OnChannelError() override;

  // FilteredSender implementation:
  bool Send(IPC::Message* msg) override;
  void AddFilter(IPC::MessageFilter* filter) override;
  void RemoveFilter(IPC::MessageFilter* filter) override;

  void OnCommandBufferScheduled(GpuCommandBufferStub* stub);
  void OnCommandBufferDescheduled(GpuCommandBufferStub* stub);

  gl::GLShareGroup* share_group() const { return share_group_.get(); }

  GpuCommandBufferStub* LookupCommandBuffer(int32_t route_id);

#if defined(ENABLE_HIGHWEB_WEBCL)
  unsigned int LookupGLServiceId(unsigned int resource_id, GLResourceType glResourceType);
#endif

  bool HasActiveWebGLContext() const;
  void LoseAllContexts();
  void MarkAllContextsLost();

  // Called to add a listener for a particular message routing ID.
  // Returns true if succeeded.
  bool AddRoute(int32_t route_id,
                SequenceId sequence_id,
                IPC::Listener* listener);

  // Called to remove a listener for a particular message routing ID.
  void RemoveRoute(int32_t route_id);

  void CacheShader(const std::string& key, const std::string& shader);

  uint64_t GetMemoryUsage();

  scoped_refptr<gl::GLImage> CreateImageForGpuMemoryBuffer(
      const gfx::GpuMemoryBufferHandle& handle,
      const gfx::Size& size,
      gfx::BufferFormat format,
      uint32_t internalformat,
      SurfaceHandle surface_handle);

  void HandleMessage(const IPC::Message& msg);

  // Handle messages enqueued in |message_queue_|.
  void HandleMessageOnQueue();

  // Some messages such as WaitForGetOffsetInRange and WaitForTokenInRange are
  // processed as soon as possible because the client is blocked until they
  // are completed.
  void HandleOutOfOrderMessage(const IPC::Message& msg);

  void HandleMessageForTesting(const IPC::Message& msg);

#if defined(OS_ANDROID)
  const GpuCommandBufferStub* GetOneStub() const;
#endif

 private:
  bool OnControlMessageReceived(const IPC::Message& msg);

  void HandleMessageHelper(const IPC::Message& msg);

#if defined(ENABLE_HIGHWEB_WEBCL)
  void OnCallclGetPlatformIDs(
		    const cl_uint& num_entries,
		    const std::vector<bool>& return_variable_null_status,
		    std::vector<cl_point>* point_platform_list,
		    cl_uint* num_platforms,
		    cl_int* errcode_ret);

  void OnCallclGetPlatformInfo(
		cl_point platform,
		cl_platform_info param_name,
		size_t param_value_size,
		std::vector<bool> null_param_status,
		cl_int* errcode_ret,
		std::string* param_value,
		size_t* param_value_size_ret);

  void OnCallclGetDeviceIDs(
      const cl_point&,
      const cl_device_type&,
      const cl_uint&,
      const std::vector<bool>&,
      std::vector<cl_point>*,
      cl_uint*,
      cl_int*);

  void OnCallclGetDeviceInfo_string(
      const cl_point&,
      const cl_device_info&,
      const size_t&,
      const std::vector<bool>&,
      std::string*,
      size_t*,
      cl_int*);
  void OnCallclGetDeviceInfo_cl_uint(
      const cl_point&,
      const cl_device_info&,
      const size_t&,
      const std::vector<bool>&,
      cl_uint*,
      size_t*,
      cl_int*);

  void OnCallclGetDeviceInfo_size_t_list(
      const cl_point&,
      const cl_device_info&,
      const size_t&,
      const std::vector<bool>&,
      std::vector<size_t>*,
      size_t*,
      cl_int*);

  void OnCallclGetDeviceInfo_size_t(
      const cl_point&,
      const cl_device_info&,
      const size_t&,
      const std::vector<bool>&,
      size_t*,
      size_t*,
      cl_int*);

  void OnCallclGetDeviceInfo_cl_ulong(
      const cl_point&,
      const cl_device_info&,
      const size_t&,
      const std::vector<bool>&,
      cl_ulong*,
      size_t*,
      cl_int*);

  void OnCallclGetDeviceInfo_cl_point(
      const cl_point&,
      const cl_device_info&,
      const size_t&,
      const std::vector<bool>&,
      cl_point*,
      size_t*,
      cl_int*);

  void OnCallclGetDeviceInfo_intptr_t_list(
      const cl_point&,
      const cl_device_info&,
      const size_t&,
      const std::vector<bool>&,
      std::vector<intptr_t>*,
      size_t*,
      cl_int*);

  void OnCallclCreateContextFromType(
      const std::vector<cl_context_properties>&,
      const cl_device_type&,
      const cl_point&,
      const cl_point&,
      const std::vector<bool>&,
      cl_int*,
      cl_point*);

  void OnCallclCreateContext(
      const std::vector<cl_context_properties>&,
	  const std::vector<cl_point>&,
      const cl_point&,
      const cl_point&,
      const std::vector<bool>&,
      cl_int*,
      cl_point*);

  void OnCallclWaitForevents(
      const cl_uint&,
      const std::vector<cl_point>&,
      cl_int*);

  void OnCallclGetMemObjectInfo_cl_int(
      const cl_point&,
      const cl_mem_info&,
      const size_t&,
      const std::vector<bool>&,
	  cl_int*,
	  size_t*,
	  cl_int*);

  void OnCallclGetMemObjectInfo_cl_uint(
      const cl_point&,
      const cl_mem_info&,
      const size_t&,
      const std::vector<bool>&,
	  cl_uint*,
	  size_t*,
	  cl_int*);

  void OnCallclGetMemObjectInfo_cl_ulong(
      const cl_point&,
      const cl_mem_info&,
      const size_t&,
      const std::vector<bool>&,
	  cl_ulong*,
	  size_t*,
	  cl_int*);

  void OnCallclGetMemObjectInfo_size_t(
      const cl_point&,
      const cl_mem_info&,
      const size_t&,
      const std::vector<bool>&,
	  size_t*,
	  size_t*,
	  cl_int*);

  void OnCallclGetMemObjectInfo_cl_point(
      const cl_point&,
      const cl_mem_info&,
      const size_t&,
      const std::vector<bool>&,
	  cl_point*,
	  size_t*,
	  cl_int*);

  void OnCallclCreateSubBuffer(
      const cl_point&,
      const cl_mem_flags&,
      const cl_buffer_create_type&,
	  const size_t,
	  const size_t,
	  cl_point*,
	  cl_int*);

	void OnCallclCreateSampler(
		const cl_point&,
		const cl_bool&,
		const cl_addressing_mode&,
		const cl_filter_mode&,
		const std::vector<bool>&,
		cl_int*,
		cl_point*);

	void OnCallclGetSamplerInfo_cl_uint(
		const cl_point&,
		const cl_sampler_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_uint*,
		size_t*,
		cl_int*);

	void OnCallclGetSamplerInfo_cl_point(
		const cl_point&,
		const cl_sampler_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_point*,
		size_t*,
		cl_int*);

	void OnCallclReleaseSampler(
		const cl_point&,
		cl_int*);

  void OnCallclGetImageInfo_cl_int(
      const cl_point&,
      const cl_image_info&,
      const size_t&,
      const std::vector<bool>&,
	  cl_int*,
	  size_t*,
	  cl_int*);

  void OnCallclGetImageInfo_cl_uint_list(
      const cl_point&,
      const cl_image_info&,
      const size_t&,
      const std::vector<bool>&,
	  std::vector<cl_uint>*,
	  size_t*,
	  cl_int*);

  void OnCallclGetImageInfo_size_t(
      const cl_point&,
      const cl_image_info&,
      const size_t&,
      const std::vector<bool>&,
	  size_t*,
	  size_t*,
	  cl_int*);

  void OnCallclGetImageInfo_cl_point(
      const cl_point&,
      const cl_image_info&,
      const size_t&,
      const std::vector<bool>&,
	  cl_point*,
	  size_t*,
	  cl_int*);

	void OnCallclGetEventInfo_cl_point(
		const cl_point& point_event,
		const cl_event_info& param_name,
		const size_t& param_value_size,
		const std::vector<bool>& return_variable_null_status,
		cl_point* cl_point_ret,
		size_t* param_value_size_ret,
		cl_int* errcode_ret);

	void OnCallclGetEventInfo_cl_uint(
		const cl_point& point_event,
		const cl_event_info& param_name,
		const size_t& param_value_size,
		const std::vector<bool>& return_variable_null_status,
		cl_uint* cl_uint_ret,
		size_t* param_value_size_ret,
		cl_int* errcode_ret);

	void OnCallclGetEventInfo_cl_int(
		const cl_point& point_event,
		const cl_event_info& param_name,
		const size_t& param_value_size,
		const std::vector<bool>& return_variable_null_status,
		cl_int* cl_int_ret,
		size_t* param_value_size_ret,
		cl_int* errcode_ret);

	void OnCallclGetEventProfilingInfo_cl_ulong(
		const cl_point& point_event,
		const cl_profiling_info& param_name,
		const size_t& param_value_size,
		const std::vector<bool>& return_variable_null_status,
		cl_ulong* cl_ulong_ret,
		size_t* param_value_size_ret,
		cl_int* errcode_ret);

	void OnCallclSetEventCallback(
		const cl_point& point_event,
		const cl_int& command_exec_callback_type,
		const std::vector<int>& key_list,
		cl_int* errcode_ret);

	void OnCallclReleaseEvent(
		const cl_point& point_event,
		cl_int* errcode_ret);

  void OnCallclGetContextInfo_cl_uint(
      const cl_point&,
      const cl_context_info&,
      const size_t&,
      const std::vector<bool>&,
	  cl_uint*,
	  size_t*,
	  cl_int*);

  void OnCallclGetContextInfo_cl_point(
      const cl_point&,
      const cl_context_info&,
      const size_t&,
      const std::vector<bool>&,
	  cl_point*,
	  size_t*,
	  cl_int*);

  void OnCallclGetContextInfo_cl_point_list(
      const cl_point&,
      const cl_context_info&,
      const size_t&,
      const std::vector<bool>&,
	  std::vector<cl_point>*,
	  size_t*,
	  cl_int*);

	void OnCallclSetUserEventStatus(
		const cl_point&,
		const cl_int&,
		cl_int*);

	void OnCallclCreateUserEvent(
		const cl_point&,
		const std::vector<bool>&,
		cl_int*,
		cl_point*);

	void OnCallclGetSupportedImageFormat(
		const cl_point&,
		const cl_mem_flags&,
		const cl_mem_object_type&,
		const cl_uint&,
		const std::vector<bool>&,
		std::vector<cl_uint>*,
		cl_uint*,
		cl_int*);

	void OnCallclReleaseCommon(
		const cl_point&,
		const int,
		cl_int*);

	void OnCallclCreateCommandQueue(
		const cl_point&,
		const cl_point&,
		const cl_command_queue_properties&,
		const std::vector<bool>&,
		cl_int*,
		cl_point*);

	  void OnCallclGetCommandQueueInfo_cl_ulong(
	      const cl_point&,
	      const cl_context_info&,
	      const size_t&,
	      const std::vector<bool>&,
		  cl_ulong*,
		  size_t*,
		  cl_int*);

	  void OnCallclGetCommandQueueInfo_cl_point(
	      const cl_point&,
	      const cl_context_info&,
	      const size_t&,
	      const std::vector<bool>&,
		  cl_point*,
		  size_t*,
		  cl_int*);

	  void OnCallFlush(
			  const cl_point&,
			  cl_int*);

	void OnCallclGetKernelInfo_string(
		const cl_point&,
		const cl_kernel_info&,
		const size_t&,
		const std::vector<bool>&,
		std::string*,
		size_t*,
		cl_int*);

	void OnCallclGetKernelInfo_cl_uint(
		const cl_point&,
		const cl_kernel_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_uint*,
		size_t*,
		cl_int*);

	void OnCallclGetKernelInfo_cl_point(
		const cl_point&,
		const cl_kernel_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_point*,
		size_t*,
		cl_int*);

	void OnCallclGetKernelWorkGroupInfo_size_t_list(
		const cl_point&,
		const cl_point&,
		const cl_kernel_work_group_info&,
		const size_t&,
		const std::vector<bool>&,
		std::vector<size_t>*,
		size_t*,
		cl_int*);

	void OnCallclGetKernelWorkGroupInfo_size_t(
		const cl_point&,
		const cl_point&,
		const cl_kernel_work_group_info&,
		const size_t&,
		const std::vector<bool>&,
		size_t*,
		size_t*,
		cl_int*);

	void OnCallclGetKernelWorkGroupInfo_cl_ulong(
		const cl_point&,
		const cl_point&,
		const cl_kernel_work_group_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_ulong*,
		size_t*,
		cl_int*);

	void OnCallclGetKernelArgInfo_string(
		const cl_point&,
		const cl_uint&,
		const cl_kernel_arg_info&,
		const size_t&,
		const std::vector<bool>&,
		std::string*,
		size_t*,
		cl_int*);

	void OnCallclGetKernelArgInfo_cl_uint(
		const cl_point&,
		const cl_uint&,
		const cl_kernel_arg_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_uint*,
		size_t*,
		cl_int*);

	void OnCallclGetKernelArgInfo_cl_ulong(
		const cl_point&,
		const cl_uint&,
		const cl_kernel_arg_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_ulong*,
		size_t*,
		cl_int*);

	void OnCallclReleaseKernel(
		const cl_point&,
		cl_int*);

	void OnCallclGetProgramInfo_cl_uint(
		const cl_point&,
		const cl_program_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_uint*,
		size_t*,
		cl_int*);

	void OnCallclGetProgramInfo_cl_point(
		const cl_point&,
		const cl_program_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_point*,
		size_t*,
		cl_int*);

	void OnCallclGetProgramInfo_cl_point_list(
		const cl_point&,
		const cl_program_info&,
		const size_t&,
		const std::vector<bool>&,
		std::vector<cl_point>*,
		size_t*,
		cl_int*);

	void OnCallclGetProgramInfo_string(
		const cl_point&,
		const cl_program_info&,
		const size_t&,
		const std::vector<bool>&,
		std::string*,
		size_t*,
		cl_int*);

	void OnCallclGetProgramInfo_size_t_list(
		const cl_point&,
		const cl_program_info&,
		const size_t&,
		const std::vector<bool>&,
		std::vector<size_t>*,
		size_t*,
		cl_int*);

	void OnCallclGetProgramInfo_string_list(
		const cl_point&,
		const cl_program_info&,
		const size_t&,
		const std::vector<bool>&,
		std::vector<std::string>*,
		size_t*,
		cl_int*);

	void OnCallclGetProgramInfo_size_t(
		const cl_point&,
		const cl_program_info&,
		const size_t&,
		const std::vector<bool>&,
		size_t*,
		size_t*,
		cl_int*);

	void OnCallclCreateProgramWithSource(
		const cl_point&,
		const cl_uint&,
		const std::vector<std::string>&,
		const std::vector<size_t>&,
		const std::vector<bool>&,
		cl_int*,
		cl_point*);

	void OnCallclGetProgramBuildInfo_cl_int(
		const cl_point&,
		const cl_point&,
		const cl_program_build_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_int*,
		size_t*,
		cl_int*);

	void OnCallclGetProgramBuildInfo_string(
		const cl_point&,
		const cl_point&,
		const cl_program_build_info&,
		const size_t&,
		const std::vector<bool>&,
		std::string*,
		size_t*,
		cl_int*);

	void OnCallclGetProgramBuildInfo_cl_uint(
		const cl_point&,
		const cl_point&,
		const cl_program_build_info&,
		const size_t&,
		const std::vector<bool>&,
		cl_uint*,
		size_t*,
		cl_int*);

	void OnCallclBuildProgram(
		const cl_point&,
		const cl_uint&,
		const std::vector<cl_point>&,
		const std::string&,
		const std::vector<cl_point>& key_list,
		cl_int*);

	void OnCallclEnqueueMarker(
		const cl_point&,
		cl_point*,
		cl_int*);

	void OnCallclEnqueueBarrier(
		const cl_point&,
		cl_int*);

	void OnCallclEnqueueWaitForEvents(
		const cl_point&,
		const std::vector<cl_point>&,
		const cl_uint&,
		cl_int*);

	void OnCallclCreateKernel(
		const cl_point&,
		const std::string&,
		const std::vector<bool>&,
		cl_int*,
		cl_point*);

	void OnCallclCreateKernelsInProgram(
		const cl_point&,
		const cl_uint&,
		const std::vector<cl_point>&,
		const std::vector<bool>&,
		std::vector<cl_point>*,
		cl_uint*,
		cl_int*);

	void OnCallclReleaseProgram(
		const cl_point&,
		cl_int*);

  // gl/cl sharing
  void OnCallGetGLContext(cl_point*, cl_point*);

	void OnCallCtrlSetSharedHandles(
		const base::SharedMemoryHandle& data_handle,
		const base::SharedMemoryHandle& operation_handle,
		const base::SharedMemoryHandle& result_handle,
		const base::SharedMemoryHandle& events_handle,
		bool* result);

	void OnCallCtrlClearSharedHandles(
		bool* result);

#endif

#if defined(ENABLE_HIGHWEB_WEBVKC)
  // Vulkan Function
  void OnCallVKCSetSharedHandles(
    const base::SharedMemoryHandle& data_handle,
    const base::SharedMemoryHandle& operation_handle,
    const base::SharedMemoryHandle& result_handle,
    bool* result);

  void OnCallVKCClearSharedHandles(bool* result);

  void OnCallVKCCreateInstance(
    const std::vector<std::string>& names,
    const std::vector<uint32_t>& versions,
    const std::vector<std::string>& enabledLayerNames,
    const std::vector<std::string>& enabledExtensionNames,
    VKCPoint* vkcInstance,
    int* result);

  void OnCallVKCDestroyInstance(const VKCPoint& vkcInstance, int* result);
  void OnCallVKCEnumeratePhysicalDevice(const VKCPoint& vkcInstance, VKCuint* physicalDeviceCount, VKCPoint* physicalDeviceList, int* result);
  void OnCallVKCDestroyPhysicalDevice(const VKCPoint& physicalDeviceList, int* result);
  void OnCallVKCCreateDevice(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, VKCPoint* vkcDevice, VKCPoint* vkcQueue, int* result);
  void OnCallVKCDestroyDevice(const VKCPoint& vkcDevice, const VKCPoint& vkcQueue, int* result);
  void OnCallVKCGetDeviceInfoUint(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, const VKCuint& name, VKCuint* data_uint, int* result);
  void OnCallVKCGetDeviceInfoArray(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, const VKCuint& name, std::vector<VKCuint>* data_array, int* result);
  void OnCallVKCGetDeviceInfoString(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, const VKCuint& name, std::string* data_string, int* result);
  void OnCallVKCCreateBuffer(const VKCPoint& vkcDevice, const VKCPoint& physicalDeviceList, const VKCuint& vdIndex, const VKCuint& sizeInBytes, VKCPoint* vkcBuffer, VKCPoint* vkcMemory, int* result);
  void OnCallVKCReleaseBuffer(const VKCPoint& vkcDevice, const VKCPoint& vkcBuffer, const VKCPoint& vkcMemory, int* result);
  void OnCallVKCFillBuffer(const VKCPoint& vkcDevice, const VKCPoint& vkcMemory, const std::vector<VKCuint>& uintVector, int* result);
  void OnCallVKCCreateCommandQueue(const VKCPoint& vkcDevice, const VKCPoint& physicalDeviceList, const VKCuint& vdIndex, VKCPoint* vkcCMDBuffer, VKCPoint* vkcCMDPool, int* result);
  void OnCallVKCReleaseCommandQueue(const VKCPoint& vkcDevice, const VKCPoint& vkcCMDBuffer, const VKCPoint& vkcCMDPool, int* result);
  void OnCallVKCCreateDescriptorSetLayout(const VKCPoint& vkcDevice, const VKCuint& useBufferCount, VKCPoint* vkcDescriptorSetLayout, int* result);
  void OnCallVKCReleaseDescriptorSetLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSetLayout, int* result);
  void OnCallVKCCreateDescriptorPool(const VKCPoint& vkcDevice, const VKCuint& useBufferCount, VKCPoint* vkcDescriptorPool, int* result);
  void OnCallVKCReleaseDescriptorPool(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, int* result);
  void OnCallVKCCreateDescriptorSet(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, const VKCPoint& vkcDescriptorSetLayout, VKCPoint* vkcDescriptorSet, int* result);
  void OnCallVKCReleaseDescriptorSet(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, const VKCPoint& vkcDescriptorSet, int* result);
  void OnCallVKCCreatePipelineLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSetLayout, VKCPoint* vkcPipelineLayout, int* result);
  void OnCallVKCReleasePipelineLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineLayout, int* result);
  void OnCallVKCCreateShaderModuleWithUrl(const VKCPoint& vkcDevice, const std::string& shaderPath, VKCPoint* vkcShaderModule, int* result);
  void OnCallVKCCreateShaderModuleWithSource(const VKCPoint& vkcDevice, const std::string& shaderCode, VKCPoint* vkcShaderModule, int* result);
  void OnCallVKCReleaseShaderModule(const VKCPoint& vkcDevice, const VKCPoint& vkcShaderModule, int* result);
  void OnCallVKCCreatePipeline(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineLayout, const VKCPoint& vkcShaderModule, VKCPoint* vkcPipelineCache, VKCPoint* vkcPipeline, int* result);
  void OnCallVKCReleasePipeline(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineCache, const VKCPoint& vkcPipeline, int* result);
  void OnCallVKCUpdateDescriptorSets(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSet, const std::vector<VKCPoint>& bufferVector, int* result);
  void OnCallVKCBeginQueue(const VKCPoint& vkcCMDBuffer, const VKCPoint& vkcPipeline, const VKCPoint& vkcPipelineLayout, const VKCPoint& vkcDescriptorSet, int* result);
  void OnCallVKCEndQueue(const VKCPoint& vkcCMDBuffer, int* result);
  void OnCallVKCDispatch(const VKCPoint& vkcCMDBuffer, const VKCuint& workGroupX, const VKCuint& workGroupY, const VKCuint& workGroupZ, int* result);
  void OnCallVKCPipelineBarrier(const VKCPoint& vkcCMDBuffer, int* result);
  void OnCallVKCCmdCopyBuffer(const VKCPoint& vkcCMDBuffer, const VKCPoint& srcBuffer, const VKCPoint& dstBuffer, const VKCuint& copySize, int* result);
  void OnCallVKCQueueSubmit(const VKCPoint& vkcQueue, const VKCPoint& vkcCMDBuffer, int* result);
  void OnCallVKCDeviceWaitIdle(const VKCPoint& vkcDevice, int* result);
#endif

  // Message handlers for control messages.
  void OnCreateCommandBuffer(const GPUCreateCommandBufferConfig& init_params,
                             int32_t route_id,
                             base::SharedMemoryHandle shared_state_shm,
                             bool* result,
                             gpu::Capabilities* capabilities);
  void OnDestroyCommandBuffer(int32_t route_id);
  void OnGetDriverBugWorkArounds(
      std::vector<std::string>* gpu_driver_bug_workarounds);

  std::unique_ptr<GpuCommandBufferStub> CreateCommandBuffer(
      const GPUCreateCommandBufferConfig& init_params,
      int32_t route_id,
      std::unique_ptr<base::SharedMemory> shared_state_shm);

  std::unique_ptr<FilteredSender> channel_;

  base::ProcessId peer_pid_ = base::kNullProcessId;

  scoped_refptr<GpuChannelMessageQueue> message_queue_;

  // The message filter on the io thread.
  scoped_refptr<GpuChannelMessageFilter> filter_;

  // Map of routing id to command buffer stub.
  base::flat_map<int32_t, std::unique_ptr<GpuCommandBufferStub>> stubs_;

  // Map of stream id to scheduler sequence id.
  base::flat_map<int32_t, SequenceId> stream_sequences_;

  // The lifetime of objects of this class is managed by a GpuChannelManager.
  // The GpuChannelManager destroy all the GpuChannels that they own when they
  // are destroyed. So a raw pointer is safe.
  GpuChannelManager* const gpu_channel_manager_;

  Scheduler* const scheduler_;

  // Sync point manager. Outlives the channel and is guaranteed to outlive the
  // message loop.
  SyncPointManager* const sync_point_manager_;

  IPC::Listener* unhandled_message_listener_ = nullptr;

  // Used to implement message routing functionality to CommandBuffer objects
  IPC::MessageRouter router_;

  // Whether the processing of IPCs on this channel is stalled and we should
  // preempt other GpuChannels.
  scoped_refptr<PreemptionFlag> preempting_flag_;

  // If non-NULL, all stubs on this channel should stop processing GL
  // commands (via their CommandExecutor) when preempted_flag_->IsSet()
  scoped_refptr<PreemptionFlag> preempted_flag_;

  // The id of the client who is on the other side of the channel.
  const int32_t client_id_;

  // The tracing ID used for memory allocations associated with this client.
  const uint64_t client_tracing_id_;

  // The task runners for the main thread and the io thread.
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner_;

  // The share group that all contexts associated with a particular renderer
  // process use.
  scoped_refptr<gl::GLShareGroup> share_group_;

  std::unique_ptr<gles2::ImageManager> image_manager_;

  const bool is_gpu_host_;

  #if defined(ENABLE_HIGHWEB_WEBCL)
    gpu::CLApi* clApiImpl;
    gpu::OpenCLProxy* opencl_proxy;
  #endif
  #if defined(ENABLE_HIGHWEB_WEBVKC)
    gpu::VKCApi* vkcApiImpl;
  #endif

  // Member variables should appear before the WeakPtrFactory, to ensure that
  // any WeakPtrs to Controller are invalidated before its members variable's
  // destructors are executed, rendering them invalid.

  base::WeakPtrFactory<GpuChannel> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(GpuChannel);
};

}  // namespace gpu

#endif  // GPU_IPC_SERVICE_GPU_CHANNEL_H_
