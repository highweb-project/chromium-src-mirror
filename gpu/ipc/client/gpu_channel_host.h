// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_IPC_CLIENT_GPU_CHANNEL_HOST_H_
#define GPU_IPC_CLIENT_GPU_CHANNEL_HOST_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "base/atomic_sequence_num.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/process/process.h"
#include "base/synchronization/lock.h"
#include "gpu/config/gpu_info.h"
#include "gpu/gpu_export.h"
#include "gpu/ipc/common/gpu_stream_constants.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_sync_channel.h"
#include "ipc/message_filter.h"
#include "ipc/message_router.h"
#include "ui/events/latency_info.h"
#include "ui/gfx/gpu_memory_buffer.h"
#include "ui/opencl/opencl_include.h"
#include "ui/native_vulkan/vulkan_include.h"

namespace base {
class WaitableEvent;
}

namespace IPC {
class SyncMessageFilter;
}

namespace gpu {
class GpuMemoryBufferManager;
}

namespace gpu {

class GPU_EXPORT GpuChannelHostFactory {
 public:
  virtual ~GpuChannelHostFactory() {}

  virtual bool IsMainThread() = 0;
  virtual scoped_refptr<base::SingleThreadTaskRunner>
  GetIOThreadTaskRunner() = 0;
  virtual std::unique_ptr<base::SharedMemory> AllocateSharedMemory(
      size_t size) = 0;
};

// Encapsulates an IPC channel between the client and one GPU process.
// On the GPU process side there's a corresponding GpuChannel.
// Every method can be called on any thread with a message loop, except for the
// IO thread.
class GPU_EXPORT GpuChannelHost
    : public IPC::Sender,
      public base::RefCountedThreadSafe<GpuChannelHost> {
 public:
	cl_int webcl_getPlatformIDs(cl_uint, cl_platform_id*, cl_uint*);
	cl_int webcl_getInfoTest(
				cl_platform_id platform,
				char* result
	);
	cl_int webcl_clGetPlatformInfo(
			cl_platform_id,
			cl_platform_info,
			size_t,
			char*,
			size_t*
	);
	cl_int webcl_clGetDeviceIDs(
			cl_platform_id platform_id,
			cl_device_type device_type,
			cl_uint num_entries,
			cl_device_id* devices,
			cl_uint* num_devices
	);
	cl_int webcl_clGetDeviceInfo(
			cl_device_id device,
			cl_device_info param_name,
			size_t param_value_size,
			void* param_value,
			size_t* param_value_size_ret
	);
	cl_context webcl_clCreateContextFromType(
			cl_context_properties* properties,
			cl_device_type device_type,
			void(CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
			void* user_data,
			cl_int* errcode_ret
	);
	cl_int webcl_clWaitForEvents(
			cl_uint num_events,
			const cl_event* event_ist
	);
	cl_int webcl_clGetMemObjectInfo(
			cl_mem memobj,
			cl_mem_info param_name,
			size_t param_value_size,
			void* param_value,
			size_t* param_value_size_ret
	);
	cl_mem webcl_clCreateSubBuffer(
			cl_mem buffer,
			cl_mem_flags flags,
			cl_buffer_create_type buffer_create_type,
			void* buffer_create_info,
			cl_int* errcode_ret
	);
	cl_sampler webcl_clCreateSampler(
			cl_context context,
			cl_bool normalized_coords,
			cl_addressing_mode addressing_mode,
			cl_filter_mode filter_mode,
			cl_int* errcode_ret
	);
	cl_int webcl_clGetSamplerInfo(
			cl_sampler sampler,
			cl_sampler_info param_name,
			size_t param_value_size,
			void* param_value,
			size_t* param_value_size_ret
	);
	cl_int webcl_clReleaseSampler(
			cl_sampler sampler
	);
	cl_int webcl_clGetImageInfo(
			cl_mem image,
			cl_image_info param_name,
			size_t param_value_size,
			void* param_value,
			size_t* param_value_size_ret
	);
	cl_int webcl_clGetContextInfo(
			cl_context context,
			cl_context_info param_name,
			size_t param_value_size,
			void* param_value,
			size_t* param_value_size_ret
	);

	cl_int webcl_clGetEventInfo(
			cl_event,
			cl_event_info,
			size_t,
			void*,
			size_t*
	);

	cl_int webcl_clGetEventProfilingInfo(
			cl_event,
			cl_profiling_info,
			size_t,
			void*,
			size_t*
	);

	cl_int webcl_clSetEventCallback(
			cl_event,
			cl_int,
			int,
			int,
			int
	);

	cl_int webcl_clReleaseEvent(
			cl_event
	);
	cl_context webcl_clCreateContext(
			cl_context_properties* properties,
			cl_uint num_devices,
			const cl_device_id* devices,
			void(CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
			void* user_data,
			cl_int* errcode_ret
	);

	cl_int webcl_clSetUserEventStatus(
			cl_event event,
			cl_int executionStatus
	);

	cl_event webcl_clCreateUserEvent(
			cl_context mContext,
			cl_int* errcodeRet
	);
	cl_int webcl_clGetSupportedImageFormats(
			cl_context context,
			cl_mem_flags flags,
			cl_mem_object_type image_type,
			cl_uint num_entries,
			cl_image_format* image_formats,
			cl_uint* num_image_formats
	);
	cl_int webcl_clReleaseCommon(
			OPENCL_OBJECT_TYPE objectType,
			cl_point object
	);
	cl_command_queue webcl_clCreateCommandQueue(
			cl_context context,
			cl_device_id device,
			cl_command_queue_properties properties,
			cl_int* errcode_ret
	);
	cl_int webcl_clGetCommandQueueInfo(
			cl_command_queue command_queue,
			cl_command_queue_info param_name,
			size_t param_value_size,
			void* param_value,
			size_t* param_value_size_ret
	);

	cl_int webcl_clEnqueueMarker(
			cl_command_queue command_queue,
			cl_event* event);

	cl_int webcl_clEnqueueBarrier(
			cl_command_queue command_queue);

	cl_int webcl_clEnqueueWaitForEvents(
			cl_command_queue command_queue,
			cl_uint num_events,
			const cl_event* event_list);

	cl_int webcl_clFinish(
			cl_command_queue command_queue);

	cl_int webcl_clFlush(
			cl_command_queue command_queue);

	cl_int webcl_clGetKernelInfo(
			cl_kernel,
			cl_kernel_info,
			size_t,
			void*,
			size_t*);
	cl_int webcl_clGetKernelWorkGroupInfo(
			cl_kernel,
			cl_device_id,
			cl_kernel_work_group_info,
			size_t,
			void*,
			size_t*);
	cl_int webcl_clGetKernelArgInfo(
			cl_kernel,
			cl_uint,
			cl_kernel_arg_info,
			size_t,
			void*,
			size_t*);

	cl_int webcl_clReleaseKernel(
			cl_kernel);

	cl_int webcl_clGetProgramInfo(
			cl_program,
			cl_program_info,
			size_t,
			void*,
			size_t*);

	cl_program webcl_clCreateProgramWithSource(
			cl_context,
			cl_uint,
			const char**,
			const size_t*,
			cl_int*);

	cl_int webcl_clGetProgramBuildInfo(
			cl_program,
			cl_device_id,
			cl_program_build_info,
			size_t,
			void*,
			size_t*);

	cl_int webcl_clBuildProgram(
			cl_program,
			cl_uint,
			const cl_device_id*,
			const char*,
			cl_point,
			unsigned,
			unsigned);

	cl_kernel webcl_clCreateKernel(
			cl_program,
			const char*,
			cl_int*);

	cl_int webcl_clCreateKernelsInProgram(
			cl_program,
			cl_uint,
			cl_kernel*,
			cl_uint*);

	cl_int webcl_clReleaseProgram(
			cl_program);

	// gl/cl sharing
	cl_point webcl_getGLContext();
	cl_point webcl_getGLDisplay();

	bool webcl_ctrlSetSharedHandles(
			base::SharedMemoryHandle,
			base::SharedMemoryHandle,
			base::SharedMemoryHandle,
			base::SharedMemoryHandle);

	bool webcl_ctrlClearSharedHandles();

	bool webcl_ctrlTriggerSharedOperation(int operation);

	//vulkan
	//sharedMemory
	bool webvkc_SetSharedHandles(
		base::SharedMemoryHandle,
		base::SharedMemoryHandle,
		base::SharedMemoryHandle);

	bool webvkc_ClearSharedHandles();

	bool webvkc_TriggerSharedOperation(int operation);

	VKCResult webvkc_createInstance(std::string& applicationName, std::string& engineName,
		uint32_t& applicationVersion, uint32_t& engineVersion, uint32_t& apiVersion,
		std::vector<std::string>& enabledLayerNames, std::vector<std::string>& enabledExtensionNames, VKCPoint* vkcInstance);
	VKCResult webvkc_destroyInstance(VKCPoint* vkcInstance);
	VKCResult webvkc_enumeratePhysicalDeviceCount(VKCPoint* vkcInstance, VKCuint* physicalDeviceCount, VKCPoint* physicalDeviceList);
	VKCResult webvkc_destroyPhysicalDevice(VKCPoint* physicalDeviceList);
	VKCResult webvkc_createDevice(VKCuint& vdIndex, VKCPoint& physicalDeviceList, VKCPoint* vkcDevice, VKCPoint* vkcQueue);
	VKCResult webvkc_destroyDevice(VKCPoint* vkcDevice, VKCPoint* vkcQueue);
	VKCResult webvkc_getDeviceInfo(VKCuint&, VKCPoint&, VKCenum&, void*);
	VKCResult webvkc_createBuffer(VKCPoint&, VKCPoint&, VKCuint&, VKCuint&, VKCPoint*, VKCPoint*);
	VKCResult webvkc_releaseBuffer(VKCPoint&, VKCPoint&, VKCPoint&);
	VKCResult webvkc_fillBuffer(VKCPoint&, VKCPoint&, std::vector<VKCuint>&);
	VKCResult webvkc_createCommandQueue(VKCPoint&, VKCPoint&, VKCuint&, VKCPoint*, VKCPoint*);
	VKCResult webvkc_releaseCommandQueue(VKCPoint&, VKCPoint&, VKCPoint&);
	VKCResult webvkc_createDescriptorSetLayout(VKCPoint&, VKCuint&, VKCPoint*);
	VKCResult webvkc_releaseDescriptorSetLayout(VKCPoint&, VKCPoint&);
	VKCResult webvkc_createDescriptorPool(VKCPoint&, VKCuint&, VKCPoint*);
	VKCResult webvkc_releaseDescriptorPool(VKCPoint&, VKCPoint&);
	VKCResult webvkc_createDescriptorSet(VKCPoint&, VKCPoint&, VKCPoint&, VKCPoint*);
	VKCResult webvkc_releaseDescriptorSet(VKCPoint&, VKCPoint&, VKCPoint&);
	VKCResult webvkc_createPipelineLayout(VKCPoint&, VKCPoint&, VKCPoint*);
	VKCResult webvkc_releasePipelineLayout(VKCPoint&, VKCPoint&);
	VKCResult webvkc_createShaderModule(VKCPoint&, std::string&, VKCPoint*);
	VKCResult webvkc_releaseShaderModule(VKCPoint&, VKCPoint&);
	VKCResult webvkc_createPipeline(VKCPoint&, VKCPoint&, VKCPoint&, VKCPoint*, VKCPoint*);
	VKCResult webvkc_releasePipeline(VKCPoint&, VKCPoint&, VKCPoint&);
	VKCResult webvkc_updateDescriptorSets(VKCPoint&, VKCPoint&, std::vector<VKCPoint>&);
	VKCResult webvkc_beginQueue(VKCPoint&, VKCPoint, VKCPoint, VKCPoint);
	VKCResult webvkc_endQueue(VKCPoint&);
	VKCResult webvkc_dispatch(VKCPoint&, VKCuint&, VKCuint&, VKCuint&);
	VKCResult webvkc_pipelineBarrier(VKCPoint&);
	VKCResult webvkc_cmdCopyBuffer(VKCPoint&, VKCPoint, VKCPoint, VKCuint&);
	VKCResult webvkc_queueSubmit(VKCPoint&, VKCPoint);
	VKCResult webvkc_deviceWaitIdle(VKCPoint&);

  // Must be called on the main thread (as defined by the factory).
  static scoped_refptr<GpuChannelHost> Create(
      GpuChannelHostFactory* factory,
      int channel_id,
      const gpu::GPUInfo& gpu_info,
      const IPC::ChannelHandle& channel_handle,
      base::WaitableEvent* shutdown_event,
      gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager);

  bool IsLost() const {
    DCHECK(channel_filter_.get());
    return channel_filter_->IsLost();
  }

  int channel_id() const { return channel_id_; }

  // The GPU stats reported by the GPU process.
  const gpu::GPUInfo& gpu_info() const { return gpu_info_; }

  // IPC::Sender implementation:
  bool Send(IPC::Message* msg) override;

  // Set an ordering barrier.  AsyncFlushes any pending barriers on other
  // routes. Combines multiple OrderingBarriers into a single AsyncFlush.
  // Returns the flush ID for the stream or 0 if put offset was not changed.
  uint32_t OrderingBarrier(int32_t route_id,
                           int32_t stream_id,
                           int32_t put_offset,
                           uint32_t flush_count,
                           const std::vector<ui::LatencyInfo>& latency_info,
                           bool put_offset_changed,
                           bool do_flush);

  void FlushPendingStream(int32_t stream_id);

  // Destroy this channel. Must be called on the main thread, before
  // destruction.
  void DestroyChannel();

  // Add a message route for the current message loop.
  void AddRoute(int route_id, base::WeakPtr<IPC::Listener> listener);

  // Add a message route to be handled on the provided |task_runner|.
  void AddRouteWithTaskRunner(
      int route_id,
      base::WeakPtr<IPC::Listener> listener,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);

  // Remove the message route associated with |route_id|.
  void RemoveRoute(int route_id);

  GpuChannelHostFactory* factory() const { return factory_; }

  gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager() const {
    return gpu_memory_buffer_manager_;
  }

  // Returns a handle to the shared memory that can be sent via IPC to the
  // GPU process. The caller is responsible for ensuring it is closed. Returns
  // an invalid handle on failure.
  base::SharedMemoryHandle ShareToGpuProcess(
      base::SharedMemoryHandle source_handle);

  // Reserve one unused transfer buffer ID.
  int32_t ReserveTransferBufferId();

  // Returns a GPU memory buffer handle to the buffer that can be sent via
  // IPC to the GPU process. The caller is responsible for ensuring it is
  // closed. Returns an invalid handle on failure.
  gfx::GpuMemoryBufferHandle ShareGpuMemoryBufferToGpuProcess(
      const gfx::GpuMemoryBufferHandle& source_handle,
      bool* requires_sync_point);

  // Reserve one unused image ID.
  int32_t ReserveImageId();

  // Generate a route ID guaranteed to be unique for this channel.
  int32_t GenerateRouteID();

  // Generate a stream ID guaranteed to be unique for this channel.
  int32_t GenerateStreamID();

  // Sends a synchronous nop to the server which validate that all previous IPC
  // messages have been received. Once the synchronous nop has been sent to the
  // server all previous flushes will all be marked as validated, including
  // flushes for other streams on the same channel. Once a validation has been
  // sent, it will return the highest validated flush id for the stream.
  // If the validation fails (which can only happen upon context lost), the
  // highest validated flush id will not change. If no flush ID were ever
  // validated then it will return 0 (Note the lowest valid flush ID is 1).
  uint32_t ValidateFlushIDReachedServer(int32_t stream_id, bool force_validate);

  // Returns the highest validated flush ID for a given stream.
  uint32_t GetHighestValidatedFlushID(int32_t stream_id);

 private:
  friend class base::RefCountedThreadSafe<GpuChannelHost>;

  // A filter used internally to route incoming messages from the IO thread
  // to the correct message loop. It also maintains some shared state between
  // all the contexts.
  class MessageFilter : public IPC::MessageFilter {
   public:
    MessageFilter();

    // Called on the IO thread.
    void AddRoute(int32_t route_id,
                  base::WeakPtr<IPC::Listener> listener,
                  scoped_refptr<base::SingleThreadTaskRunner> task_runner);
    // Called on the IO thread.
    void RemoveRoute(int32_t route_id);

    // IPC::MessageFilter implementation
    // (called on the IO thread):
    bool OnMessageReceived(const IPC::Message& msg) override;
    void OnChannelError() override;

    // The following methods can be called on any thread.

    // Whether the channel is lost.
    bool IsLost() const;

   private:
    struct ListenerInfo {
      ListenerInfo();
      ListenerInfo(const ListenerInfo& other);
      ~ListenerInfo();

      base::WeakPtr<IPC::Listener> listener;
      scoped_refptr<base::SingleThreadTaskRunner> task_runner;
    };

    ~MessageFilter() override;

    // Threading notes: |listeners_| is only accessed on the IO thread. Every
    // other field is protected by |lock_|.
    base::hash_map<int32_t, ListenerInfo> listeners_;

    // Protects all fields below this one.
    mutable base::Lock lock_;

    // Whether the channel has been lost.
    bool lost_;
  };

  struct StreamFlushInfo {
    StreamFlushInfo();
    StreamFlushInfo(const StreamFlushInfo& other);
    ~StreamFlushInfo();

    // These are global per stream.
    uint32_t next_stream_flush_id;
    uint32_t flushed_stream_flush_id;
    uint32_t verified_stream_flush_id;

    // These are local per context.
    bool flush_pending;
    int32_t route_id;
    int32_t put_offset;
    uint32_t flush_count;
    uint32_t flush_id;
    std::vector<ui::LatencyInfo> latency_info;
  };

  GpuChannelHost(GpuChannelHostFactory* factory,
                 int channel_id,
                 const gpu::GPUInfo& gpu_info,
                 gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager);
  ~GpuChannelHost() override;
  void Connect(const IPC::ChannelHandle& channel_handle,
               base::WaitableEvent* shutdown_event);
  bool InternalSend(IPC::Message* msg);
  void InternalFlush(StreamFlushInfo* flush_info);

  // Threading notes: all fields are constant during the lifetime of |this|
  // except:
  // - |next_image_id_|, atomic type
  // - |next_route_id_|, atomic type
  // - |next_stream_id_|, atomic type
  // - |channel_| and |stream_flush_info_|, protected by |context_lock_|
  GpuChannelHostFactory* const factory_;

  const int channel_id_;
  const gpu::GPUInfo gpu_info_;

  scoped_refptr<MessageFilter> channel_filter_;

  gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager_;

  // A filter for sending messages from thread other than the main thread.
  scoped_refptr<IPC::SyncMessageFilter> sync_filter_;

  // Image IDs are allocated in sequence.
  base::AtomicSequenceNumber next_image_id_;

  // Route IDs are allocated in sequence.
  base::AtomicSequenceNumber next_route_id_;

  // Stream IDs are allocated in sequence.
  base::AtomicSequenceNumber next_stream_id_;

  // Protects channel_ and stream_flush_info_.
  mutable base::Lock context_lock_;
  std::unique_ptr<IPC::SyncChannel> channel_;
  base::hash_map<int32_t, StreamFlushInfo> stream_flush_info_;

  DISALLOW_COPY_AND_ASSIGN(GpuChannelHost);
};

}  // namespace gpu

#endif  // GPU_IPC_CLIENT_GPU_CHANNEL_HOST_H_
