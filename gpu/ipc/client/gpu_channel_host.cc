// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/ipc/client/gpu_channel_host.h"

#include <algorithm>
#include <utility>

#include "base/atomic_sequence_num.h"
#include "base/bind.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/trace_event/trace_event.h"
#include "build/build_config.h"
#include "gpu/ipc/common/gpu_messages.h"
#include "gpu/ipc/common/gpu_param_traits_macros.h"
#include "ipc/ipc_sync_message_filter.h"
#include "url/gurl.h"

using base::AutoLock;

namespace gpu {
namespace {

// Global atomic to generate unique transfer buffer IDs.
base::StaticAtomicSequenceNumber g_next_transfer_buffer_id;

}  // namespace

GpuChannelHost::StreamFlushInfo::StreamFlushInfo()
    : next_stream_flush_id(1),
      flushed_stream_flush_id(0),
      verified_stream_flush_id(0),
      flush_pending(false),
      route_id(MSG_ROUTING_NONE),
      put_offset(0),
      flush_count(0),
      flush_id(0) {}

GpuChannelHost::StreamFlushInfo::StreamFlushInfo(const StreamFlushInfo& other) =
    default;

GpuChannelHost::StreamFlushInfo::~StreamFlushInfo() {}

// static
scoped_refptr<GpuChannelHost> GpuChannelHost::Create(
    GpuChannelHostFactory* factory,
    int channel_id,
    const gpu::GPUInfo& gpu_info,
    const IPC::ChannelHandle& channel_handle,
    base::WaitableEvent* shutdown_event,
    gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager) {
  DCHECK(factory->IsMainThread());
  scoped_refptr<GpuChannelHost> host = new GpuChannelHost(
      factory, channel_id, gpu_info, gpu_memory_buffer_manager);
  host->Connect(channel_handle, shutdown_event);
  return host;
}

GpuChannelHost::GpuChannelHost(
    GpuChannelHostFactory* factory,
    int channel_id,
    const gpu::GPUInfo& gpu_info,
    gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager)
    : factory_(factory),
      channel_id_(channel_id),
      gpu_info_(gpu_info),
      gpu_memory_buffer_manager_(gpu_memory_buffer_manager) {
  next_image_id_.GetNext();
  next_route_id_.GetNext();
  next_stream_id_.GetNext();
}

void GpuChannelHost::Connect(const IPC::ChannelHandle& channel_handle,
                             base::WaitableEvent* shutdown_event) {
  DCHECK(factory_->IsMainThread());
  // Open a channel to the GPU process. We pass nullptr as the main listener
  // here since we need to filter everything to route it to the right thread.
  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner =
      factory_->GetIOThreadTaskRunner();
  channel_ = IPC::SyncChannel::Create(channel_handle, IPC::Channel::MODE_CLIENT,
                                      nullptr, io_task_runner.get(), true,
                                      shutdown_event);

  sync_filter_ = channel_->CreateSyncMessageFilter();

  channel_filter_ = new MessageFilter();

  // Install the filter last, because we intercept all leftover
  // messages.
  channel_->AddFilter(channel_filter_.get());
}

bool GpuChannelHost::Send(IPC::Message* msg) {
  // Callee takes ownership of message, regardless of whether Send is
  // successful. See IPC::Sender.
  std::unique_ptr<IPC::Message> message(msg);
  // The GPU process never sends synchronous IPCs so clear the unblock flag to
  // preserve order.
  message->set_unblock(false);

  // Currently we need to choose between two different mechanisms for sending.
  // On the main thread we use the regular channel Send() method, on another
  // thread we use SyncMessageFilter. We also have to be careful interpreting
  // IsMainThread() since it might return false during shutdown,
  // impl we are actually calling from the main thread (discard message then).
  //
  // TODO: Can we just always use sync_filter_ since we setup the channel
  //       without a main listener?
  if (factory_->IsMainThread()) {
    // channel_ is only modified on the main thread, so we don't need to take a
    // lock here.
    if (!channel_) {
      DVLOG(1) << "GpuChannelHost::Send failed: Channel already destroyed";
      return false;
    }
    // http://crbug.com/125264
    base::ThreadRestrictions::ScopedAllowWait allow_wait;
    bool result = channel_->Send(message.release());
    if (!result)
      DVLOG(1) << "GpuChannelHost::Send failed: Channel::Send failed";
    return result;
  }

  bool result = sync_filter_->Send(message.release());
  return result;
}

uint32_t GpuChannelHost::OrderingBarrier(
    int32_t route_id,
    int32_t stream_id,
    int32_t put_offset,
    uint32_t flush_count,
    const std::vector<ui::LatencyInfo>& latency_info,
    bool put_offset_changed,
    bool do_flush,
    uint32_t* highest_verified_flush_id) {
  AutoLock lock(context_lock_);
  StreamFlushInfo& flush_info = stream_flush_info_[stream_id];
  if (flush_info.flush_pending && flush_info.route_id != route_id)
    InternalFlush(&flush_info);

  *highest_verified_flush_id = flush_info.verified_stream_flush_id;

  if (put_offset_changed) {
    const uint32_t flush_id = flush_info.next_stream_flush_id++;
    flush_info.flush_pending = true;
    flush_info.route_id = route_id;
    flush_info.put_offset = put_offset;
    flush_info.flush_count = flush_count;
    flush_info.flush_id = flush_id;
    flush_info.latency_info.insert(flush_info.latency_info.end(),
                                   latency_info.begin(), latency_info.end());

    if (do_flush)
      InternalFlush(&flush_info);

    return flush_id;
  }
  return 0;
}

void GpuChannelHost::FlushPendingStream(int32_t stream_id) {
  AutoLock lock(context_lock_);
  auto flush_info_iter = stream_flush_info_.find(stream_id);
  if (flush_info_iter == stream_flush_info_.end())
    return;

  StreamFlushInfo& flush_info = flush_info_iter->second;
  if (flush_info.flush_pending)
    InternalFlush(&flush_info);
}

void GpuChannelHost::InternalFlush(StreamFlushInfo* flush_info) {
  context_lock_.AssertAcquired();
  DCHECK(flush_info);
  DCHECK(flush_info->flush_pending);
  DCHECK_LT(flush_info->flushed_stream_flush_id, flush_info->flush_id);
  Send(new GpuCommandBufferMsg_AsyncFlush(
      flush_info->route_id, flush_info->put_offset, flush_info->flush_count,
      flush_info->latency_info));
  flush_info->latency_info.clear();
  flush_info->flush_pending = false;

  flush_info->flushed_stream_flush_id = flush_info->flush_id;
}

void GpuChannelHost::DestroyChannel() {
  DCHECK(factory_->IsMainThread());
  AutoLock lock(context_lock_);
  channel_.reset();
}

void GpuChannelHost::AddRoute(int route_id,
                              base::WeakPtr<IPC::Listener> listener) {
  AddRouteWithTaskRunner(route_id, listener,
                         base::ThreadTaskRunnerHandle::Get());
}

void GpuChannelHost::AddRouteWithTaskRunner(
    int route_id,
    base::WeakPtr<IPC::Listener> listener,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner =
      factory_->GetIOThreadTaskRunner();
  io_task_runner->PostTask(
      FROM_HERE,
      base::Bind(&GpuChannelHost::MessageFilter::AddRoute,
                 channel_filter_, route_id, listener, task_runner));
}

void GpuChannelHost::RemoveRoute(int route_id) {
  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner =
      factory_->GetIOThreadTaskRunner();
  io_task_runner->PostTask(
      FROM_HERE, base::Bind(&GpuChannelHost::MessageFilter::RemoveRoute,
                            channel_filter_, route_id));
}

base::SharedMemoryHandle GpuChannelHost::ShareToGpuProcess(
    const base::SharedMemoryHandle& source_handle) {
  if (IsLost())
    return base::SharedMemory::NULLHandle();

  return base::SharedMemory::DuplicateHandle(source_handle);
}

int32_t GpuChannelHost::ReserveTransferBufferId() {
  // 0 is a reserved value.
  return g_next_transfer_buffer_id.GetNext() + 1;
}

int32_t GpuChannelHost::ReserveImageId() {
  return next_image_id_.GetNext();
}

int32_t GpuChannelHost::GenerateRouteID() {
  return next_route_id_.GetNext();
}

int32_t GpuChannelHost::GenerateStreamID() {
  const int32_t stream_id = next_stream_id_.GetNext();
  DCHECK_NE(gpu::GPU_STREAM_INVALID, stream_id);
  DCHECK_NE(gpu::GPU_STREAM_DEFAULT, stream_id);
  return stream_id;
}

uint32_t GpuChannelHost::ValidateFlushIDReachedServer(int32_t stream_id,
                                                      bool force_validate) {
  // Store what flush ids we will be validating for all streams.
  base::hash_map<int32_t, uint32_t> validate_flushes;
  uint32_t flushed_stream_flush_id = 0;
  uint32_t verified_stream_flush_id = 0;
  {
    AutoLock lock(context_lock_);
    for (const auto& iter : stream_flush_info_) {
      const int32_t iter_stream_id = iter.first;
      const StreamFlushInfo& flush_info = iter.second;
      if (iter_stream_id == stream_id) {
        flushed_stream_flush_id = flush_info.flushed_stream_flush_id;
        verified_stream_flush_id = flush_info.verified_stream_flush_id;
      }

      if (flush_info.flushed_stream_flush_id >
          flush_info.verified_stream_flush_id) {
        validate_flushes.insert(
            std::make_pair(iter_stream_id, flush_info.flushed_stream_flush_id));
      }
    }
  }

  if (!force_validate && flushed_stream_flush_id == verified_stream_flush_id) {
    // Current stream has no unverified flushes.
    return verified_stream_flush_id;
  }

  if (Send(new GpuChannelMsg_Nop())) {
    // Update verified flush id for all streams.
    uint32_t highest_flush_id = 0;
    AutoLock lock(context_lock_);
    for (const auto& iter : validate_flushes) {
      const int32_t validated_stream_id = iter.first;
      const uint32_t validated_flush_id = iter.second;
      StreamFlushInfo& flush_info = stream_flush_info_[validated_stream_id];
      if (flush_info.verified_stream_flush_id < validated_flush_id) {
        flush_info.verified_stream_flush_id = validated_flush_id;
      }

      if (validated_stream_id == stream_id)
        highest_flush_id = flush_info.verified_stream_flush_id;
    }

    return highest_flush_id;
  }

  return 0;
}

uint32_t GpuChannelHost::GetHighestValidatedFlushID(int32_t stream_id) {
  AutoLock lock(context_lock_);
  StreamFlushInfo& flush_info = stream_flush_info_[stream_id];
  return flush_info.verified_stream_flush_id;
}

GpuChannelHost::~GpuChannelHost() {
#if DCHECK_IS_ON()
  AutoLock lock(context_lock_);
  DCHECK(!channel_)
      << "GpuChannelHost::DestroyChannel must be called before destruction.";
#endif
}

GpuChannelHost::MessageFilter::ListenerInfo::ListenerInfo() {}

GpuChannelHost::MessageFilter::ListenerInfo::ListenerInfo(
    const ListenerInfo& other) = default;

GpuChannelHost::MessageFilter::ListenerInfo::~ListenerInfo() {}

GpuChannelHost::MessageFilter::MessageFilter() : lost_(false) {}

GpuChannelHost::MessageFilter::~MessageFilter() {}

void GpuChannelHost::MessageFilter::AddRoute(
    int32_t route_id,
    base::WeakPtr<IPC::Listener> listener,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  DCHECK(listeners_.find(route_id) == listeners_.end());
  DCHECK(task_runner);
  ListenerInfo info;
  info.listener = listener;
  info.task_runner = task_runner;
  listeners_[route_id] = info;
}

void GpuChannelHost::MessageFilter::RemoveRoute(int32_t route_id) {
  listeners_.erase(route_id);
}

bool GpuChannelHost::MessageFilter::OnMessageReceived(
    const IPC::Message& message) {
  // Never handle sync message replies or we will deadlock here.
  if (message.is_reply())
    return false;

  auto it = listeners_.find(message.routing_id());
  if (it == listeners_.end())
    return false;

  const ListenerInfo& info = it->second;
  info.task_runner->PostTask(
      FROM_HERE,
      base::Bind(base::IgnoreResult(&IPC::Listener::OnMessageReceived),
                 info.listener, message));
  return true;
}

void GpuChannelHost::MessageFilter::OnChannelError() {
  // Set the lost state before signalling the proxies. That way, if they
  // themselves post a task to recreate the context, they will not try to re-use
  // this channel host.
  {
    AutoLock lock(lock_);
    lost_ = true;
  }

  // Inform all the proxies that an error has occurred. This will be reported
  // via OpenGL as a lost context.
  for (const auto& kv : listeners_) {
    const ListenerInfo& info = kv.second;
    info.task_runner->PostTask(
        FROM_HERE, base::Bind(&IPC::Listener::OnChannelError, info.listener));
  }

  listeners_.clear();
}

bool GpuChannelHost::MessageFilter::IsLost() const {
  AutoLock lock(lock_);
  return lost_;
}

#if defined(ENABLE_HIGHWEB_WEBCL)
cl_int webcl_getPlatformIDs(
		GpuChannelHost* channel_webcl,
		cl_uint num_entries,
		cl_platform_id* platforms,
		cl_uint* num_platforms)
{
  return channel_webcl->webcl_getPlatformIDs(num_entries, platforms, num_platforms);
}

cl_int webcl_clGetPlatformInfo(
		GpuChannelHost* channel_webcl,
		cl_platform_id platform,
		cl_platform_info param_name,
		size_t param_value_size,
		std::string* param_value,
		size_t* param_value_size_ret)
{
	return channel_webcl->webcl_clGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int webcl_clGetDeviceIDs(
		GpuChannelHost* channel_webcl,
		cl_platform_id platform_id,
		cl_device_type device_type,
		cl_uint num_entries,
		cl_device_id* devices,
		cl_uint* num_devices)
{
  return channel_webcl ->webcl_clGetDeviceIDs(
      platform_id,
      device_type,
      num_entries,
      devices,
	  num_devices);
}

cl_int webcl_clGetDeviceInfo(
		GpuChannelHost* channel_webcl,
		cl_device_id device,
		cl_device_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	return channel_webcl->webcl_clGetDeviceInfo(
			device,
			param_name,
			param_value_size,
			param_value,
			param_value_size_ret
	);
}

cl_context webcl_clCreateContextFromType(
		GpuChannelHost* channel_webcl,
		cl_context_properties* properties,
		cl_device_type device_type,
		void(CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
		void* user_data,
		cl_int* errcode_ret
)
{
	return channel_webcl->webcl_clCreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret);
}

cl_context webcl_clCreateContext(
		GpuChannelHost* channel_webcl,
		cl_context_properties* properties,
		cl_uint num_devices,
		const cl_device_id* devices,
		void(CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
		void* user_data,
		cl_int* errcode_ret
)
{
	return channel_webcl->webcl_clCreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
}

cl_int webcl_clWaitForEvents(
		GpuChannelHost* channel_webcl,
		cl_uint num_events,
		const cl_event* event_ist
)
{
	return channel_webcl->webcl_clWaitForEvents(num_events, event_ist);
}

cl_int webcl_clGetMemObjectInfo(
		GpuChannelHost* channel_webcl,
		cl_mem memobj,
		cl_mem_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	return channel_webcl->webcl_clGetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_mem webcl_clCreateSubBuffer(
		GpuChannelHost* channel_webcl,
		cl_mem buffer,
		cl_mem_flags flags,
		cl_buffer_create_type buffer_create_type,
		void* buffer_create_info,
		cl_int* errcode_ret
)
{
	return channel_webcl->webcl_clCreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret);
}

cl_sampler webcl_clCreateSampler(
		GpuChannelHost* channel_webcl,
		cl_context context,
		cl_bool normalized_coords,
		cl_addressing_mode addressing_mode,
		cl_filter_mode filter_mode,
		cl_int* errcode_ret
)
{
	return channel_webcl->webcl_clCreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);
}

cl_int webcl_clGetSamplerInfo(
		GpuChannelHost* channel_webcl,
		cl_sampler sampler,
		cl_sampler_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	return channel_webcl->webcl_clGetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int webcl_clReleaseSampler(
		GpuChannelHost* channel_webcl,
		cl_sampler sampler
)
{
	return channel_webcl->webcl_clReleaseSampler(sampler);
}
cl_int webcl_clGetImageInfo(
		GpuChannelHost* channel_webcl,
		cl_mem image,
		cl_image_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	return channel_webcl->webcl_clGetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int webcl_clGetEventInfo(
		GpuChannelHost* channel_webcl,
		cl_event clEvent,
		cl_event_info clEventInfo,
		size_t paramValueSize,
		void* paramValue,
		size_t* paramValueSizeRet
)
{
	return channel_webcl->webcl_clGetEventInfo(clEvent, clEventInfo, paramValueSize, paramValue, paramValueSizeRet);
}

cl_int webcl_clGetEventProfilingInfo(
		GpuChannelHost* channel_webcl,
		cl_event clEvent,
		cl_profiling_info clProfilingInfo,
		size_t paramValueSize,
		void* paramValue,
		size_t* paramValueSizeRet
)
{
	return channel_webcl->webcl_clGetEventProfilingInfo(clEvent, clProfilingInfo, paramValueSize, paramValue, paramValueSizeRet);
}

cl_int webcl_clSetEventCallback(
		GpuChannelHost* channel_webcl,
		cl_event clEvent,
		cl_int commandExecCallbackType,
		int callback_key,
		int handler_key,
		int object_type
)
{
	return channel_webcl->webcl_clSetEventCallback(clEvent, commandExecCallbackType, callback_key, handler_key, object_type);
}

cl_int webcl_clReleaseEvent(
		GpuChannelHost* channel_webcl,
		cl_event event
)
{
	return channel_webcl->webcl_clReleaseEvent(event);
}
cl_int webcl_clGetContextInfo(
		GpuChannelHost* channel_webcl,
		cl_context context,
		cl_context_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	return channel_webcl->webcl_clGetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int webcl_clSetUserEventStatus(
		GpuChannelHost* channel_webcl,
		cl_event clEvent,
		cl_int executionStatus
)
{
	return channel_webcl->webcl_clSetUserEventStatus(clEvent, executionStatus);
}

cl_event webcl_clCreateUserEvent(
		GpuChannelHost* channel_webcl,
		cl_context mContext,
		cl_int* errcodeRet
)
{
	return channel_webcl->webcl_clCreateUserEvent(mContext, errcodeRet);
}

cl_int webcl_clGetSupportedImageFormats(
		GpuChannelHost* channel_webcl,
		cl_context context,
		cl_mem_flags flags,
		cl_mem_object_type image_type,
		cl_uint num_entries,
		cl_image_format* image_formats,
		cl_uint* num_image_formats
)
{
	return channel_webcl->webcl_clGetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats);
}

cl_int webcl_clReleaseCommon(
		GpuChannelHost* channel_webcl,
		OPENCL_OBJECT_TYPE objectType,
		cl_point object
)
{
	return channel_webcl->webcl_clReleaseCommon(objectType, object);
}

cl_command_queue webcl_clCreateCommandQueue(
		GpuChannelHost* channel_webcl,
		cl_context context,
		cl_device_id device,
		cl_command_queue_properties properties,
		cl_int* errcode_ret
)
{
	return channel_webcl->webcl_clCreateCommandQueue(context, device, properties, errcode_ret);
}

cl_int webcl_clGetCommandQueueInfo(
		GpuChannelHost* channel_webcl,
		cl_command_queue command_queue,
		cl_command_queue_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	return channel_webcl->webcl_clGetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int webcl_clEnqueueMarker(
		GpuChannelHost* webcl_channel_,
		cl_command_queue command_queue,
		cl_event* event
)
{
	return webcl_channel_->webcl_clEnqueueMarker(command_queue, event);
}

cl_int webcl_clEnqueueBarrier(
		GpuChannelHost* webcl_channel_,
		cl_command_queue command_queue
)
{
	return webcl_channel_->webcl_clEnqueueBarrier(command_queue);
}

cl_int webcl_clEnqueueWaitForEvents(
		GpuChannelHost* webcl_channel_,
		cl_command_queue command_queue,
		cl_uint num_events,
		const cl_event* event_list
)
{
	return webcl_channel_->webcl_clEnqueueWaitForEvents(command_queue, num_events, event_list);
}

cl_int webcl_clFlush(
		GpuChannelHost* channel_webcl,
		cl_command_queue command_queue
)
{
	return channel_webcl->webcl_clFlush(command_queue);
}

cl_int webcl_clGetKernelInfo(
		GpuChannelHost* channel_webcl,
		cl_kernel clKernel,
		cl_kernel_info clKernelInfo,
		size_t paramValueSize,
		void* paramValue,
		size_t* paramValueSizeRet)
{
	return channel_webcl->webcl_clGetKernelInfo(clKernel, clKernelInfo, paramValueSize, paramValue, paramValueSizeRet);
}

cl_int webcl_clGetKernelWorkGroupInfo(
		GpuChannelHost* channel_webcl,
		cl_kernel kernel,
		cl_device_id device,
		cl_kernel_work_group_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	return channel_webcl->webcl_clGetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int webcl_clGetKernelArgInfo(
		GpuChannelHost* channel_webcl,
		cl_kernel kernel,
		cl_uint arg_indx,
		cl_kernel_arg_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	return channel_webcl->webcl_clGetKernelArgInfo(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int webcl_clReleaseKernel(
		GpuChannelHost* channel_webcl,
		cl_kernel kernel)
{
	return channel_webcl->webcl_clReleaseKernel(kernel);
}

cl_int webcl_clGetProgramInfo(
		GpuChannelHost* channel_webcl,
		cl_program program,
		cl_program_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	return channel_webcl->webcl_clGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_program webcl_clCreateProgramWithSource(
		GpuChannelHost* channel_webcl,
		cl_context context,
		cl_uint count,
		const char** strings,
		const size_t* lengths,
		cl_int* errcode_ret)
{
	return channel_webcl->webcl_clCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
}

cl_int webcl_clGetProgramBuildInfo(
		GpuChannelHost* channel_webcl,
		cl_program program,
		cl_device_id device,
		cl_program_build_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	return channel_webcl->webcl_clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int webcl_clBuildProgram(
		GpuChannelHost* channel_webcl,
		cl_program program,
		cl_uint num_devices,
		const cl_device_id* device_list,
		const char* options,
		cl_point event_key,
		unsigned handler_key,
		unsigned object_type)
{
	return channel_webcl->webcl_clBuildProgram(program, num_devices, device_list, options, event_key, handler_key, object_type);
}

cl_kernel webcl_clCreateKernel(
		GpuChannelHost* channel_webcl,
		cl_program program,
		const char* kernel_name,
		cl_int* errcode_ret)
{
	return channel_webcl->webcl_clCreateKernel(program, kernel_name, errcode_ret);
}

cl_int webcl_clCreateKernelsInProgram(
		GpuChannelHost* channel_webcl,
		cl_program program,
		cl_uint num_kernels,
		cl_kernel* kernels,
		cl_uint* num_kernels_ret)
{
	return channel_webcl->webcl_clCreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret);
}

cl_int webcl_clReleaseProgram(
		GpuChannelHost* channel_webcl,
		cl_program program)
{
	return channel_webcl->webcl_clReleaseProgram(program);
}

// gl/cl sharing
cl_point webcl_getGLContext(
    GpuChannelHost* channel_webcl)
{
  return channel_webcl->webcl_getGLContext();
}

cl_point webcl_getGLDisplay(
    GpuChannelHost* channel_webcl)
{
  return channel_webcl->webcl_getGLDisplay();
}

bool webcl_ctrlSetSharedHandles(
		GpuChannelHost* channel_webcl,
		base::SharedMemoryHandle data_handle,
		base::SharedMemoryHandle operation_handle,
		base::SharedMemoryHandle result_handle,
		base::SharedMemoryHandle events_handle)
{
	return channel_webcl->webcl_ctrlSetSharedHandles(data_handle, operation_handle, result_handle, events_handle);
}

bool webcl_ctrlClearSharedHandles(
		GpuChannelHost* channel_webcl)
{
	return channel_webcl->webcl_ctrlClearSharedHandles();
}

bool webcl_ctrlTriggerSharedOperation(GpuChannelHost* channel_webcl, int operation)
{
	return channel_webcl->webcl_ctrlTriggerSharedOperation(operation);
}

// gl/cl sharing
cl_point GpuChannelHost::webcl_getGLContext()
{
  cl_point glContext = 0;
  cl_point glDisplay = 0;
  CLLOG(INFO) << "webcl_getGLContext, glContext : " << glContext << ", glDisplay : " << glDisplay;

  bool result = Send(new OpenCLChannelMsg_getGLContext(&glContext, &glDisplay));
  CLLOG(INFO) << "webcl_getGLContext, IPC Result : " << result;

  if(result) {
    CLLOG(INFO) << "webcl_getGLContext, GET glContext : " << glContext << ", glDisplay : " << glDisplay;
  }

  return glContext;
}

cl_point GpuChannelHost::webcl_getGLDisplay()
{
  cl_point glContext = 0;
  cl_point glDisplay = 0;
  CLLOG(INFO) << "webcl_getGLContext, glContext : " << glContext << ", glDisplay : " << glDisplay;

  bool result = Send(new OpenCLChannelMsg_getGLContext(&glContext, &glDisplay));
  CLLOG(INFO) << "webcl_getGLContext, IPC Result : " << result;

  if(result) {
    CLLOG(INFO) << "webcl_getGLContext, GET glContext : " << glContext << ", glDisplay : " << glDisplay;
  }

  // TODO: temp modify
  // return glDisplay;
  return 1;
}

cl_int GpuChannelHost::webcl_getPlatformIDs(
		cl_uint num_entries,
		cl_platform_id* platforms,
		cl_uint* num_platforms
)
{
  // Sending a Sync IPC Message, to call a clGetPlatformIDs API
  // in other process, and getting the results of the API.
  cl_int errcode_ret;
  cl_uint num_platforms_inter;
  std::vector<cl_point> point_platform_list;
  std::vector<bool> return_variable_null_status;

  return_variable_null_status.resize(2);
  return_variable_null_status[0] = return_variable_null_status[1] = false;

  // The Sync Message can't get value back by NULL ptr, so if a
  // return back ptr is NULL, we must instead it using another
  // no-NULL ptr.
  if (NULL == num_platforms) {
    num_platforms = &num_platforms_inter;
    return_variable_null_status[0] = true;
  }
  if (NULL == platforms)
    return_variable_null_status[1] = true;

  // Send a Sync IPC Message and wait for the results.
  if (!Send(new OpenCLChannelMsg_getPlatformsIDs(
           num_entries,
           return_variable_null_status,
           &point_platform_list,
           num_platforms,
           &errcode_ret))) {
    return -1;
  }

  if (CL_SUCCESS == errcode_ret)
    for (cl_uint index = 0; index < num_entries; ++index)
      platforms[index] = (cl_platform_id) point_platform_list[index];



  return errcode_ret;
}

cl_int GpuChannelHost::webcl_clGetPlatformInfo(
		cl_platform_id platform,
		cl_platform_info param_name,
		size_t param_value_size,
		std::string* param_value,
		size_t* param_value_size_ret
)
{
	cl_int errcode_ret;
	size_t param_value_size_ret_inter = (size_t) -1;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(2);
	return_variable_null_status[0] = return_variable_null_status[1] = false;

	if(NULL == param_value_size_ret) {
		param_value_size_ret = &param_value_size_ret_inter;
		return_variable_null_status[0] = true;
	}

	if (!Send(new OpenCLChannelMsg_getPlatformInfo(
			(cl_point)platform,
			param_name,
			param_value_size,
			return_variable_null_status,
			&errcode_ret,
			param_value,
			param_value_size_ret
		))) {
		return -1;
	}

	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clGetDeviceIDs(
		cl_platform_id platform_id,
		cl_device_type device_type,
		cl_uint num_entries,
		cl_device_id* devices,
		cl_uint* num_devices)
{
  // Sending a Sync IPC Message, to call a clGetDeviceIDs API
  // in other process, and getting the results of the API.
  cl_int errcode_ret;
  cl_uint num_devices_inter;
  cl_point point_platform = (cl_point) platform_id;
  std::vector<cl_point> point_device_list;
  std::vector<bool> return_variable_null_status;

  return_variable_null_status.resize(2);
  return_variable_null_status[0] = return_variable_null_status[1] = false;

  CLLOG(INFO) << "GpuChannelHost::webcl_clGetDeviceIDs";

  // The Sync Message can't get value back by NULL ptr, so if a
  // return back ptr is NULL, we must instead it using another
  // no-NULL ptr.
  if (NULL == num_devices) {
    num_devices = &num_devices_inter;
    return_variable_null_status[0] = true;
  }
  if (NULL == devices)
    return_variable_null_status[1] = true;

  // Send a Sync IPC Message and wait for the results.
  if (!Send(new OpenCLChannelMsg_GetDeviceIDs(
           point_platform,
           device_type,
           num_entries,
           return_variable_null_status,
           &point_device_list,
           num_devices,
           &errcode_ret))) {
    return -1;
  }

  // Dump the results of the Sync IPC Message calling.
  if (CL_SUCCESS == errcode_ret)
    for (cl_uint index = 0; index < num_entries; ++index)
      devices[index] = (cl_device_id) point_device_list[index];

  return errcode_ret;
}

cl_int GpuChannelHost::webcl_clGetDeviceInfo(
		cl_device_id device,
		cl_device_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	  cl_int errcode_ret = -1;
	  size_t param_value_size_ret_inter = (size_t) -1;
	  cl_point point_device = (cl_point) device;
	  cl_uint cl_uint_ret;
	  std::vector<size_t> size_t_list_ret;
	  size_t size_t_ret;
	  cl_ulong cl_ulong_ret;
	  std::vector<intptr_t> intptr_t_list_ret;
	  std::vector<bool> return_variable_null_status;
    cl_point cl_point_ret;

	  return_variable_null_status.resize(2);
	  return_variable_null_status[0] = return_variable_null_status[1] = false;

	  if (param_value_size_ret == NULL) {
	    param_value_size_ret = &param_value_size_ret_inter;
	    return_variable_null_status[0] = true;
	  }

	  if (NULL == param_value)
	    return_variable_null_status[1] = true;

	  switch(param_name) {
        case CL_DEVICE_PLATFORM: {
        if (!Send(new OpenCLChannelMsg_GetDeviceInfo_cl_point(
                 point_device,
                 param_name,
                 param_value_size,
                 return_variable_null_status,
                 &cl_point_ret,
                 param_value_size_ret,
                 &errcode_ret))) {
          return CL_SEND_IPC_MESSAGE_FAILURE;
        }

        // Dump the results of the Sync IPC Message calling.
        if (CL_SUCCESS == errcode_ret)
          *(cl_point*) param_value = cl_point_ret;

        return errcode_ret;
      }
	    case CL_DEVICE_VENDOR_ID:
	    case CL_DEVICE_MAX_COMPUTE_UNITS:
	    case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
	    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
	    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
	    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
	    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
	    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
	    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
	    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
	    case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
	    case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
	    case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
	    case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
	    case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
	    case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
	    case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
	    case CL_DEVICE_ADDRESS_BITS:
	    case CL_DEVICE_MAX_READ_IMAGE_ARGS:
	    case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
	    case CL_DEVICE_MAX_SAMPLERS:
	    case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
	    case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
	    case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
	    case CL_DEVICE_MAX_CLOCK_FREQUENCY:
	    case CL_DEVICE_MAX_CONSTANT_ARGS:
	    case CL_DEVICE_IMAGE_SUPPORT:
	    case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
	    case CL_DEVICE_LOCAL_MEM_TYPE:
	    case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
	    case CL_DEVICE_HOST_UNIFIED_MEMORY:
	    case CL_DEVICE_ENDIAN_LITTLE:
	    case CL_DEVICE_AVAILABLE:
	    case CL_DEVICE_COMPILER_AVAILABLE: {
	      if (!Send(new OpenCLChannelMsg_GetDeviceInfo_cl_uint(
	               point_device,
	               param_name,
	               param_value_size,
	               return_variable_null_status,
	               &cl_uint_ret,
	               param_value_size_ret,
	               &errcode_ret))) {
	        return CL_SEND_IPC_MESSAGE_FAILURE;
	      }

	      // Dump the results of the Sync IPC Message calling.
	      if (CL_SUCCESS == errcode_ret)
	        *(cl_uint*) param_value = cl_uint_ret;

	      return errcode_ret;
	    }
	    case CL_DEVICE_MAX_WORK_ITEM_SIZES: {
	      // Send a Sync IPC Message and wait for the results.
	      if (!Send(new OpenCLChannelMsg_GetDeviceInfo_size_t_list(
	               point_device,
	               param_name,
	               param_value_size,
	               return_variable_null_status,
	               &size_t_list_ret,
	               param_value_size_ret,
	               &errcode_ret))) {
	        return CL_SEND_IPC_MESSAGE_FAILURE;
	      }

	      // Dump the results of the Sync IPC Message calling.
	      if (CL_SUCCESS == errcode_ret)
	        for (cl_uint index = 0; index < param_value_size/sizeof(size_t); ++index)
	          ((size_t*) (param_value))[index] = size_t_list_ret[index];

	      return errcode_ret;
	    }
	    case CL_DEVICE_MAX_WORK_GROUP_SIZE:
	    case CL_DEVICE_IMAGE2D_MAX_WIDTH:
	    case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
	    case CL_DEVICE_IMAGE3D_MAX_WIDTH:
	    case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
	    case CL_DEVICE_IMAGE3D_MAX_DEPTH:
	    case CL_DEVICE_MAX_PARAMETER_SIZE:
	    case CL_DEVICE_PROFILING_TIMER_RESOLUTION: {
	      if (!Send(new OpenCLChannelMsg_GetDeviceInfo_size_t(
	               point_device,
	               param_name,
	               param_value_size,
	               return_variable_null_status,
	               &size_t_ret,
	               param_value_size_ret,
	               &errcode_ret))) {
	        return CL_SEND_IPC_MESSAGE_FAILURE;
	      }

	      // Dump the results of the Sync IPC Message calling.
	      if (CL_SUCCESS == errcode_ret)
	        *(size_t*) param_value = size_t_ret;

	      return errcode_ret;
	    }
      case CL_DEVICE_TYPE:
	    case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
	    case CL_DEVICE_SINGLE_FP_CONFIG:
	    case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
	    case CL_DEVICE_GLOBAL_MEM_SIZE:
	    case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
	    case CL_DEVICE_LOCAL_MEM_SIZE:
	    case CL_DEVICE_EXECUTION_CAPABILITIES:
	    case CL_DEVICE_QUEUE_PROPERTIES: {
	      // Send a Sync IPC Message and wait for the results.
	      if (!Send(new OpenCLChannelMsg_GetDeviceInfo_cl_ulong(
	               point_device,
	               param_name,
	               param_value_size,
	               return_variable_null_status,
	               &cl_ulong_ret,
	               param_value_size_ret,
	               &errcode_ret))) {
	        return CL_SEND_IPC_MESSAGE_FAILURE;
	      }
	      // Dump the results of the Sync IPC Message calling.
	      if (CL_SUCCESS == errcode_ret)
	        *(cl_ulong*) param_value = cl_ulong_ret;

	      return errcode_ret;
	    }
	    case CL_DEVICE_NAME:
	    case CL_DEVICE_VENDOR:
	    case CL_DRIVER_VERSION:
	    case CL_DEVICE_EXTENSIONS: {
	      // Send a Sync IPC Message and wait for the results.
	      if (!Send(new OpenCLChannelMsg_GetDeviceInfo_string(
	               point_device,
	               param_name,
	               param_value_size,
	               return_variable_null_status,
                (std::string*)param_value,
	               param_value_size_ret,
	               &errcode_ret))) {
	        return CL_SEND_IPC_MESSAGE_FAILURE;
	      }

	      return errcode_ret;
	    }
	  }

	  return errcode_ret;
}

cl_context GpuChannelHost::webcl_clCreateContextFromType(
		cl_context_properties* properties,
		cl_device_type device_type,
		void(CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
		void* user_data,
		cl_int* errcode_ret
)
{
	// Sending a Sync IPC Message, to call a clCreateContextFromType API
	// in other process, and getting the results of the API.
	cl_int errcode_ret_inter;
	cl_point point_context_ret;
	std::vector<cl_device_partition_property> property_list;
	cl_point point_pfn_notify = (cl_point) pfn_notify;
	cl_point point_user_data = (cl_point) user_data;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(1);
	return_variable_null_status[0] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (NULL == errcode_ret) {
	errcode_ret = &errcode_ret_inter;
	return_variable_null_status[0] = true;
	}

	// Dump the inputs of the Sync IPC Message calling.
	property_list.clear();
	if (NULL != properties) {
		while (0 != *properties)
		  property_list.push_back(*properties++);
		property_list.push_back(0);
	}

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_CreateContextFromType(
		   property_list,
		   device_type,
		   point_pfn_notify,
		   point_user_data,
		   return_variable_null_status,
		   errcode_ret,
		   &point_context_ret))) {
		return NULL;
	}
	return (cl_context) point_context_ret;
}

cl_context GpuChannelHost::webcl_clCreateContext(
		cl_context_properties* properties,
		cl_uint num_devices,
		const cl_device_id* devices,
		void(CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
		void* user_data,
		cl_int* errcode_ret
)
{
	CLLOG(INFO) << "GpuChannelHost::webcl_clCreateContext, num_devices=" << num_devices;
	// Sending a Sync IPC Message, to call a clCreateContextFromType API
	// in other process, and getting the results of the API.
	cl_int errcode_ret_inter;
	cl_point point_context_ret;
	std::vector<cl_device_partition_property> property_list;
	std::vector<cl_point> device_list;
	cl_point point_pfn_notify = (cl_point) pfn_notify;
	cl_point point_user_data = (cl_point) user_data;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(1);
	return_variable_null_status[0] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (NULL == errcode_ret) {
		errcode_ret = &errcode_ret_inter;
		return_variable_null_status[0] = true;
	}

	// Dump the inputs of the Sync IPC Message calling.
	property_list.clear();
	if (NULL != properties) {
		while (0 != *properties)
		  property_list.push_back(*properties++);
		property_list.push_back(0);
	}

	//create device list
	for(unsigned i=0; i<num_devices; i++) {
		device_list.push_back((cl_point)devices[i]);
	}

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_CreateContext(
		   property_list,
		   device_list,
		   point_pfn_notify,
		   point_user_data,
		   return_variable_null_status,
		   errcode_ret,
		   &point_context_ret))) {
		return NULL;
	}
	return (cl_context) point_context_ret;
}

cl_int GpuChannelHost::webcl_clWaitForEvents(
		cl_uint num_events,
		const cl_event* event_ist
)
{
	cl_int errcode_ret_inter;
	std::vector<cl_point> event_list_inter;

	event_list_inter.clear();
	for(size_t i=0; i<num_events; i++) {
		event_list_inter.push_back((cl_point)event_ist[i]);
	}

	if (!Send(new OpenCLChannelMsg_WaitForEvents(
			num_events,
			event_list_inter,
			&errcode_ret_inter))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	return errcode_ret_inter;
}

cl_int GpuChannelHost::webcl_clGetMemObjectInfo(
		cl_mem memobj,
		cl_mem_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	cl_int errcode_ret;
	cl_point point_memobj = (cl_point)memobj;
	size_t param_value_size_ret_inter = (size_t) -1;

	std::vector<bool> return_variable_null_status;
	return_variable_null_status.resize(2);

	if(param_value == NULL) {
		return_variable_null_status[0] = true;
	}

	if(param_value_size_ret == NULL) {
		return_variable_null_status[1] = true;
		param_value_size_ret = &param_value_size_ret_inter;
	}

	switch(param_name) {
	case CL_MEM_TYPE:
		cl_int param_value_cl_int_ret;

		if (!Send(new OpenCLChannelMsg_GetMemObjectInfo_cl_int(
				point_memobj,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_cl_int_ret,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		// Dump the results of the Sync IPC Message calling.
		if (CL_SUCCESS == errcode_ret)
			*(cl_int*) param_value = param_value_cl_int_ret;

		return errcode_ret;
		break;
	case CL_MEM_FLAGS:
		cl_ulong param_value_cl_ulong_ret;

		if (!Send(new OpenCLChannelMsg_GetMemObjectInfo_cl_ulong(
				point_memobj,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_cl_ulong_ret,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		// Dump the results of the Sync IPC Message calling.
		if (CL_SUCCESS == errcode_ret)
			*(cl_ulong*) param_value = param_value_cl_ulong_ret;

		return errcode_ret;
		break;
	case CL_MEM_SIZE:
		size_t param_value_size_t_ret;

		if (!Send(new OpenCLChannelMsg_GetMemObjectInfo_size_t(
				point_memobj,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_size_t_ret,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		// Dump the results of the Sync IPC Message calling.
		if (CL_SUCCESS == errcode_ret)
			*(size_t*) param_value = param_value_size_t_ret;

		return errcode_ret;
		break;
	case CL_MEM_CONTEXT:
		cl_point param_value_cl_point_ret;

		if (!Send(new OpenCLChannelMsg_GetMemObjectInfo_size_t(
				point_memobj,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_cl_point_ret,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		// Dump the results of the Sync IPC Message calling.
		if (CL_SUCCESS == errcode_ret)
			*(cl_point*) param_value = param_value_cl_point_ret;

		return errcode_ret;
		break;
	default:
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}
}

cl_mem GpuChannelHost::webcl_clCreateSubBuffer(
		cl_mem buffer,
		cl_mem_flags flags,
		cl_buffer_create_type buffer_create_type,
		void* buffer_create_info,
		cl_int* errcode_ret
)
{
	CLLOG(INFO) << "origin=" << ((cl_buffer_region*)buffer_create_info)->origin << ", size=" << ((cl_buffer_region*)buffer_create_info)->size;

	cl_point point_buffer = (cl_point)buffer;
	cl_point point_buffer_ret;
	cl_point point_buffer_create_info = (cl_point)buffer_create_info;

	CLLOG(INFO) << "origin=" << ((cl_buffer_region*)point_buffer_create_info)->origin << ", size=" << ((cl_buffer_region*)point_buffer_create_info)->size;

	if (!Send(new OpenCLChannelMsg_CreateSubBuffer(
			point_buffer,
			flags,
			buffer_create_type,
			((cl_buffer_region*)buffer_create_info)->origin,
			((cl_buffer_region*)buffer_create_info)->size,
			&point_buffer_ret,
			errcode_ret))) {
		*errcode_ret = CL_SEND_IPC_MESSAGE_FAILURE;
	}

	return (cl_mem)point_buffer_ret;
}

cl_sampler GpuChannelHost::webcl_clCreateSampler(
		cl_context context,
		cl_bool normalized_coords,
		cl_addressing_mode addressing_mode,
		cl_filter_mode filter_mode,
		cl_int* errcode_ret
)
{
	// Sending a Sync IPC Message, to call a clCreateSampler API
	// in other process, and getting the results of the API.
	cl_int errcode_ret_inter;
	cl_point point_context = (cl_point) context;
	cl_point point_sampler_ret;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(1);
	return_variable_null_status[0] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (NULL == errcode_ret) {
		errcode_ret = &errcode_ret_inter;
		return_variable_null_status[0] = true;
	}

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_CreateSampler(
					point_context,
					normalized_coords,
					addressing_mode,
					filter_mode,
					return_variable_null_status,
					errcode_ret,
					&point_sampler_ret))) {
		return NULL;
	}
	return (cl_sampler) point_sampler_ret;
}

cl_int GpuChannelHost::webcl_clGetSamplerInfo(
		cl_sampler sampler,
		cl_sampler_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	// Sending a Sync IPC Message, to call a clCreateSubDevices API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_sampler = (cl_point) sampler;
	cl_uint cl_uint_ret;
	cl_point cl_point_ret;
	size_t param_value_size_ret_inter = (size_t) -1;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(2);
	return_variable_null_status[0] = return_variable_null_status[1] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (param_value_size_ret == NULL) {
		param_value_size_ret = &param_value_size_ret_inter;
		return_variable_null_status[0] = true;
	}

	if (NULL == param_value)
		return_variable_null_status[1] = true;

	switch(param_name) {
		case CL_SAMPLER_REFERENCE_COUNT:
		case CL_SAMPLER_NORMALIZED_COORDS:
		case CL_SAMPLER_ADDRESSING_MODE:
		case CL_SAMPLER_FILTER_MODE: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetSamplerInfo_cl_uint(
							point_sampler,
							param_name,
							param_value_size,
							return_variable_null_status,
							&cl_uint_ret,
							param_value_size_ret,
							&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_uint*) param_value = cl_uint_ret;

			return errcode_ret;
		}
		case CL_SAMPLER_CONTEXT: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetSamplerInfo_cl_point(
							point_sampler,
							param_name,
							param_value_size,
							return_variable_null_status,
							&cl_point_ret,
							param_value_size_ret,
							&errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_context*) param_value = (cl_context) cl_point_ret;

			return errcode_ret;
		}
		default: return CL_SEND_IPC_MESSAGE_FAILURE;
	}
}

cl_int GpuChannelHost::webcl_clReleaseSampler(
		cl_sampler sampler
)
{
	// Sending a Sync IPC Message, to call a clReleaseSampler API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_sampler = (cl_point) sampler;

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_ReleaseSampler(
					point_sampler,
					&errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clGetImageInfo(
		cl_mem image,
		cl_image_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	cl_int errcode_ret = CL_SEND_IPC_MESSAGE_FAILURE;
	cl_point point_image = (cl_point)image;
	size_t param_value_size_ret_inter = (size_t) -1;

	std::vector<bool> return_variable_null_status;
	return_variable_null_status.resize(2);

	if(param_value == NULL) {
		return_variable_null_status[0] = true;
	}

	if(param_value_size_ret == NULL) {
		return_variable_null_status[1] = true;
		param_value_size_ret = &param_value_size_ret_inter;
	}

	switch(param_name) {
	case CL_IMAGE_FORMAT: {
		std::vector<cl_uint> param_value_cl_uint_list;
		param_value_cl_uint_list.clear();
		param_value_cl_uint_list.push_back(0);
		param_value_cl_uint_list.push_back(1);

		if (!Send(new OpenCLChannelMsg_GetImageInfo_cl_uint_list(
				point_image,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_cl_uint_list,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		if(errcode_ret == CL_SUCCESS) {
			((cl_image_format*)param_value)->image_channel_order = param_value_cl_uint_list[0];
			((cl_image_format*)param_value)->image_channel_data_type = param_value_cl_uint_list[1];
		}

	} break;
	case CL_IMAGE_ELEMENT_SIZE:
	case CL_IMAGE_ROW_PITCH:
	case CL_IMAGE_SLICE_PITCH:
	case CL_IMAGE_WIDTH:
	case CL_IMAGE_HEIGHT:
	case CL_IMAGE_DEPTH:
		size_t param_value_size_t_ret;

		if (!Send(new OpenCLChannelMsg_GetImageInfo_size_t(
				point_image,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_size_t_ret,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		if(errcode_ret == CL_SUCCESS)
			*(size_t*)param_value = param_value_size_t_ret;

		break;
	}

	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clGetEventInfo(
		cl_event clEvent,
		cl_event_info paramName,
		size_t paramValueSize,
		void* paramValue,
		size_t* paramValueSizeRet
)
{
	// Sending a Sync IPC Message, to call a clCreateSubDevices API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_event = (cl_point) clEvent;
	cl_point cl_point_ret;
	cl_uint cl_uint_ret;
	cl_int cl_int_ret;
	size_t param_value_size_ret_inter = (size_t) -1;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(2);
	return_variable_null_status[0] = return_variable_null_status[1] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (paramValueSizeRet == NULL) {
		paramValueSizeRet = &param_value_size_ret_inter;
		return_variable_null_status[0] = true;
	}

	if (NULL == paramValue)
		return_variable_null_status[1] = true;

	switch(paramName) {
		case CL_EVENT_COMMAND_QUEUE: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetEventInfo_cl_point(
							 point_event,
							 paramName,
							 paramValueSize,
							 return_variable_null_status,
							 &cl_point_ret,
							 paramValueSizeRet,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_command_queue*) paramValue = (cl_command_queue) cl_point_ret;

			return errcode_ret;
		}
		case CL_EVENT_CONTEXT: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetEventInfo_cl_point(
							 point_event,
							 paramName,
							 paramValueSize,
							 return_variable_null_status,
							 &cl_point_ret,
							 paramValueSizeRet,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
			*(cl_context*) paramValue = (cl_context) cl_point_ret;

			return errcode_ret;
		}
		case CL_EVENT_COMMAND_TYPE: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetEventInfo_cl_uint(
							 point_event,
							 paramName,
							 paramValueSize,
							 return_variable_null_status,
							 &cl_uint_ret,
							 paramValueSizeRet,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_command_type*) paramValue = cl_uint_ret;

			return errcode_ret;
		}
		case CL_EVENT_COMMAND_EXECUTION_STATUS: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetEventInfo_cl_int(
							 point_event,
							 paramName,
							 paramValueSize,
							 return_variable_null_status,
							 &cl_int_ret,
							 paramValueSizeRet,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_int*) paramValue = cl_int_ret;

			return errcode_ret;
		}
		default: return CL_SEND_IPC_MESSAGE_FAILURE;
	}
}

cl_int GpuChannelHost::webcl_clGetEventProfilingInfo(
		cl_event clEvent,
		cl_profiling_info paramName,
		size_t paramValueSize,
		void* paramValue,
		size_t* paramValueSizeRet
)
{
	// Sending a Sync IPC Message, to call a clCreateSubDevices API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_event = (cl_point) clEvent;
	cl_ulong cl_ulong_ret;
	size_t param_value_size_ret_inter = (size_t) -1;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(2);
	return_variable_null_status[0] = return_variable_null_status[1] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (paramValueSizeRet == NULL) {
		paramValueSizeRet = &param_value_size_ret_inter;
	return_variable_null_status[0] = true;
	}

	if (NULL == paramValue)
		return_variable_null_status[1] = true;

	switch(paramName) {
		case CL_PROFILING_COMMAND_QUEUED:
		case CL_PROFILING_COMMAND_SUBMIT:
		case CL_PROFILING_COMMAND_START:
		case CL_PROFILING_COMMAND_END:
		{
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetEventProfilingInfo_cl_ulong(
							 point_event,
							 paramName,
							 paramValueSize,
							 return_variable_null_status,
							 &cl_ulong_ret,
							 paramValueSizeRet,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_ulong*) paramValue = cl_ulong_ret;

			return errcode_ret;
		}
		default: return CL_SEND_IPC_MESSAGE_FAILURE;
	}
}

cl_int GpuChannelHost::webcl_clSetEventCallback(
		cl_event clEvent,
		cl_int commandExecCallbackType,
		int callback_key,
		int handler_key,
		int object_type
)
{
	// Sending a Sync IPC Message, to call a clSetEventCallback
	// API in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_event = (cl_point) clEvent;
	std::vector<int> key_list;
	key_list.push_back(callback_key);
	key_list.push_back(handler_key);
	key_list.push_back(object_type);


	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_SetEventCallback(
					 point_event,
					 commandExecCallbackType,
					 key_list,
					 &errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}
	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clReleaseEvent(
		cl_event clEvent
)
{
	// Sending a Sync IPC Message, to call a clReleaseEvent
	// API in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_event = (cl_point) clEvent;

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_ReleaseEvent(
					 point_event,
					 &errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}
	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clGetContextInfo(
		cl_context context,
		cl_context_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	CLLOG(INFO) << "GpuChannelHost::webcl_clGetContextInfo";

	cl_int errcode_ret = CL_SEND_IPC_MESSAGE_FAILURE;
	cl_point point_context = (cl_point)context;
	size_t param_value_size_ret_inter = (size_t) -1;

	std::vector<bool> return_variable_null_status;
	return_variable_null_status.resize(2);

	if(param_value == NULL) {
		return_variable_null_status[0] = true;
	}

	if(param_value_size_ret == NULL) {
		return_variable_null_status[1] = true;
		param_value_size_ret = &param_value_size_ret_inter;
	}

	switch(param_name) {
	case CL_CONTEXT_DEVICES: {
		std::vector<cl_point> cl_point_list_ret;

		if (!Send(new OpenCLChannelMsg_GetContextInfo_cl_point_list(
				point_context,
				param_name,
				param_value_size,
				return_variable_null_status,
				&cl_point_list_ret,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		if(errcode_ret == CL_SUCCESS)
			for (cl_uint index = 0; index < param_value_size/sizeof(cl_point); ++index)
				((cl_device_id*) (param_value))[index] = (cl_device_id)cl_point_list_ret[index];

	} break;
	case CL_CONTEXT_NUM_DEVICES:
		cl_uint param_value_cl_uint_ret;

		if (!Send(new OpenCLChannelMsg_GetContextInfo_cl_uint(
				point_context,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_cl_uint_ret,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		if(errcode_ret == CL_SUCCESS)
			*(cl_uint*)param_value = param_value_cl_uint_ret;

		break;
	}

	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clSetUserEventStatus(
		cl_event clEvent,
		cl_int executionStatus
)
{
	// Sending a Sync IPC Message, to call a clSetUserEventStatus
	// API in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_event = (cl_point) clEvent;

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_SetUserEventStatus(
			point_event,
			executionStatus,
			&errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}
	return errcode_ret;
}

cl_event GpuChannelHost::webcl_clCreateUserEvent(
		cl_context mContext,
		cl_int* errcodeRet
)
{
	// Sending a Sync IPC Message, to call a clCreateUserEvent
	// API in other process, and getting the results of the API.
	cl_int errcode_ret_inter;
	cl_point point_out_context;
	cl_point point_in_context = (cl_point) mContext;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(1);
	return_variable_null_status[0] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (NULL == errcodeRet) {
		errcodeRet = &errcode_ret_inter;
		return_variable_null_status[0] = true;
	}

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_CreateUserEvent(
					point_in_context,
					return_variable_null_status,
					errcodeRet,
					&point_out_context))) {
		return NULL;
	}
	return (cl_event) point_out_context;
}

cl_int GpuChannelHost::webcl_clGetSupportedImageFormats(
		cl_context context,
		cl_mem_flags flags,
		cl_mem_object_type image_type,
		cl_uint num_entries,
		cl_image_format* image_formats,
		cl_uint* num_image_formats
)
{
	cl_int errcode_ret_inter;
	cl_point point_context = (cl_point)context;
	std::vector<cl_uint> cl_image_format_list;
	cl_uint num_image_formats_inter;

	std::vector<bool> return_variable_null_status;
	return_variable_null_status.resize(2);
	return_variable_null_status[0] = false;
	return_variable_null_status[1] = false;

	if(image_formats == NULL) {
		return_variable_null_status[0] = true;
	}

	if(num_image_formats == NULL) {
		return_variable_null_status[1] = true;
		num_image_formats = &num_image_formats_inter;
	}

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_GetSupportedImageFormat(
					point_context,
					flags,
					image_type,
					num_entries,
					return_variable_null_status,
					&cl_image_format_list,
					num_image_formats,
					&errcode_ret_inter))) {

		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	if(errcode_ret_inter == CL_SUCCESS && image_formats != NULL && num_entries > 0) {
		//image_formats = new cl_image_format[num_entries];

		for(unsigned i=0; i<num_entries; i++) {
			image_formats[i] = cl_image_format{
				(cl_channel_order)cl_image_format_list[i*2],
				(cl_channel_type)cl_image_format_list[i*2+1]
			};
		}
	}

	return errcode_ret_inter;
}

cl_int GpuChannelHost::webcl_clReleaseCommon(
		OPENCL_OBJECT_TYPE objectType,
		cl_point object
)
{
	cl_int errcode_inter = CL_INVALID_VALUE;

	if (!Send(new OpenCLChannelMsg_ReleaseCommon(
					object,
					objectType,
					&errcode_inter))) {

		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	return errcode_inter;
}

cl_command_queue GpuChannelHost::webcl_clCreateCommandQueue(
		cl_context context,
		cl_device_id device,
		cl_command_queue_properties properties,
		cl_int* errcode_ret
)
{
	cl_point point_context = (cl_point)context;
	cl_point point_device = (cl_point)device;
	cl_point point_command_queue_inter;
	cl_int errcode_inter;

	std::vector<bool> return_variable_null_status;
	return_variable_null_status.resize(1);
	return_variable_null_status[0] = false;

	if(errcode_ret == NULL) {
		return_variable_null_status[0] = true;
	}

	if (!Send(new OpenCLChannelMsg_CreateCommandQueue(
					point_context,
					point_device,
					properties,
					return_variable_null_status,
					&errcode_inter,
					&point_command_queue_inter))) {

		return NULL;
	}

	if(!return_variable_null_status[0]) {
		*errcode_ret = errcode_inter;
	}

	if(errcode_inter == CL_SUCCESS) {
		return (cl_command_queue)point_command_queue_inter;
	}
	else {
		return NULL;
	}
}

cl_int GpuChannelHost::webcl_clGetCommandQueueInfo(
		cl_command_queue command_queue,
		cl_command_queue_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret
)
{
	cl_point point_command_queue = (cl_point)command_queue;

	size_t param_value_size_ret_inter = 0;
	cl_int errcode_ret;

	std::vector<bool> return_variable_null_status;
	return_variable_null_status.resize(2);

	if(param_value == NULL) {
		return_variable_null_status[0] = true;
	}

	if(param_value_size_ret == NULL) {
		return_variable_null_status[1] = true;
		param_value_size_ret = &param_value_size_ret_inter;
	}

	switch(param_name) {
	case CL_QUEUE_CONTEXT:
		cl_point param_value_cl_context_inter;

		if (!Send(new OpenCLChannelMsg_GetCommandQueueInfo_cl_point(
				point_command_queue,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_cl_context_inter,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		if(errcode_ret == CL_SUCCESS)
			*(cl_context*)param_value = (cl_context)param_value_cl_context_inter;
		break;
	case CL_QUEUE_DEVICE:
		cl_point param_value_cl_device_inter;

		if (!Send(new OpenCLChannelMsg_GetCommandQueueInfo_cl_point(
				point_command_queue,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_cl_device_inter,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		if(errcode_ret == CL_SUCCESS)
			*(cl_device_id*)param_value = (cl_device_id)param_value_cl_device_inter;
		break;
	case CL_QUEUE_PROPERTIES:
		cl_ulong param_value_cl_ulong_inter;

		if (!Send(new OpenCLChannelMsg_GetCommandQueueInfo_cl_ulong(
				point_command_queue,
				param_name,
				param_value_size,
				return_variable_null_status,
				&param_value_cl_ulong_inter,
				param_value_size_ret,
				&errcode_ret))) {
			return CL_SEND_IPC_MESSAGE_FAILURE;
		}

		if(errcode_ret == CL_SUCCESS)
			*(cl_ulong*)param_value = param_value_cl_ulong_inter;

		break;
	}

	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clFlush(
		cl_command_queue command_queue
)
{
	cl_int errcode_inter = CL_INVALID_VALUE;

	if (!Send(new OpenCLChannelMsg_Flush(
					(cl_point)command_queue,
					&errcode_inter))) {

		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	return errcode_inter;
}

cl_int GpuChannelHost::webcl_clGetKernelInfo(
		cl_kernel kernel,
		cl_kernel_info param_name,
		size_t param_value_size,
		void *param_value,
		size_t *param_value_size_ret) {
	// Sending a Sync IPC Message, to call a clCreateSubDevices API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_kernel = (cl_point) kernel;
	std::string string_ret;
	cl_uint cl_uint_ret;
	cl_point cl_point_ret;
	size_t param_value_size_ret_inter = (size_t) -1;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(2);
	return_variable_null_status[0] = return_variable_null_status[1] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.    if ((size_t)-1 == *param_value_size_ret)
	if (param_value_size_ret == NULL) {
		param_value_size_ret = &param_value_size_ret_inter;
		return_variable_null_status[0] = true;
	}

	if (NULL == param_value)
		return_variable_null_status[1] = true;

	switch(param_name) {
		case CL_KERNEL_FUNCTION_NAME:
		case CL_KERNEL_ATTRIBUTES: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetKernelInfo_string(
							 point_kernel,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &string_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				strcpy((char*)param_value,string_ret.c_str());

			return errcode_ret;
		}
		case CL_KERNEL_NUM_ARGS:
		case CL_KERNEL_REFERENCE_COUNT: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetKernelInfo_cl_uint(
							 point_kernel,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_uint_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_uint*) param_value = cl_uint_ret;

			return errcode_ret;
		}
		case CL_KERNEL_CONTEXT: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetKernelInfo_cl_point(
							 point_kernel,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_point_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_context*) param_value = (cl_context) cl_point_ret;

			return errcode_ret;
		}
		case CL_KERNEL_PROGRAM: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetKernelInfo_cl_point(
							 point_kernel,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_point_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_program*) param_value = (cl_program) cl_point_ret;

			return errcode_ret;
		}
		default: return CL_SEND_IPC_MESSAGE_FAILURE;
	}
}

cl_int GpuChannelHost::webcl_clGetKernelWorkGroupInfo(
		cl_kernel kernel,
		cl_device_id device,
		cl_kernel_work_group_info param_name,
		size_t param_value_size,
		void *param_value,
		size_t *param_value_size_ret) {
	// Sending a Sync IPC Message, to call a clCreateSubDevices API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_kernel = (cl_point) kernel;
	cl_point point_device = (cl_point) device;
	std::vector<size_t> size_t_list_ret;
	size_t size_t_ret;
	cl_ulong cl_ulong_ret;
	size_t param_value_size_ret_inter = (size_t) -1;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(2);
	return_variable_null_status[0] = return_variable_null_status[1] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (param_value_size_ret == NULL) {
		param_value_size_ret = &param_value_size_ret_inter;
		return_variable_null_status[0] = true;
	}

	if (NULL == param_value)
		return_variable_null_status[1] = true;

	switch(param_name) {
		case CL_KERNEL_GLOBAL_WORK_SIZE:
		case CL_KERNEL_COMPILE_WORK_GROUP_SIZE: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetKernelWorkGroupInfo_size_t_list(
							 point_kernel,
							 point_device,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &size_t_list_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				for (cl_uint index = 0; index < param_value_size/sizeof(size_t); ++index)
					((size_t*) (param_value))[index] = size_t_list_ret[index];

			return errcode_ret;
		}
		case CL_KERNEL_WORK_GROUP_SIZE:
		case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetKernelWorkGroupInfo_size_t(
							 point_kernel,
							 point_device,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &size_t_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
		*(size_t*) param_value = size_t_ret;

			return errcode_ret;
		}
		case CL_KERNEL_LOCAL_MEM_SIZE:
		case CL_KERNEL_PRIVATE_MEM_SIZE: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetKernelWorkGroupInfo_cl_ulong(
							 point_kernel,
							 point_device,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_ulong_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
			 *(cl_ulong*) param_value = cl_ulong_ret;

			return errcode_ret;
		}
		default: return CL_SEND_IPC_MESSAGE_FAILURE;
	}
}

cl_int GpuChannelHost::webcl_clGetKernelArgInfo(
    cl_kernel kernel,
    cl_uint arg_indx,
    cl_kernel_arg_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret) {
  // Sending a Sync IPC Message, to call a clCreateSubDevices API
  // in other process, and getting the results of the API.
  cl_int errcode_ret;
  cl_point point_kernel = (cl_point) kernel;
  cl_uint cl_uint_arg_indx = arg_indx;
  cl_uint cl_uint_ret;
  std::string string_ret;
  cl_ulong cl_ulong_ret;
  size_t param_value_size_ret_inter = (size_t) -1;
  std::vector<bool> return_variable_null_status;

  return_variable_null_status.resize(2);
  return_variable_null_status[0] = return_variable_null_status[1] = false;

  // The Sync Message can't get value back by NULL ptr, so if a
  // return back ptr is NULL, we must instead it using another
  // no-NULL ptr.
  if (param_value_size_ret == NULL) {
    param_value_size_ret = &param_value_size_ret_inter;
    return_variable_null_status[0] = true;
  }

  if (NULL == param_value)
    return_variable_null_status[1] = true;

  switch(param_name) {
    case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
    case CL_KERNEL_ARG_ACCESS_QUALIFIER: {
      // Send a Sync IPC Message and wait for the results.
      if (!Send(new OpenCLChannelMsg_GetKernelArgInfo_cl_uint(
               point_kernel,
               cl_uint_arg_indx,
               param_name,
               param_value_size,
               return_variable_null_status,
               &cl_uint_ret,
               param_value_size_ret,
               &errcode_ret))) {
        return CL_SEND_IPC_MESSAGE_FAILURE;
      }

      // Dump the results of the Sync IPC Message calling.
      if (CL_SUCCESS == errcode_ret)
        *(cl_uint*) param_value = cl_uint_ret;
      return errcode_ret;
    }
    case CL_KERNEL_ARG_TYPE_NAME:
    case CL_KERNEL_ARG_NAME: {
      // Send a Sync IPC Message and wait for the results.
      if (!Send(new OpenCLChannelMsg_GetKernelArgInfo_string(
               point_kernel,
               cl_uint_arg_indx,
               param_name,
               param_value_size,
               return_variable_null_status,
               &string_ret,
               param_value_size_ret,
               &errcode_ret))) {
        return CL_SEND_IPC_MESSAGE_FAILURE;
      }

      // Dump the results of the Sync IPC Message calling.
      if (CL_SUCCESS == errcode_ret)
        strcpy((char*)param_value,string_ret.c_str());

      return errcode_ret;
    }
    case CL_KERNEL_ARG_TYPE_QUALIFIER: {
      // Send a Sync IPC Message and wait for the results.
      if (!Send(new OpenCLChannelMsg_GetKernelArgInfo_cl_ulong(
               point_kernel,
               cl_uint_arg_indx,
               param_name,
               param_value_size,
               return_variable_null_status,
               &cl_ulong_ret,
               param_value_size_ret,
               &errcode_ret))) {
        return CL_SEND_IPC_MESSAGE_FAILURE;
      }

      // Dump the results of the Sync IPC Message calling.
      if (CL_SUCCESS == errcode_ret)
        *(cl_ulong*) param_value = cl_ulong_ret;

      return errcode_ret;
    }
    default: return CL_SEND_IPC_MESSAGE_FAILURE;
  }
}

cl_int GpuChannelHost::webcl_clReleaseKernel(cl_kernel kernel) {
	// Sending a Sync IPC Message, to call a clReleaseKernel
	// API in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_kernel = (cl_point) kernel;

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_ReleaseKernel(
					 point_kernel,
					 &errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}
	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clGetProgramInfo(
		cl_program program,
		cl_program_info param_name,
		size_t param_value_size,
		void *param_value,
		size_t *param_value_size_ret) {
	// Sending a Sync IPC Message, to call a clCreateSubDevices API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_program = (cl_point) program;
	cl_uint cl_uint_ret;
	cl_point cl_point_ret;
	std::vector<cl_point> cl_point_list_ret;
	std::string string_ret;
	std::vector<size_t> size_t_list_ret;
	std::vector<std::string> string_list_ret;
	size_t size_t_ret;
	size_t param_value_size_ret_inter = (size_t) -1;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(2);
	return_variable_null_status[0] = return_variable_null_status[1] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (param_value_size_ret == NULL) {
		param_value_size_ret = &param_value_size_ret_inter;
		return_variable_null_status[0] = true;
	}

	if (NULL == param_value)
		return_variable_null_status[1] = true;

	switch(param_name) {
		case CL_PROGRAM_REFERENCE_COUNT:
		case CL_PROGRAM_NUM_DEVICES: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramInfo_cl_uint(
							 point_program,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_uint_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_uint*) param_value = cl_uint_ret;

			return errcode_ret;
		}
		case CL_PROGRAM_CONTEXT: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramInfo_cl_point(
							 point_program,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_point_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_context*) param_value = (cl_context) cl_point_ret;

			return errcode_ret;
		}
		case CL_PROGRAM_DEVICES: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramInfo_cl_point_list(
							 point_program,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_point_list_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				for (cl_uint index = 0; index < param_value_size/sizeof(cl_point); ++index)
					((cl_device_id*) (param_value))[index] = (cl_device_id)cl_point_list_ret[index];

			return errcode_ret;
		}
		case CL_PROGRAM_SOURCE:
		case CL_PROGRAM_KERNEL_NAMES: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramInfo_string(
							 point_program,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &string_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				strcpy((char*)param_value,string_ret.c_str());

			return errcode_ret;
		}
		case CL_PROGRAM_BINARY_SIZES: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramInfo_size_t_list(
							 point_program,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &size_t_list_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				for (cl_uint index = 0; index < param_value_size/sizeof(size_t); ++index)
					((size_t*) (param_value))[index] = size_t_list_ret[index];

			return errcode_ret;
		}
		case CL_PROGRAM_BINARIES: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramInfo_string_list(
							 point_program,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &string_list_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
			for (cl_uint index = 0; index < param_value_size/sizeof(std::string); ++index)
				strcpy(((char **)(param_value))[index],string_list_ret[index].c_str());

			return errcode_ret;
		}
		case CL_PROGRAM_NUM_KERNELS: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramInfo_size_t(
							 point_program,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &size_t_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(size_t*) param_value = size_t_ret;

			return errcode_ret;
		}
		default:return CL_SEND_IPC_MESSAGE_FAILURE;
	}
}

cl_program GpuChannelHost::webcl_clCreateProgramWithSource(
		cl_context context,
		cl_uint count,
		const char **strings,
		const size_t *lengths,
		cl_int *errcode_ret) {
	// Sending a Sync IPC Message, to call a clCreateProgramWithSource API
	// in other process, and getting the results of the API.
	cl_int errcode_ret_inter;
	cl_point point_program_ret;
	cl_point point_context = (cl_point) context;
	std::vector<std::string> string_list;
	std::vector<size_t> length_list;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(1);
	return_variable_null_status[0] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (NULL == errcode_ret) {
		errcode_ret = &errcode_ret_inter;
		return_variable_null_status[0] = true;
	}

	// Dump the inputs of the Sync IPC Message calling.
	string_list.clear();
	if (NULL != strings) {
		for (cl_uint index = 0; index < count; ++index)
			string_list.push_back(std::string(strings[index]));
	}

	length_list.clear();
	if (NULL != lengths) {
		for (cl_uint index = 0; index < count; ++index)
			length_list.push_back(lengths[index]);
	}

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_CreateProgramWithSource(
					 point_context,
					 count,
					 string_list,
					 length_list,
					 return_variable_null_status,
					 errcode_ret,
					 &point_program_ret))) {
		return NULL;
	}
	return (cl_program) point_program_ret;
}

cl_int GpuChannelHost::webcl_clGetProgramBuildInfo(
		cl_program program,
		cl_device_id device,
		cl_program_build_info param_name,
		size_t param_value_size,
		void *param_value,
		size_t *param_value_size_ret) {
	// Sending a Sync IPC Message, to call a clCreateSubDevices API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_program = (cl_point) program;
	cl_point point_device  = (cl_point) device;
	cl_int cl_int_ret;
	std::string string_ret;
	cl_uint cl_uint_ret;
	size_t param_value_size_ret_inter = (size_t) -1;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(2);
	return_variable_null_status[0] = return_variable_null_status[1] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (param_value_size_ret == NULL) {
		param_value_size_ret = &param_value_size_ret_inter;
		return_variable_null_status[0] = true;
	}

	if (NULL == param_value)
		return_variable_null_status[1] = true;

	switch(param_name) {
		case CL_PROGRAM_BUILD_STATUS: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramBuildInfo_cl_int(
							 point_program,
							 point_device,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_int_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_int*) param_value = cl_int_ret;

			return errcode_ret;
		}
		case CL_PROGRAM_BUILD_OPTIONS:
		case CL_PROGRAM_BUILD_LOG: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramBuildInfo_string(
							 point_program,
							 point_device,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &string_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				strcpy((char*) param_value,string_ret.c_str());

			return errcode_ret;
		}
		case CL_PROGRAM_BINARY_TYPE: {
			// Send a Sync IPC Message and wait for the results.
			if (!Send(new OpenCLChannelMsg_GetProgramBuildInfo_cl_uint(
							 point_program,
							 point_device,
							 param_name,
							 param_value_size,
							 return_variable_null_status,
							 &cl_uint_ret,
							 param_value_size_ret,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
			}

			// Dump the results of the Sync IPC Message calling.
			if (CL_SUCCESS == errcode_ret)
				*(cl_program_binary_type*) param_value = (cl_program_binary_type) cl_uint_ret;

			return errcode_ret;
		}
		default: return CL_SEND_IPC_MESSAGE_FAILURE;
	}
}

cl_int GpuChannelHost::webcl_clBuildProgram(
		cl_program program,
		cl_uint num_devices,
		const cl_device_id* device_list,
		const char* options,
		cl_point event_key,
		unsigned handler_key,
		unsigned object_type)
{
	cl_int errcode_ret;
	cl_point point_program = (cl_point)program;
	cl_uint num_devices_inter = num_devices;

	std::vector<cl_point> vector_device_list;

	for(unsigned i=0; i<num_devices; i++) {
		vector_device_list.push_back((cl_point)device_list[i]);
	}

	std::string options_string = std::string(options);
	std::vector<cl_point> key_list;
	key_list.push_back(event_key);
	key_list.push_back(handler_key);
	key_list.push_back(object_type);

	if (!Send(new OpenCLChannelMsg_BuildProgram(
							 point_program,
							 num_devices_inter,
							 vector_device_list,
							 options_string,
							 key_list,
							 &errcode_ret))) {
				return CL_SEND_IPC_MESSAGE_FAILURE;
	}
	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clEnqueueMarker(
		cl_command_queue command_queue,
		cl_event* event
)
{
	cl_int errcode_ret;
	cl_point point_command_queue = (cl_point)command_queue;
	cl_point event_ret = 0;

	if (!Send(new OpenCLChannelMsg_EnqueueMarker(
			point_command_queue,
			&event_ret,
			&errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	if(event != nullptr && event_ret != 0) {
		*event = (cl_event)event_ret;
	}

	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clEnqueueBarrier(
		cl_command_queue command_queue
)
{
	cl_int errcode_ret;
	cl_point point_command_queue = (cl_point)command_queue;

	if (!Send(new OpenCLChannelMsg_EnqueueBarrier(
			point_command_queue,
			&errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clEnqueueWaitForEvents(
		cl_command_queue command_queue,
		cl_uint num_events,
		const cl_event* event_list
)
{
	cl_int errcode_ret;
	cl_point point_command_queue = (cl_point)command_queue;
	std::vector<cl_point> point_event_list;

	for(unsigned i=0; i<num_events; i++) {
		point_event_list.push_back((cl_point)event_list[i]);
	}

	if (!Send(new OpenCLChannelMsg_EnqueueWaitForEvents(
			point_command_queue,
			point_event_list,
			num_events,
			&errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	return errcode_ret;
}

cl_kernel GpuChannelHost::webcl_clCreateKernel(
		cl_program program,
		const char *kernel_name,
		cl_int *errcode_ret) {
	// Sending a Sync IPC Message, to call a clCreateKernel API
	// in other process, and getting the results of the API.
	cl_int errcode_ret_inter = 0xFFFFFFF;
	cl_point point_kernel_ret;
	cl_point point_program = (cl_point) program;
	std::string str_kernel_name(kernel_name);
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(1);
	return_variable_null_status[0] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (NULL == errcode_ret) {
		errcode_ret = &errcode_ret_inter;
		return_variable_null_status[0] = true;
	}

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_CreateKernel(
					point_program,
					str_kernel_name,
					return_variable_null_status,
					errcode_ret,
					&point_kernel_ret))) {
				return NULL;
	}
	return (cl_kernel) point_kernel_ret;
}

cl_int GpuChannelHost::webcl_clCreateKernelsInProgram(
		cl_program program,
		cl_uint num_kernels,
		cl_kernel *kernels,
		cl_uint *num_kernels_ret) {
	// Sending a Sync IPC Message, to call a clCreateKernel API
	// in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_uint num_kernels_ret_inter = (cl_uint) -1;
	cl_point point_program = (cl_point) program;
	std::vector<cl_point> point_kernel_list;
	std::vector<bool> return_variable_null_status;

	return_variable_null_status.resize(1);
	return_variable_null_status[0] = false;

	// The Sync Message can't get value back by NULL ptr, so if a
	// return back ptr is NULL, we must instead it using another
	// no-NULL ptr.
	if (NULL == num_kernels_ret) {
		num_kernels_ret = &num_kernels_ret_inter;
		return_variable_null_status[0] = true;
	}

	// Dump the inputs of the Sync IPC Message calling.
	point_kernel_list.clear();

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_CreateKernelsInProgram(
					 point_program,
					 num_kernels,
					 point_kernel_list,
					 return_variable_null_status,
					 &point_kernel_list,
					 num_kernels_ret,
					 &errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}

	if(errcode_ret == CL_SUCCESS && num_kernels > 0) {
		for(size_t i=0; i<num_kernels; i++)
			kernels[i] = (cl_kernel)point_kernel_list[i];
	}

	return errcode_ret;
}

cl_int GpuChannelHost::webcl_clReleaseProgram(cl_program program)
{
	// Sending a Sync IPC Message, to call a clReleaseProgram
	// API in other process, and getting the results of the API.
	cl_int errcode_ret;
	cl_point point_program = (cl_point) program;

	// Send a Sync IPC Message and wait for the results.
	if (!Send(new OpenCLChannelMsg_ReleaseProgram(
					 point_program,
					 &errcode_ret))) {
		return CL_SEND_IPC_MESSAGE_FAILURE;
	}
	return errcode_ret;
}

bool GpuChannelHost::webcl_ctrlSetSharedHandles(
		base::SharedMemoryHandle data_handle,
		base::SharedMemoryHandle operation_handle,
		base::SharedMemoryHandle result_handle,
		base::SharedMemoryHandle events_handle)
{
	bool result_inter = false;
	if (!Send(new OpenCLChannelMsg_CTRL_SetSharedHandles(
					 data_handle,
					 operation_handle,
					 result_handle,
					 events_handle,
					 &result_inter))) {
		return false;
	}
	return result_inter;
}

bool GpuChannelHost::webcl_ctrlClearSharedHandles()
{
	bool result_inter = false;

	if (!Send(new OpenCLChannelMsg_CTRL_ClearSharedHandles(
					 &result_inter))) {
		return false;
	}

	return result_inter;
}


bool GpuChannelHost::webcl_ctrlTriggerSharedOperation(int operation)
{
	switch(operation) {
	case OPENCL_OPERATION_TYPE::SET_ARG:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_setArg())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::CREATE_BUFFER:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_createBuffer())) {
			return false;
		}
		break;
  // gl/cl sharing
  case OPENCL_OPERATION_TYPE::CREATE_BUFFER_FROM_GL_BUFFER:
    if (!Send(new OpenCLChannelMsg_CTRL_Trigger_createBufferFromGLBuffer())) {
      return false;
    }
    break;
  case OPENCL_OPERATION_TYPE::CREATE_IMAGE_FROM_GL_RENDER_BUFFER:
    if (!Send(new OpenCLChannelMsg_CTRL_Trigger_createImageFromGLRenderbuffer())) {
      return false;
    }
    break;
  case OPENCL_OPERATION_TYPE::CREATE_IMAGE_FROM_GL_TEXTURE:
    if (!Send(new OpenCLChannelMsg_CTRL_Trigger_createImageFromGLTexture())) {
      return false;
    }
    break;
	case OPENCL_OPERATION_TYPE::CREATE_IMAGE:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_createImage())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueCopyBuffer())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER_RECT:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueCopyBufferRect())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER_TO_IMAGE:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueCopyBufferToImage())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_IMAGE_TO_BUFFER:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueCopyImageToBuffer())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_IMAGE:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueCopyImage())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueReadBuffer())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER_RECT:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueReadBufferRect())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_READ_IMAGE:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueReadImage())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueWriteBuffer())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER_RECT:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueWriteBufferRect())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_IMAGE:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueWriteImage())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_NDRANGE_KERNEL:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueNDRangeKernel())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::FINISH:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_finish())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::GET_GL_OBJECT_INFO:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_getGLObjectInfo())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_ACQUIRE_GLOBJECTS:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueAcquireGLObjects())) {
			return false;
		}
		break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_RELEASE_GLOBJECTS:
		if (!Send(new OpenCLChannelMsg_CTRL_Trigger_enqueueReleaseGLObjects())) {
			return false;
		}
		break;
  }
	return true;
}
#endif
//if enable_highweb_webcl build flag


}  // namespace gpu
