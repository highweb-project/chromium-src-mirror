// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/ipc/service/gpu_channel.h"

#include <utility>

#if defined(OS_WIN)
#include <windows.h>
#endif

#include <algorithm>
#include <deque>
#include <set>
#include <vector>

#include "base/atomicops.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/location.h"
#include "base/numerics/safe_conversions.h"
#include "base/single_thread_task_runner.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/timer/timer.h"
#include "base/trace_event/memory_dump_manager.h"
#include "base/trace_event/process_memory_dump.h"
#include "base/trace_event/trace_event.h"
#include "build/build_config.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/service/command_executor.h"
#include "gpu/command_buffer/service/image_factory.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/sync_point_manager.h"
#include "gpu/ipc/common/gpu_messages.h"
#include "gpu/ipc/service/gpu_channel_manager.h"
#include "gpu/ipc/service/gpu_channel_manager_delegate.h"
#include "gpu/ipc/service/gpu_memory_buffer_factory.h"
#include "ipc/ipc_channel.h"
#include "ipc/message_filter.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_image_shared_memory.h"
#include "ui/gl/gl_surface.h"

// gl/cl sharing
#include "ui/gl/gl_context_egl.h"
#include "gpu/command_buffer/service/buffer_manager.h"
#include "gpu/command_buffer/service/renderbuffer_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"

#include "ui/opencl/opencl_implementation.h"

//vulkan implementation
#include "ui/native_vulkan/vulkan_implementation.h"

#if defined(OS_POSIX)
#include "ipc/ipc_channel_posix.h"
#endif

namespace gpu {
namespace {

// Number of milliseconds between successive vsync. Many GL commands block
// on vsync, so thresholds for preemption should be multiples of this.
const int64_t kVsyncIntervalMs = 17;

// Amount of time that we will wait for an IPC to be processed before
// preempting. After a preemption, we must wait this long before triggering
// another preemption.
const int64_t kPreemptWaitTimeMs = 2 * kVsyncIntervalMs;

// Once we trigger a preemption, the maximum duration that we will wait
// before clearing the preemption.
const int64_t kMaxPreemptTimeMs = kVsyncIntervalMs;

// Stop the preemption once the time for the longest pending IPC drops
// below this threshold.
const int64_t kStopPreemptThresholdMs = kVsyncIntervalMs;

}  // anonymous namespace

scoped_refptr<GpuChannelMessageQueue> GpuChannelMessageQueue::Create(
    int32_t stream_id,
    GpuStreamPriority stream_priority,
    GpuChannel* channel,
    const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner,
    const scoped_refptr<PreemptionFlag>& preempting_flag,
    const scoped_refptr<PreemptionFlag>& preempted_flag,
    SyncPointManager* sync_point_manager) {
  return new GpuChannelMessageQueue(stream_id, stream_priority, channel,
                                    io_task_runner, preempting_flag,
                                    preempted_flag, sync_point_manager);
}

scoped_refptr<SyncPointOrderData>
GpuChannelMessageQueue::GetSyncPointOrderData() {
  return sync_point_order_data_;
}

GpuChannelMessageQueue::GpuChannelMessageQueue(
    int32_t stream_id,
    GpuStreamPriority stream_priority,
    GpuChannel* channel,
    const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner,
    const scoped_refptr<PreemptionFlag>& preempting_flag,
    const scoped_refptr<PreemptionFlag>& preempted_flag,
    SyncPointManager* sync_point_manager)
    : stream_id_(stream_id),
      stream_priority_(stream_priority),
      enabled_(true),
      scheduled_(true),
      channel_(channel),
      preemption_state_(IDLE),
      max_preemption_time_(
          base::TimeDelta::FromMilliseconds(kMaxPreemptTimeMs)),
      timer_(new base::OneShotTimer),
      sync_point_order_data_(SyncPointOrderData::Create()),
      io_task_runner_(io_task_runner),
      preempting_flag_(preempting_flag),
      preempted_flag_(preempted_flag),
      sync_point_manager_(sync_point_manager) {
  timer_->SetTaskRunner(io_task_runner);
  io_thread_checker_.DetachFromThread();
}

GpuChannelMessageQueue::~GpuChannelMessageQueue() {
  DCHECK(!enabled_);
  DCHECK(channel_messages_.empty());
}

void GpuChannelMessageQueue::Disable() {
  {
    base::AutoLock auto_lock(channel_lock_);
    DCHECK(enabled_);
    enabled_ = false;
  }

  // We guarantee that the queues will no longer be modified after enabled_
  // is set to false, it is now safe to modify the queue without the lock.
  // All public facing modifying functions check enabled_ while all
  // private modifying functions DCHECK(enabled_) to enforce this.
  while (!channel_messages_.empty()) {
    const IPC::Message& msg = channel_messages_.front()->message;
    if (msg.is_sync()) {
      IPC::Message* reply = IPC::SyncMessage::GenerateReply(&msg);
      reply->set_reply_error();
      channel_->Send(reply);
    }
    channel_messages_.pop_front();
  }

  sync_point_order_data_->Destroy();
  sync_point_order_data_ = nullptr;

  io_task_runner_->PostTask(
      FROM_HERE, base::Bind(&GpuChannelMessageQueue::DisableIO, this));
}

void GpuChannelMessageQueue::DisableIO() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  timer_ = nullptr;
}

bool GpuChannelMessageQueue::IsScheduled() const {
  base::AutoLock lock(channel_lock_);
  return scheduled_;
}

void GpuChannelMessageQueue::OnRescheduled(bool scheduled) {
  base::AutoLock lock(channel_lock_);
  DCHECK(enabled_);
  if (scheduled_ == scheduled)
    return;
  scheduled_ = scheduled;
  if (scheduled)
    channel_->PostHandleMessage(this);
  if (preempting_flag_) {
    io_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&GpuChannelMessageQueue::UpdatePreemptionState, this));
  }
}

uint32_t GpuChannelMessageQueue::GetUnprocessedOrderNum() const {
  return sync_point_order_data_->unprocessed_order_num();
}

uint32_t GpuChannelMessageQueue::GetProcessedOrderNum() const {
  return sync_point_order_data_->processed_order_num();
}

bool GpuChannelMessageQueue::PushBackMessage(const IPC::Message& message) {
  base::AutoLock auto_lock(channel_lock_);
  if (enabled_) {
    if (message.type() == GpuCommandBufferMsg_WaitForTokenInRange::ID ||
        message.type() == GpuCommandBufferMsg_WaitForGetOffsetInRange::ID) {
      channel_->PostHandleOutOfOrderMessage(message);
      return true;
    }

    uint32_t order_num = sync_point_order_data_->GenerateUnprocessedOrderNumber(
        sync_point_manager_);
    std::unique_ptr<GpuChannelMessage> msg(
        new GpuChannelMessage(message, order_num, base::TimeTicks::Now()));

    if (channel_messages_.empty()) {
      DCHECK(scheduled_);
      channel_->PostHandleMessage(this);
    }

    channel_messages_.push_back(std::move(msg));

    if (preempting_flag_)
      UpdatePreemptionStateHelper();

    return true;
  }
  return false;
}

const GpuChannelMessage* GpuChannelMessageQueue::BeginMessageProcessing() {
  base::AutoLock auto_lock(channel_lock_);
  DCHECK(enabled_);
  // If we have been preempted by another channel, just post a task to wake up.
  if (preempted_flag_ && preempted_flag_->IsSet()) {
    channel_->PostHandleMessage(this);
    return nullptr;
  }
  if (channel_messages_.empty())
    return nullptr;
  sync_point_order_data_->BeginProcessingOrderNumber(
      channel_messages_.front()->order_number);
  return channel_messages_.front().get();
}

void GpuChannelMessageQueue::PauseMessageProcessing() {
  base::AutoLock auto_lock(channel_lock_);
  DCHECK(!channel_messages_.empty());

  // If we have been preempted by another channel, just post a task to wake up.
  if (scheduled_)
    channel_->PostHandleMessage(this);

  sync_point_order_data_->PauseProcessingOrderNumber(
      channel_messages_.front()->order_number);
}

void GpuChannelMessageQueue::FinishMessageProcessing() {
  base::AutoLock auto_lock(channel_lock_);
  DCHECK(!channel_messages_.empty());
  DCHECK(scheduled_);

  sync_point_order_data_->FinishProcessingOrderNumber(
      channel_messages_.front()->order_number);
  channel_messages_.pop_front();

  if (!channel_messages_.empty())
    channel_->PostHandleMessage(this);

  if (preempting_flag_) {
    io_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&GpuChannelMessageQueue::UpdatePreemptionState, this));
  }
}

void GpuChannelMessageQueue::UpdatePreemptionState() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  base::AutoLock lock(channel_lock_);
  UpdatePreemptionStateHelper();
}

void GpuChannelMessageQueue::UpdatePreemptionStateHelper() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  switch (preemption_state_) {
    case IDLE:
      UpdateStateIdle();
      break;
    case WAITING:
      UpdateStateWaiting();
      break;
    case CHECKING:
      UpdateStateChecking();
      break;
    case PREEMPTING:
      UpdateStatePreempting();
      break;
    case WOULD_PREEMPT_DESCHEDULED:
      UpdateStateWouldPreemptDescheduled();
      break;
    default:
      NOTREACHED();
  }
}

void GpuChannelMessageQueue::UpdateStateIdle() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  DCHECK(!timer_->IsRunning());
  if (!channel_messages_.empty())
    TransitionToWaiting();
}

void GpuChannelMessageQueue::UpdateStateWaiting() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  // Transition to CHECKING if timer fired.
  if (!timer_->IsRunning())
    TransitionToChecking();
}

void GpuChannelMessageQueue::UpdateStateChecking() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  if (!channel_messages_.empty()) {
    base::TimeTicks time_recv = channel_messages_.front()->time_received;
    base::TimeDelta time_elapsed = base::TimeTicks::Now() - time_recv;
    if (time_elapsed.InMilliseconds() < kPreemptWaitTimeMs) {
      // Schedule another check for when the IPC may go long.
      timer_->Start(
          FROM_HERE,
          base::TimeDelta::FromMilliseconds(kPreemptWaitTimeMs) - time_elapsed,
          this, &GpuChannelMessageQueue::UpdatePreemptionState);
    } else {
      timer_->Stop();
      if (!scheduled_)
        TransitionToWouldPreemptDescheduled();
      else
        TransitionToPreempting();
    }
  }
}

void GpuChannelMessageQueue::UpdateStatePreempting() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  // We should stop preempting if the timer fired or for other conditions.
  if (!timer_->IsRunning() || ShouldTransitionToIdle()) {
    TransitionToIdle();
  } else if (!scheduled_) {
    // Save the remaining preemption time before stopping the timer.
    max_preemption_time_ = timer_->desired_run_time() - base::TimeTicks::Now();
    timer_->Stop();
    TransitionToWouldPreemptDescheduled();
  }
}

void GpuChannelMessageQueue::UpdateStateWouldPreemptDescheduled() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  DCHECK(!timer_->IsRunning());
  if (ShouldTransitionToIdle()) {
    TransitionToIdle();
  } else if (scheduled_) {
    TransitionToPreempting();
  }
}

bool GpuChannelMessageQueue::ShouldTransitionToIdle() const {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  DCHECK(preemption_state_ == PREEMPTING ||
         preemption_state_ == WOULD_PREEMPT_DESCHEDULED);
  if (channel_messages_.empty()) {
    return true;
  } else {
    base::TimeTicks next_tick = channel_messages_.front()->time_received;
    base::TimeDelta time_elapsed = base::TimeTicks::Now() - next_tick;
    if (time_elapsed.InMilliseconds() < kStopPreemptThresholdMs)
      return true;
  }
  return false;
}

void GpuChannelMessageQueue::TransitionToIdle() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  DCHECK(preemption_state_ == PREEMPTING ||
         preemption_state_ == WOULD_PREEMPT_DESCHEDULED);

  preemption_state_ = IDLE;
  preempting_flag_->Reset();

  max_preemption_time_ = base::TimeDelta::FromMilliseconds(kMaxPreemptTimeMs);
  timer_->Stop();

  TRACE_COUNTER_ID1("gpu", "GpuChannel::Preempting", this, 0);

  UpdateStateIdle();
}

void GpuChannelMessageQueue::TransitionToWaiting() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  DCHECK_EQ(preemption_state_, IDLE);
  DCHECK(!timer_->IsRunning());

  preemption_state_ = WAITING;

  timer_->Start(FROM_HERE,
                base::TimeDelta::FromMilliseconds(kPreemptWaitTimeMs), this,
                &GpuChannelMessageQueue::UpdatePreemptionState);
}

void GpuChannelMessageQueue::TransitionToChecking() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  DCHECK_EQ(preemption_state_, WAITING);
  DCHECK(!timer_->IsRunning());

  preemption_state_ = CHECKING;

  UpdateStateChecking();
}

void GpuChannelMessageQueue::TransitionToPreempting() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  DCHECK(preemption_state_ == CHECKING ||
         preemption_state_ == WOULD_PREEMPT_DESCHEDULED);
  DCHECK(scheduled_);

  preemption_state_ = PREEMPTING;
  preempting_flag_->Set();
  TRACE_COUNTER_ID1("gpu", "GpuChannel::Preempting", this, 1);

  DCHECK_LE(max_preemption_time_,
            base::TimeDelta::FromMilliseconds(kMaxPreemptTimeMs));
  timer_->Start(FROM_HERE, max_preemption_time_, this,
                &GpuChannelMessageQueue::UpdatePreemptionState);
}

void GpuChannelMessageQueue::TransitionToWouldPreemptDescheduled() {
  DCHECK(io_thread_checker_.CalledOnValidThread());
  DCHECK(preempting_flag_);
  channel_lock_.AssertAcquired();
  DCHECK(preemption_state_ == CHECKING || preemption_state_ == PREEMPTING);
  DCHECK(!scheduled_);

  preemption_state_ = WOULD_PREEMPT_DESCHEDULED;
  preempting_flag_->Reset();
  TRACE_COUNTER_ID1("gpu", "GpuChannel::Preempting", this, 0);
}

GpuChannelMessageFilter::GpuChannelMessageFilter()
    : sender_(nullptr), peer_pid_(base::kNullProcessId) {}

GpuChannelMessageFilter::~GpuChannelMessageFilter() {}

void GpuChannelMessageFilter::OnFilterAdded(IPC::Sender* sender) {
  DCHECK(!sender_);
  sender_ = sender;
  for (scoped_refptr<IPC::MessageFilter>& filter : channel_filters_) {
    filter->OnFilterAdded(sender_);
  }
}

void GpuChannelMessageFilter::OnFilterRemoved() {
  DCHECK(sender_);
  for (scoped_refptr<IPC::MessageFilter>& filter : channel_filters_) {
    filter->OnFilterRemoved();
  }
  sender_ = nullptr;
  peer_pid_ = base::kNullProcessId;
}

void GpuChannelMessageFilter::OnChannelConnected(int32_t peer_pid) {
  DCHECK(peer_pid_ == base::kNullProcessId);
  peer_pid_ = peer_pid;
  for (scoped_refptr<IPC::MessageFilter>& filter : channel_filters_) {
    filter->OnChannelConnected(peer_pid);
  }
}

void GpuChannelMessageFilter::OnChannelError() {
  for (scoped_refptr<IPC::MessageFilter>& filter : channel_filters_) {
    filter->OnChannelError();
  }
}

void GpuChannelMessageFilter::OnChannelClosing() {
  for (scoped_refptr<IPC::MessageFilter>& filter : channel_filters_) {
    filter->OnChannelClosing();
  }
}

void GpuChannelMessageFilter::AddChannelFilter(
    scoped_refptr<IPC::MessageFilter> filter) {
  channel_filters_.push_back(filter);
  if (sender_)
    filter->OnFilterAdded(sender_);
  if (peer_pid_ != base::kNullProcessId)
    filter->OnChannelConnected(peer_pid_);
}

void GpuChannelMessageFilter::RemoveChannelFilter(
    scoped_refptr<IPC::MessageFilter> filter) {
  if (sender_)
    filter->OnFilterRemoved();
  channel_filters_.erase(
      std::find(channel_filters_.begin(), channel_filters_.end(), filter));
}

// This gets called from the main thread and assumes that all messages which
// lead to creation of a new route are synchronous messages.
// TODO(sunnyps): Create routes (and streams) on the IO thread so that we can
// make the CreateCommandBuffer/VideoDecoder/VideoEncoder messages asynchronous.
void GpuChannelMessageFilter::AddRoute(
    int32_t route_id,
    const scoped_refptr<GpuChannelMessageQueue>& queue) {
  base::AutoLock lock(routes_lock_);
  routes_.insert(std::make_pair(route_id, queue));
}

void GpuChannelMessageFilter::RemoveRoute(int32_t route_id) {
  base::AutoLock lock(routes_lock_);
  routes_.erase(route_id);
}

bool GpuChannelMessageFilter::OnMessageReceived(const IPC::Message& message) {
  DCHECK(sender_);

  if (message.should_unblock() || message.is_reply())
    return MessageErrorHandler(message, "Unexpected message type");

  if (message.type() == GpuChannelMsg_Nop::ID) {
    IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
    Send(reply);
    return true;
  }

  for (scoped_refptr<IPC::MessageFilter>& filter : channel_filters_) {
    if (filter->OnMessageReceived(message))
      return true;
  }

    bool isNeedFastReplyWebCLMsg = false;
	switch(message.type()) {
	case OpenCLChannelMsg_CTRL_Trigger_enqueueCopyBuffer::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueCopyBuffer();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueCopyBufferRect::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueCopyBufferRect();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueCopyBufferToImage::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueCopyBufferToImage();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueCopyImageToBuffer::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueCopyImageToBuffer();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueCopyImage::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueCopyImage();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueReadBuffer::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueReadBuffer();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueReadBufferRect::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueReadBufferRect();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueReadImage::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueReadImage();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueWriteBuffer::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueWriteBuffer();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueWriteBufferRect::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueWriteBufferRect();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueWriteImage::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueWriteImage();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueNDRangeKernel::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClEnqueueNDRangeKernel();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_finish::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClFinish();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_setArg::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClSetKernelArg();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_createBuffer::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClCreateBuffer();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_createImage::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClCreateImage2D();
		break;
	// gl/cl sharing
	case OpenCLChannelMsg_CTRL_Trigger_createBufferFromGLBuffer::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doClCreateBufferFromGLBuffer();
		break;
  case OpenCLChannelMsg_CTRL_Trigger_createImageFromGLRenderbuffer::ID:
    isNeedFastReplyWebCLMsg = true;
    cl_api_->doClCreateImageFromGLRenderbuffer();
    break;
  case OpenCLChannelMsg_CTRL_Trigger_createImageFromGLTexture::ID:
    isNeedFastReplyWebCLMsg = true;
    cl_api_->doClCreateImageFromGLTexture();
    break;
	case OpenCLChannelMsg_CTRL_Trigger_getGLObjectInfo::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doGetGLObjectInfo();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueAcquireGLObjects::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doEnqueueAcquireGLObjects();
		break;
	case OpenCLChannelMsg_CTRL_Trigger_enqueueReleaseGLObjects::ID:
		isNeedFastReplyWebCLMsg = true;
		cl_api_->doEnqueueReleaseGLObjects();
		break;
	}

	if(isNeedFastReplyWebCLMsg) {
		IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
		Send(reply);
		return true;
	}

  //vulkan
  bool isNeedFastReplyWebVKCMsg = false;
  switch(message.type()) {
    case VulkanComputeChannelMsg_Trigger_WriteBuffer::ID:
      isNeedFastReplyWebVKCMsg = true;
      vkc_api_->vkcWriteBuffer();
      break;
    case VulkanComputeChannelMsg_Trigger_ReadBuffer::ID:
      isNeedFastReplyWebVKCMsg = true;
      vkc_api_->vkcReadBuffer();
      break;
  }
  if(isNeedFastReplyWebVKCMsg) {
    IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
    Send(reply);
    return true;
  }

  scoped_refptr<GpuChannelMessageQueue> message_queue =
      LookupStreamByRoute(message.routing_id());

  if (!message_queue)
    return MessageErrorHandler(message, "Could not find message queue");

  if (!message_queue->PushBackMessage(message))
    return MessageErrorHandler(message, "Channel destroyed");

  return true;
}

bool GpuChannelMessageFilter::Send(IPC::Message* message) {
  return sender_->Send(message);
}

scoped_refptr<GpuChannelMessageQueue>
GpuChannelMessageFilter::LookupStreamByRoute(int32_t route_id) {
  base::AutoLock lock(routes_lock_);
  auto it = routes_.find(route_id);
  if (it != routes_.end())
    return it->second;
  return nullptr;
}

bool GpuChannelMessageFilter::MessageErrorHandler(const IPC::Message& message,
                                                  const char* error_msg) {
  DLOG(ERROR) << error_msg;
  if (message.is_sync()) {
    IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
    reply->set_reply_error();
    Send(reply);
  }
  return true;
}

GpuChannel::GpuChannel(GpuChannelManager* gpu_channel_manager,
                       SyncPointManager* sync_point_manager,
                       GpuWatchdog* watchdog,
                       gl::GLShareGroup* share_group,
                       gles2::MailboxManager* mailbox,
                       PreemptionFlag* preempting_flag,
                       PreemptionFlag* preempted_flag,
                       base::SingleThreadTaskRunner* task_runner,
                       base::SingleThreadTaskRunner* io_task_runner,
                       int32_t client_id,
                       uint64_t client_tracing_id,
                       bool allow_view_command_buffers,
                       bool allow_real_time_streams)
    : gpu_channel_manager_(gpu_channel_manager),
      sync_point_manager_(sync_point_manager),
      unhandled_message_listener_(nullptr),
      channel_id_(IPC::Channel::GenerateVerifiedChannelID("gpu")),
      preempting_flag_(preempting_flag),
      preempted_flag_(preempted_flag),
      client_id_(client_id),
      client_tracing_id_(client_tracing_id),
      task_runner_(task_runner),
      io_task_runner_(io_task_runner),
      share_group_(share_group),
      mailbox_manager_(mailbox),
      watchdog_(watchdog),
      allow_view_command_buffers_(allow_view_command_buffers),
      allow_real_time_streams_(allow_real_time_streams),
      weak_factory_(this) {
  DCHECK(gpu_channel_manager);
  DCHECK(client_id);

  filter_ = new GpuChannelMessageFilter();

  scoped_refptr<GpuChannelMessageQueue> control_queue =
      CreateStream(GPU_STREAM_DEFAULT, GpuStreamPriority::HIGH);
  AddRouteToStream(MSG_ROUTING_CONTROL, GPU_STREAM_DEFAULT);
  clApiImpl = new gfx::CLApi();
  vkcApiImpl = new gfx::VKCApi();
#if defined(OS_LINUX)
  gfx::CLApi::parent_channel_ = this;
  gfx::VKCApi::parent_channel_ = this;
#elif defined(OS_ANDROID)
  clApiImpl->setChannel(this);
  vkcApiImpl->setChannel(this);
#endif
  filter_->setCLApi(clApiImpl);
  filter_->setVKCApi(vkcApiImpl);

  CLLOG(INFO) << "client_id_ : " << client_id_ << "channel_id_ : " << channel_id_ << ", clApiImpl : " << clApiImpl << ", GpuChannel : " << this;

  gfx::InitializeStaticCLBindings(clApiImpl);
  gfx::InitializeStaticVKCBindings(vkcApiImpl);
}

GpuChannel::~GpuChannel() {
  // Clear stubs first because of dependencies.
  stubs_.clear();

  for (auto& kv : streams_)
    kv.second->Disable();

  if (preempting_flag_.get())
    preempting_flag_->Reset();
}

IPC::ChannelHandle GpuChannel::Init(base::WaitableEvent* shutdown_event) {
  DCHECK(shutdown_event);
  DCHECK(!channel_);

  IPC::ChannelHandle channel_handle(channel_id_);

  channel_ =
      IPC::SyncChannel::Create(channel_handle, IPC::Channel::MODE_SERVER, this,
                               io_task_runner_, false, shutdown_event);

#if defined(OS_POSIX)
  // On POSIX, pass the renderer-side FD. Also mark it as auto-close so
  // that it gets closed after it has been sent.
  base::ScopedFD renderer_fd = channel_->TakeClientFileDescriptor();
  DCHECK(renderer_fd.is_valid());
  channel_handle.socket = base::FileDescriptor(std::move(renderer_fd));
#endif

  channel_->AddFilter(filter_.get());

  return channel_handle;
}

void GpuChannel::SetUnhandledMessageListener(IPC::Listener* listener) {
  unhandled_message_listener_ = listener;
}

base::WeakPtr<GpuChannel> GpuChannel::AsWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

base::ProcessId GpuChannel::GetClientPID() const {
  return channel_->GetPeerPID();
}

uint32_t GpuChannel::GetProcessedOrderNum() const {
  uint32_t processed_order_num = 0;
  for (auto& kv : streams_) {
    processed_order_num =
        std::max(processed_order_num, kv.second->GetProcessedOrderNum());
  }
  return processed_order_num;
}

uint32_t GpuChannel::GetUnprocessedOrderNum() const {
  uint32_t unprocessed_order_num = 0;
  for (auto& kv : streams_) {
    unprocessed_order_num =
        std::max(unprocessed_order_num, kv.second->GetUnprocessedOrderNum());
  }
  return unprocessed_order_num;
}

bool GpuChannel::OnMessageReceived(const IPC::Message& msg) {
  // All messages should be pushed to channel_messages_ and handled separately.
  NOTREACHED();
  return false;
}

void GpuChannel::OnChannelError() {
  gpu_channel_manager_->RemoveChannel(client_id_);
}

bool GpuChannel::Send(IPC::Message* message) {
  // The GPU process must never send a synchronous IPC message to the renderer
  // process. This could result in deadlock.
  DCHECK(!message->is_sync());

  DVLOG(1) << "sending message @" << message << " on channel @" << this
           << " with type " << message->type();

  if (!channel_) {
    delete message;
    return false;
  }

  return channel_->Send(message);
}

void GpuChannel::OnStreamRescheduled(int32_t stream_id, bool scheduled) {
  scoped_refptr<GpuChannelMessageQueue> queue = LookupStream(stream_id);
  DCHECK(queue);
  queue->OnRescheduled(scheduled);
}

GpuCommandBufferStub* GpuChannel::LookupCommandBuffer(int32_t route_id) {
  return stubs_.get(route_id);
}

unsigned int GpuChannel::LookupGLServiceId(unsigned int resource_id, GLResourceType glResourceType) {
  CLLOG(INFO) << "GpuChannel::LookupGLServiceId, resource_id : " << resource_id << ", glResourceType : " << glResourceType;

  // print entire GL object info
  if(glResourceType == GLResourceType::BUFFER) {
    CLLOG(INFO) << "=================================================================================================================";

    for (auto& kv : stubs_) {
      std::stringstream message;

      const GpuCommandBufferStub* stub = kv.second;
      message << "GpuChannel::LookupGLServiceId, stub : " << stub;

      if(stub) {
        gpu::gles2::GLES2Decoder* decoder = stub->decoder();
        message << ", decoder : " << decoder;

        if(decoder) {
          gpu::gles2::ContextGroup* contextGroup = decoder->GetContextGroup();
          message << ", contextGroup : " << contextGroup;

          if(contextGroup) {
            gpu::gles2::BufferManager* bufferManager = contextGroup->buffer_manager();
            message << ", bufferManager : " << bufferManager;

            if(bufferManager) {
              for(int i=0; i<100; i++) {
                gpu::gles2::Buffer* buffer = bufferManager->GetBuffer(i);

                if(buffer) {
                  GLuint service_id = buffer->service_id();

                  CLLOG(INFO) << message.str() << ", buffer : " << buffer << ", resource_id : " << i << ", service_id : " << service_id;
                }
              }
            }
          }
        }
      }
    }

    CLLOG(INFO) << "=================================================================================================================";
  }

  for (auto& kv : stubs_) {
    const GpuCommandBufferStub* stub = kv.second;
    // DLOG(INFO) << "GpuChannel::LookupGLServiceId, stub : " << stub;

    if(stub) {
      gpu::gles2::GLES2Decoder* decoder = stub->decoder();
      // DLOG(INFO) << "GpuChannel::LookupGLServiceId, decoder : " << decoder;
      if(decoder) {
        gpu::gles2::ContextGroup* contextGroup = decoder->GetContextGroup();
        // DLOG(INFO) << "GpuChannel::LookupGLServiceId, contextGroup : " << contextGroup;
        if(contextGroup) {
          if(glResourceType == GLResourceType::BUFFER) {
            gpu::gles2::BufferManager* bufferManager = contextGroup->buffer_manager();
            // DLOG(INFO) << "GpuChannel::LookupGLServiceId, bufferManager : " << bufferManager;
            if(bufferManager) {
              gpu::gles2::Buffer* buffer = bufferManager->GetBuffer(resource_id);
              // DLOG(INFO) << "GpuChannel::LookupGLServiceId, buffer : " << buffer;
              if(buffer) {
                return buffer->service_id();
              }
            }
          } else if(glResourceType == GLResourceType::RENDERBUFFER) {
            gpu::gles2::RenderbufferManager* renderbufferManager = contextGroup->renderbuffer_manager();
            // DLOG(INFO) << "GpuChannel::LookupGLServiceId, renderbufferManager : " << renderbufferManager;
            if(renderbufferManager) {
              gpu::gles2::Renderbuffer* renderbuffer = renderbufferManager->GetRenderbuffer(resource_id);
              // DLOG(INFO) << "GpuChannel::LookupGLServiceId, renderbuffer : " << renderbuffer;
              if(renderbuffer) {
                return renderbuffer->service_id();
              }
            }
          } else if(glResourceType == GLResourceType::TEXTURE) {
            gpu::gles2::TextureManager* textureManager = contextGroup->texture_manager();
            // DLOG(INFO) << "GpuChannel::LookupGLServiceId, textureManager : " << textureManager;
            if(textureManager) {
              gpu::gles2::TextureRef* textureRef = textureManager->GetTexture(resource_id);
              // DLOG(INFO) << "GpuChannel::LookupGLServiceId, textureRef : " << textureRef;
              if(textureRef) {
                gpu::gles2::Texture* texture = textureRef->texture();
                // DLOG(INFO) << "GpuChannel::LookupGLServiceId, texture : " << texture;
                if(texture) {
                  // GLenum target = texture->target();
                  // DLOG(INFO) << "GpuChannel::LookupGLServiceId, target : " << target;
                  return texture->service_id();
                }
              }
            }
          }
        }
      }
    }
  }

  return 0;
}

void GpuChannel::LoseAllContexts() {
  gpu_channel_manager_->LoseAllContexts();
}

void GpuChannel::MarkAllContextsLost() {
  for (auto& kv : stubs_)
    kv.second->MarkContextLost();
}

bool GpuChannel::AddRoute(int32_t route_id,
                          int32_t stream_id,
                          IPC::Listener* listener) {
  if (router_.AddRoute(route_id, listener)) {
    AddRouteToStream(route_id, stream_id);
    return true;
  }
  return false;
}

void GpuChannel::RemoveRoute(int32_t route_id) {
  router_.RemoveRoute(route_id);
  RemoveRouteFromStream(route_id);
}

bool GpuChannel::OnControlMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(GpuChannel, msg)
    IPC_MESSAGE_HANDLER(GpuChannelMsg_CreateCommandBuffer,
                        OnCreateCommandBuffer)
    IPC_MESSAGE_HANDLER(GpuChannelMsg_DestroyCommandBuffer,
                        OnDestroyCommandBuffer)
    IPC_MESSAGE_HANDLER(GpuChannelMsg_GetDriverBugWorkArounds,
                        OnGetDriverBugWorkArounds)
	//Handle WebCL message
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_getPlatformsIDs,
						OnCallclGetPlatformIDs)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_getPlatformInfo,
						OnCallclGetPlatformInfo)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetDeviceIDs,
						OnCallclGetDeviceIDs)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetDeviceInfo_string,
						OnCallclGetDeviceInfo_string)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetDeviceInfo_cl_uint,
									OnCallclGetDeviceInfo_cl_uint)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetDeviceInfo_size_t_list,
									OnCallclGetDeviceInfo_size_t_list)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetDeviceInfo_size_t,
									OnCallclGetDeviceInfo_size_t)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetDeviceInfo_cl_ulong,
									OnCallclGetDeviceInfo_cl_ulong)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetDeviceInfo_cl_point,
									OnCallclGetDeviceInfo_cl_point)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetDeviceInfo_intptr_t_list,
									OnCallclGetDeviceInfo_intptr_t_list)
	//clCreateContext
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateContext,
									OnCallclCreateContext)
	//clCreateContextFromType
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateContextFromType,
									OnCallclCreateContextFromType)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_WaitForEvents,
									OnCallclWaitForevents)
	//clGetMemObjectInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetMemObjectInfo_cl_int,
									OnCallclGetMemObjectInfo_cl_int)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetMemObjectInfo_cl_uint,
									OnCallclGetMemObjectInfo_cl_uint)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetMemObjectInfo_cl_ulong,
									OnCallclGetMemObjectInfo_cl_ulong)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetMemObjectInfo_size_t,
									OnCallclGetMemObjectInfo_size_t)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetMemObjectInfo_cl_point,
									OnCallclGetMemObjectInfo_cl_point)
	//clCreateSubBuffer
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateSubBuffer,
									OnCallclCreateSubBuffer)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateSampler,
									OnCallclCreateSampler)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetSamplerInfo_cl_uint,
									OnCallclGetSamplerInfo_cl_uint)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetSamplerInfo_cl_point,
									OnCallclGetSamplerInfo_cl_point)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_ReleaseSampler,
									OnCallclReleaseSampler)
	//clGetImageInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetImageInfo_cl_int,
									OnCallclGetImageInfo_cl_int)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetImageInfo_size_t,
									OnCallclGetImageInfo_size_t)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetImageInfo_cl_point,
									OnCallclGetImageInfo_cl_point)

	//clGetEventInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetEventInfo_cl_point,
									OnCallclGetEventInfo_cl_point)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetEventInfo_cl_uint,
									OnCallclGetEventInfo_cl_uint)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetEventInfo_cl_int,
									OnCallclGetEventInfo_cl_int)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetEventProfilingInfo_cl_ulong,
									OnCallclGetEventProfilingInfo_cl_ulong)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_SetEventCallback,
									OnCallclSetEventCallback)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_ReleaseEvent,
									OnCallclReleaseEvent)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetImageInfo_cl_uint_list,
									OnCallclGetImageInfo_cl_uint_list)

	//clGetConextInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetContextInfo_cl_uint,
									OnCallclGetContextInfo_cl_uint)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetContextInfo_cl_point,
									OnCallclGetContextInfo_cl_point)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetContextInfo_cl_point_list,
									OnCallclGetContextInfo_cl_point_list)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_SetUserEventStatus,
									OnCallclSetUserEventStatus)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateUserEvent,
									OnCallclCreateUserEvent)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetSupportedImageFormat,
									OnCallclGetSupportedImageFormat)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_ReleaseCommon,
									OnCallclReleaseCommon)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateCommandQueue,
									OnCallclCreateCommandQueue)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_Flush,
									OnCallFlush)
	//clGetCommandQueueInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetCommandQueueInfo_cl_point,
									OnCallclGetCommandQueueInfo_cl_point)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetCommandQueueInfo_cl_ulong,
									OnCallclGetCommandQueueInfo_cl_ulong)

	//clGetKernelInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelInfo_string,
									OnCallclGetKernelInfo_string)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelInfo_cl_uint,
									OnCallclGetKernelInfo_cl_uint)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelInfo_cl_point,
									OnCallclGetKernelInfo_cl_point)

	//clGetKernelWorkGroupInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelWorkGroupInfo_size_t_list,
									OnCallclGetKernelWorkGroupInfo_size_t_list)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelWorkGroupInfo_size_t,
									OnCallclGetKernelWorkGroupInfo_size_t)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelWorkGroupInfo_cl_ulong,
									OnCallclGetKernelWorkGroupInfo_cl_ulong)

	//clGetKernelArgInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelArgInfo_string,
									OnCallclGetKernelArgInfo_string)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelArgInfo_cl_uint,
									OnCallclGetKernelArgInfo_cl_uint)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetKernelArgInfo_cl_ulong,
									OnCallclGetKernelArgInfo_cl_ulong)

	//clReleaseKernel
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_ReleaseKernel,
									OnCallclReleaseKernel)

	//clGetProgramInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramInfo_cl_uint,
									OnCallclGetProgramInfo_cl_uint)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramInfo_cl_point,
									OnCallclGetProgramInfo_cl_point)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramInfo_cl_point_list,
									OnCallclGetProgramInfo_cl_point_list)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramInfo_string,
									OnCallclGetProgramInfo_string)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramInfo_size_t_list,
									OnCallclGetProgramInfo_size_t_list)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramInfo_string_list,
									OnCallclGetProgramInfo_string_list)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramInfo_size_t,
									OnCallclGetProgramInfo_size_t)

	//clCreateProgramWithSource
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateProgramWithSource,
									OnCallclCreateProgramWithSource)

	//clGetProgramBuildInfo
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramBuildInfo_cl_int,
									OnCallclGetProgramBuildInfo_cl_int)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramBuildInfo_string,
									OnCallclGetProgramBuildInfo_string)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_GetProgramBuildInfo_cl_uint,
									OnCallclGetProgramBuildInfo_cl_uint)

	//clBuildProgram
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_BuildProgram,
									OnCallclBuildProgram)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_EnqueueMarker,
									OnCallclEnqueueMarker)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_EnqueueBarrier,
									OnCallclEnqueueBarrier)

	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_EnqueueWaitForEvents,
									OnCallclEnqueueWaitForEvents)

	//clCreateKernel
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateKernel,
									OnCallclCreateKernel)

	//clCreateKernelsInProgram
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CreateKernelsInProgram,
									OnCallclCreateKernelsInProgram)

	//clReleaseProgram
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_ReleaseProgram,
									OnCallclReleaseProgram)

	//Control SHM
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CTRL_SetSharedHandles,
									OnCallCtrlSetSharedHandles)
	IPC_MESSAGE_HANDLER(OpenCLChannelMsg_CTRL_ClearSharedHandles,
									OnCallCtrlClearSharedHandles)

  // gl/cl sharing
  IPC_MESSAGE_HANDLER(OpenCLChannelMsg_getGLContext,
                  OnCallGetGLContext)

   //Vulkan IPC Message Received
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_SetSharedHandles,
                  OnCallVKCSetSharedHandles)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ClearSharedHandles,
                  OnCallVKCClearSharedHandles)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreateInstance,
        OnCallVKCCreateInstance)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_DestroyInstance,
        OnCallVKCDestroyInstance)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_EnumeratePhysicalDevice,
        OnCallVKCEnumeratePhysicalDevice)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_DestroyPhysicalDevice,
        OnCallVKCDestroyPhysicalDevice)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreateDevice,
        OnCallVKCCreateDevice)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_DestroyDevice,
        OnCallVKCDestroyDevice)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_GetDeviceInfo_uint,
        OnCallVKCGetDeviceInfoUint)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_GetDeviceInfo_array,
        OnCallVKCGetDeviceInfoArray)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_GetDeviceInfo_string,
        OnCallVKCGetDeviceInfoString)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreateBuffer,
        OnCallVKCCreateBuffer)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ReleaseBuffer,
        OnCallVKCReleaseBuffer)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_FillBuffer,
        OnCallVKCFillBuffer)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreateCommandQueue,
        OnCallVKCCreateCommandQueue)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ReleaseCommandQueue,
        OnCallVKCReleaseCommandQueue)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreateDescriptorSetLayout,
        OnCallVKCCreateDescriptorSetLayout)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ReleaseDescriptorSetLayout,
        OnCallVKCReleaseDescriptorSetLayout)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreateDescriptorPool,
        OnCallVKCCreateDescriptorPool)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ReleaseDescriptorPool,
        OnCallVKCReleaseDescriptorPool)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreateDescriptorSet,
        OnCallVKCCreateDescriptorSet)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ReleaseDescriptorSet,
        OnCallVKCReleaseDescriptorSet)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreatePipelineLayout,
        OnCallVKCCreatePipelineLayout)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ReleasePipelineLayout,
        OnCallVKCReleasePipelineLayout)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreateShaderModule,
        OnCallVKCCreateShaderModule)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ReleaseShaderModule,
        OnCallVKCReleaseShaderModule)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CreatePipeline,
        OnCallVKCCreatePipeline)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_ReleasePipeline,
        OnCallVKCReleasePipeline)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_UpdateDescriptorSets,
        OnCallVKCUpdateDescriptorSets)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_BeginQueue,
        OnCallVKCBeginQueue)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_EndQueue,
        OnCallVKCEndQueue)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_Dispatch,
        OnCallVKCDispatch)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_PipelineBarrier,
        OnCallVKCPipelineBarrier)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_CmdCopyBuffer,
        OnCallVKCCmdCopyBuffer)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_QueueSubmit,
        OnCallVKCQueueSubmit)
  IPC_MESSAGE_HANDLER(VulkanComputeChannelMsg_DeviceWaitIdle,
        OnCallVKCDeviceWaitIdle)

  IPC_MESSAGE_UNHANDLED(handled = false)

  IPC_END_MESSAGE_MAP()
  return handled;
}

scoped_refptr<SyncPointOrderData> GpuChannel::GetSyncPointOrderData(
    int32_t stream_id) {
  auto it = streams_.find(stream_id);
  DCHECK(it != streams_.end());
  DCHECK(it->second);
  return it->second->GetSyncPointOrderData();
}

void GpuChannel::PostHandleMessage(
    const scoped_refptr<GpuChannelMessageQueue>& queue) {
  task_runner_->PostTask(FROM_HERE,
                         base::Bind(&GpuChannel::HandleMessage,
                                    weak_factory_.GetWeakPtr(), queue));
}

void GpuChannel::PostHandleOutOfOrderMessage(const IPC::Message& msg) {
  task_runner_->PostTask(FROM_HERE,
                         base::Bind(&GpuChannel::HandleOutOfOrderMessage,
                                    weak_factory_.GetWeakPtr(), msg));
}

void GpuChannel::HandleMessage(
    const scoped_refptr<GpuChannelMessageQueue>& message_queue) {
  const GpuChannelMessage* channel_msg =
      message_queue->BeginMessageProcessing();
  if (!channel_msg)
    return;

  const IPC::Message& msg = channel_msg->message;
  int32_t routing_id = msg.routing_id();
  GpuCommandBufferStub* stub = stubs_.get(routing_id);

  DCHECK(!stub || stub->IsScheduled());

  DVLOG(1) << "received message @" << &msg << " on channel @" << this
           << " with type " << msg.type();

  HandleMessageHelper(msg);

  // If we get descheduled or yield while processing a message.
  if ((stub && stub->HasUnprocessedCommands()) ||
      !message_queue->IsScheduled()) {
    DCHECK((uint32_t)GpuCommandBufferMsg_AsyncFlush::ID == msg.type() ||
           (uint32_t)GpuCommandBufferMsg_WaitSyncToken::ID == msg.type());
    message_queue->PauseMessageProcessing();
  } else {
    message_queue->FinishMessageProcessing();
  }
}

void GpuChannel::HandleMessageHelper(const IPC::Message& msg) {
  int32_t routing_id = msg.routing_id();

  bool handled = false;
  if (routing_id == MSG_ROUTING_CONTROL) {
    handled = OnControlMessageReceived(msg);
  } else {
    handled = router_.RouteMessage(msg);
  }

  if (!handled && unhandled_message_listener_)
    handled = unhandled_message_listener_->OnMessageReceived(msg);

  // Respond to sync messages even if router failed to route.
  if (!handled && msg.is_sync()) {
    IPC::Message* reply = IPC::SyncMessage::GenerateReply(&msg);
    reply->set_reply_error();
    Send(reply);
  }
}

void GpuChannel::HandleOutOfOrderMessage(const IPC::Message& msg) {
  HandleMessageHelper(msg);
}

void GpuChannel::HandleMessageForTesting(const IPC::Message& msg) {
  HandleMessageHelper(msg);
}

scoped_refptr<GpuChannelMessageQueue> GpuChannel::CreateStream(
    int32_t stream_id,
    GpuStreamPriority stream_priority) {
  DCHECK(streams_.find(stream_id) == streams_.end());
  scoped_refptr<GpuChannelMessageQueue> queue = GpuChannelMessageQueue::Create(
      stream_id, stream_priority, this, io_task_runner_,
      (stream_id == GPU_STREAM_DEFAULT) ? preempting_flag_ : nullptr,
      preempted_flag_, sync_point_manager_);
  streams_.insert(std::make_pair(stream_id, queue));
  streams_to_num_routes_.insert(std::make_pair(stream_id, 0));
  return queue;
}

scoped_refptr<GpuChannelMessageQueue> GpuChannel::LookupStream(
    int32_t stream_id) {
  auto stream_it = streams_.find(stream_id);
  if (stream_it != streams_.end())
    return stream_it->second;
  return nullptr;
}

void GpuChannel::DestroyStreamIfNecessary(
    const scoped_refptr<GpuChannelMessageQueue>& queue) {
  int32_t stream_id = queue->stream_id();
  if (streams_to_num_routes_[stream_id] == 0) {
    queue->Disable();
    streams_to_num_routes_.erase(stream_id);
    streams_.erase(stream_id);
  }
}

void GpuChannel::AddRouteToStream(int32_t route_id, int32_t stream_id) {
  DCHECK(streams_.find(stream_id) != streams_.end());
  DCHECK(routes_to_streams_.find(route_id) == routes_to_streams_.end());
  streams_to_num_routes_[stream_id]++;
  routes_to_streams_.insert(std::make_pair(route_id, stream_id));
  filter_->AddRoute(route_id, streams_[stream_id]);
}

void GpuChannel::RemoveRouteFromStream(int32_t route_id) {
  DCHECK(routes_to_streams_.find(route_id) != routes_to_streams_.end());
  int32_t stream_id = routes_to_streams_[route_id];
  DCHECK(streams_.find(stream_id) != streams_.end());
  routes_to_streams_.erase(route_id);
  streams_to_num_routes_[stream_id]--;
  filter_->RemoveRoute(route_id);
  DestroyStreamIfNecessary(streams_[stream_id]);
}

#if defined(OS_ANDROID)
const GpuCommandBufferStub* GpuChannel::GetOneStub() const {
  for (const auto& kv : stubs_) {
    const GpuCommandBufferStub* stub = kv.second;
    if (stub->decoder() && !stub->decoder()->WasContextLost())
      return stub;
  }
  return nullptr;
}
#endif

void GpuChannel::OnCreateCommandBuffer(
    const GPUCreateCommandBufferConfig& init_params,
    int32_t route_id,
    base::SharedMemoryHandle shared_state_handle,
    bool* result,
    gpu::Capabilities* capabilities) {
  TRACE_EVENT2("gpu", "GpuChannel::OnCreateCommandBuffer", "route_id", route_id,
               "offscreen", (init_params.surface_handle == kNullSurfaceHandle));
  std::unique_ptr<base::SharedMemory> shared_state_shm(
      new base::SharedMemory(shared_state_handle, false));
  std::unique_ptr<GpuCommandBufferStub> stub =
      CreateCommandBuffer(init_params, route_id, std::move(shared_state_shm));
  if (stub) {
    *result = true;
    *capabilities = stub->decoder()->GetCapabilities();
    stubs_.set(route_id, std::move(stub));
  } else {
    *result = false;
    *capabilities = gpu::Capabilities();
  }
}

std::unique_ptr<GpuCommandBufferStub> GpuChannel::CreateCommandBuffer(
    const GPUCreateCommandBufferConfig& init_params,
    int32_t route_id,
    std::unique_ptr<base::SharedMemory> shared_state_shm) {
  if (init_params.surface_handle != kNullSurfaceHandle &&
      !allow_view_command_buffers_) {
    DLOG(ERROR) << "GpuChannel::CreateCommandBuffer(): attempt to create a "
                   "view context on a non-priviledged channel";
    return nullptr;
  }

  int32_t share_group_id = init_params.share_group_id;
  GpuCommandBufferStub* share_group = stubs_.get(share_group_id);

  if (!share_group && share_group_id != MSG_ROUTING_NONE) {
    DLOG(ERROR)
        << "GpuChannel::CreateCommandBuffer(): invalid share group id";
    return nullptr;
  }

  int32_t stream_id = init_params.stream_id;
  if (share_group && stream_id != share_group->stream_id()) {
    DLOG(ERROR) << "GpuChannel::CreateCommandBuffer(): stream id does not "
                   "match share group stream id";
    return nullptr;
  }

  GpuStreamPriority stream_priority = init_params.stream_priority;
  if (!allow_real_time_streams_ &&
      stream_priority == GpuStreamPriority::REAL_TIME) {
    DLOG(ERROR) << "GpuChannel::CreateCommandBuffer(): real time stream "
                   "priority not allowed";
    return nullptr;
  }

  if (share_group && !share_group->decoder()) {
    // This should catch test errors where we did not Initialize the
    // share_group's CommandBuffer.
    DLOG(ERROR) << "GpuChannel::CreateCommandBuffer(): shared context was "
                   "not initialized";
    return nullptr;
  }

  if (share_group && share_group->decoder()->WasContextLost()) {
    DLOG(ERROR) << "GpuChannel::CreateCommandBuffer(): shared context was "
                   "already lost";
    return nullptr;
  }

  scoped_refptr<GpuChannelMessageQueue> queue = LookupStream(stream_id);
  if (!queue)
    queue = CreateStream(stream_id, stream_priority);

  std::unique_ptr<GpuCommandBufferStub> stub(GpuCommandBufferStub::Create(
      this, share_group, init_params, route_id, std::move(shared_state_shm)));

  if (!stub) {
    DestroyStreamIfNecessary(queue);
    return nullptr;
  }

  if (!AddRoute(route_id, stream_id, stub.get())) {
    DestroyStreamIfNecessary(queue);
    DLOG(ERROR) << "GpuChannel::CreateCommandBuffer(): failed to add route";
    return nullptr;
  }

  return stub;
}

void GpuChannel::OnDestroyCommandBuffer(int32_t route_id) {
  TRACE_EVENT1("gpu", "GpuChannel::OnDestroyCommandBuffer",
               "route_id", route_id);

  std::unique_ptr<GpuCommandBufferStub> stub = stubs_.take_and_erase(route_id);
  // In case the renderer is currently blocked waiting for a sync reply from the
  // stub, we need to make sure to reschedule the correct stream here.
  if (stub && !stub->IsScheduled()) {
    // This stub won't get a chance to reschedule the stream so do that now.
    OnStreamRescheduled(stub->stream_id(), true);
  }

  RemoveRoute(route_id);
}

void GpuChannel::OnGetDriverBugWorkArounds(
    std::vector<std::string>* gpu_driver_bug_workarounds) {
  gpu_driver_bug_workarounds->clear();
#define GPU_OP(type, name)                                     \
  if (gpu_channel_manager_->gpu_driver_bug_workarounds().name) \
    gpu_driver_bug_workarounds->push_back(#name);
  GPU_DRIVER_BUG_WORKAROUNDS(GPU_OP)
#undef GPU_OP
}

void GpuChannel::OnCallclGetPlatformIDs(
      const cl_uint& num_entries,
      const std::vector<bool>& return_variable_null_status,
      std::vector<cl_point>* point_platform_list,
      cl_uint* num_platforms,
      cl_int* errcode_ret)
{
    //*errcode_ret = num_entries + add_value;
    // Receiving and responding the Sync IPC Message from another process
    // and return the results of clGetPlatformIDs OpenCL API calling.
    cl_platform_id* platforms = NULL;
    cl_uint* num_platforms_inter = num_platforms;

    // If the caller wishes to pass a NULL.
    if (return_variable_null_status[0])
      num_platforms_inter = NULL;

    // Dump the inputs of the Sync IPC Message calling.
    if (num_entries > 0 && !return_variable_null_status[1])
      platforms = new cl_platform_id[num_entries];

    //Call OpenCL API
    *errcode_ret = clApiImpl->doclGetPlatformIDs(num_entries, platforms, num_platforms_inter);

    // Dump the results of OpenCL API calling.
  if (num_entries > 0 && !return_variable_null_status[1]) {
    (*point_platform_list).clear();
    for (cl_uint index = 0; index < num_entries; ++index)
      (*point_platform_list).push_back((cl_point) platforms[index]);

    if (platforms)
      delete[] platforms;
  }
}

void GpuChannel::OnCallclGetPlatformInfo(
      cl_point platform,
      cl_platform_info param_name,
      size_t param_value_size,
      std::vector<bool> null_param_status,
      cl_int* errcode_ret,
      std::string* param_value,
      size_t* param_value_size_ret)
{
  char platform_string_inter[param_value_size];

  *errcode_ret = clApiImpl->doClGetPlatformInfo(
      (cl_platform_id)platform,
      param_name,
      param_value_size,
      platform_string_inter,
      null_param_status[0]?NULL:param_value_size_ret
  );

  param_value->append(platform_string_inter);
}

void GpuChannel::OnCallclGetDeviceIDs(
    const cl_point& point_platform,
    const cl_device_type& device_type,
    const cl_uint& num_entries,
  const std::vector<bool>& return_variable_null_status,
    std::vector<cl_point>* point_device_list,
    cl_uint* num_devices,
    cl_int* errcode_ret) {
  // Receiving and responding the Sync IPC Message from another process
  // and return the results of clGetDeviceIDs OpenCL API calling.
  cl_platform_id platform = (cl_platform_id) point_platform;
  cl_device_id* devices = NULL;
  cl_uint *num_devices_inter = num_devices;

  // If the caller wishes to pass a NULL.
  if (return_variable_null_status[0])
    num_devices_inter = NULL;

  // Dump the inputs of the Sync IPC Message calling.
  if (num_entries > 0 && !return_variable_null_status[1])
    devices = new cl_device_id[num_entries];

  CLLOG(INFO) << "GpuChannel::OnCallclGetDeviceIDs";

  // Call the OpenCL API.
  if (clApiImpl) {
    *errcode_ret = clApiImpl->doClGetDeviceIDs(
                      platform,
                      device_type,
                      num_entries,
                      devices,
                      num_devices_inter);

    CLLOG(INFO) << "GpuChannel::OnCallclGetDeviceIDs num_entries : " << num_entries;
    if (num_devices_inter != NULL)
      CLLOG(INFO) << "GpuChannel::OnCallclGetDeviceIDs num_devices_inter : " << num_devices_inter;

    // Dump the results of OpenCL API calling.
    if (num_entries > 0 && !return_variable_null_status[1]) {
      (*point_device_list).clear();
      for (cl_uint index = 0; index < num_entries; ++index)
        (*point_device_list).push_back((cl_point) devices[index]);
    if (devices)
        delete[] devices;
    }
  }
}

void GpuChannel::OnCallclGetDeviceInfo_string(
  const cl_point& point_device,
  const cl_device_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  std::string* string_ret,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
    cl_device_id device = (cl_device_id) point_device;
    size_t *param_value_size_ret_inter = param_value_size_ret;
    char* param_value = NULL;
    char c;

    if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

    if (!return_variable_null_status[1] && param_value_size >= sizeof(char))
      param_value = new char[param_value_size/sizeof(char)];
    else if (!return_variable_null_status[1])
      param_value = &c;

    *errcode_ret = clApiImpl->doClGetDeviceInfo(
        device,
        param_name,
        param_value_size,
        param_value,
        param_value_size_ret_inter);

    if(!return_variable_null_status[1] && param_value_size >= sizeof(char)) {
      (*string_ret) = std::string(param_value);
      delete[] param_value;
    }
}

void GpuChannel::OnCallclGetDeviceInfo_cl_uint(
    const cl_point& point_device,
    const cl_device_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_uint* cl_uint_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_device_id device = (cl_device_id) point_device;
  size_t* param_value_size_ret_inter = param_value_size_ret;
  cl_uint* cl_uint_ret_inter = cl_uint_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_uint_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetDeviceInfo(
      device,
      param_name,
      param_value_size,
      cl_uint_ret_inter,
      param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetDeviceInfo_size_t_list(
    const cl_point& point_device,
    const cl_device_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::vector<size_t>* size_t_list_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_device_id device = (cl_device_id) point_device;
  size_t* param_value_size_ret_inter = param_value_size_ret;
  size_t* param_value = NULL;
  size_t c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (!return_variable_null_status[1] && param_value_size >= sizeof(size_t))
    param_value = new size_t[param_value_size/sizeof(size_t)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetDeviceInfo(
      device,
      param_name,
      param_value_size,
      param_value,
      param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(size_t)) {
    for (cl_uint index = 0; index < param_value_size/sizeof(size_t); ++index)
      (*size_t_list_ret).push_back(param_value[index]);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetDeviceInfo_size_t(
    const cl_point& point_device,
    const cl_device_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    size_t* size_t_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_device_id device = (cl_device_id) point_device;
  size_t* param_value_size_ret_inter = param_value_size_ret;
  size_t* size_t_ret_inter = size_t_ret;

  if (return_variable_null_status[0])
  param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    size_t_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetDeviceInfo(
      device,
      param_name,
      param_value_size,
      size_t_ret_inter,
      param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetDeviceInfo_cl_ulong(
    const cl_point& point_device,
    const cl_device_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_ulong* cl_ulong_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_device_id device = (cl_device_id) point_device;
  size_t* param_value_size_ret_inter = param_value_size_ret;
  cl_ulong* cl_ulong_ret_inter = cl_ulong_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_ulong_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetDeviceInfo(
      device,
      param_name,
      param_value_size,
      cl_ulong_ret_inter,
      param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetDeviceInfo_cl_point(
    const cl_point& point_device,
    const cl_device_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_point* cl_point_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_device_id device = (cl_device_id) point_device;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_point* cl_point_ret_inter = cl_point_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_point_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetDeviceInfo(
      device,
      param_name,
      param_value_size,
      cl_point_ret_inter,
      param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetDeviceInfo_intptr_t_list(
    const cl_point& point_device,
    const cl_device_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::vector<intptr_t>* intptr_t_list_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_device_id device = (cl_device_id) point_device;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  intptr_t* param_value = NULL;
  intptr_t c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (!return_variable_null_status[1] && param_value_size >= sizeof(intptr_t))
    param_value = new intptr_t[param_value_size/sizeof(intptr_t)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetDeviceInfo(
      device,
      param_name,
      param_value_size,
      param_value,
      param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(intptr_t)) {
    for (cl_uint index = 0; index < param_value_size/sizeof(intptr_t); ++index)
      (*intptr_t_list_ret).push_back(param_value[index]);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclCreateContext(
    const std::vector<cl_context_properties>& property_list,
  const std::vector<cl_point>& device_list,
    const cl_point& point_pfn_notify,
    const cl_point& point_user_data,
    const std::vector<bool>& return_variable_null_status,
    cl_int* errcode_ret,
    cl_point* point_context_ret) {
  cl_context_properties* properties = NULL;
  cl_context context_ret;
  cl_int* errcode_ret_inter = errcode_ret;
  void* user_data = (void*) point_user_data;
  void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*) =
    (void (CL_CALLBACK*)(const char*, const void*, size_t, void*))
      point_pfn_notify;

  if (return_variable_null_status[0])
    errcode_ret_inter = NULL;

  if (!property_list.empty()) {
    properties = new cl_context_properties[property_list.size()];
    for (cl_uint index = 0; index < property_list.size(); ++index)
      properties[index] = property_list[index];
  }

  cl_uint num_devices = device_list.size();
  cl_device_id* devices = new cl_device_id[num_devices];
  for(unsigned i=0; i<num_devices; i++) {
    devices[i] = (cl_device_id)device_list[i];
  }

  context_ret = clApiImpl->doClCreateContext(
                    properties,
          num_devices,
          devices,
                    pfn_notify,
                    user_data,
                    errcode_ret_inter);

  if (!property_list.empty())
    delete[] properties;

  *point_context_ret = (cl_point) context_ret;
}

void GpuChannel::OnCallclCreateContextFromType(
    const std::vector<cl_context_properties>& property_list,
    const cl_device_type& device_type,
    const cl_point& point_pfn_notify,
    const cl_point& point_user_data,
    const std::vector<bool>& return_variable_null_status,
    cl_int* errcode_ret,
    cl_point* point_context_ret) {
  cl_context_properties* properties = NULL;
  cl_context context_ret;
  cl_int* errcode_ret_inter = errcode_ret;
  void* user_data = (void*) point_user_data;
  void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*) =
    (void (CL_CALLBACK*)(const char*, const void*, size_t, void*))
      point_pfn_notify;

  if (return_variable_null_status[0])
    errcode_ret_inter = NULL;

  if (!property_list.empty()) {
    properties = new cl_context_properties[property_list.size()];
    for (cl_uint index = 0; index < property_list.size(); ++index)
      properties[index] = property_list[index];
  }

  context_ret = clApiImpl->doClCreateContextFromType(
                    properties,
                    device_type,
                    pfn_notify,
                    user_data,
                    errcode_ret_inter);

  if (!property_list.empty())
    delete[] properties;

  *point_context_ret = (cl_point) context_ret;
}

void GpuChannel::OnCallclWaitForevents(
    const cl_uint& num_events,
    const std::vector<cl_point>& event_list,
    cl_int* errcode_ret)
{
  cl_event* event_list_inter = new cl_event[num_events];
  for(size_t i=0; i<num_events; i++) {
    event_list_inter[i] = (cl_event)event_list[i];
  }

  cl_int errcode_inter = clApiImpl->doClWaitForEvents(
      num_events,
      event_list_inter);

  *errcode_ret = errcode_inter;

  if(event_list_inter)
    delete[] event_list_inter;
}

void GpuChannel::OnCallclGetMemObjectInfo_cl_int(
    const cl_point& memobj,
    const cl_mem_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
  cl_int* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClGetMemObjectInfo(
      (cl_mem)memobj,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:(cl_int*)param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetMemObjectInfo_cl_uint(
    const cl_point& memobj,
    const cl_mem_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
  cl_uint* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClGetMemObjectInfo(
      (cl_mem)memobj,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetMemObjectInfo_cl_ulong(
  const cl_point& memobj,
  const cl_mem_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  cl_ulong* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClGetMemObjectInfo(
      (cl_mem)memobj,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetMemObjectInfo_size_t(
  const cl_point& memobj,
  const cl_mem_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  size_t* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClGetMemObjectInfo(
      (cl_mem)memobj,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetMemObjectInfo_cl_point(
  const cl_point& memobj,
  const cl_mem_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  cl_point* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClGetMemObjectInfo(
      (cl_mem)memobj,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:(cl_context*)param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclCreateSubBuffer(
    const cl_point& buffer,
    const cl_mem_flags& flags,
    const cl_buffer_create_type& buffer_create_type,
  const size_t origin,
  const size_t size,
  cl_point* sub_buffer,
  cl_int* errcode_ret)
{
  cl_buffer_region region =
  {
    origin,
    size
  };

  cl_mem sub_buffer_inter = clApiImpl->doClCreateSubBuffer(
      (cl_mem)buffer,
      flags,
      buffer_create_type,
      &region,
      errcode_ret
  );

  *(cl_mem*)sub_buffer = sub_buffer_inter;
}

void GpuChannel::OnCallclCreateSampler (
    const cl_point& point_context,
    const cl_bool& normalized_coords,
    const cl_addressing_mode& addressing_mode,
    const cl_filter_mode& filter_mode,
    const std::vector<bool>& return_variable_null_status,
    cl_int* errcode_ret,
    cl_point* point_sampler_ret) {
  cl_context context = (cl_context) point_context;
  cl_sampler sampler_ret;
  cl_int* errcode_ret_inter = errcode_ret;

  if (return_variable_null_status[0])
    errcode_ret_inter = NULL;

  sampler_ret = clApiImpl->doClCreateSampler(
                    context,
                    normalized_coords,
                    addressing_mode,
                    filter_mode,
                    errcode_ret_inter);

  *point_sampler_ret = (cl_point) sampler_ret;
}

void GpuChannel::OnCallclGetSamplerInfo_cl_uint(
    const cl_point& point_sampler,
    const cl_sampler_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_uint* cl_uint_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_sampler sampler = (cl_sampler) point_sampler;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_uint* cl_uint_ret_inter = cl_uint_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_uint_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetSamplerInfo(
                     sampler,
                     param_name,
                     param_value_size,
                     cl_uint_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetSamplerInfo_cl_point(
    const cl_point& point_sampler,
    const cl_sampler_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_point* cl_point_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_sampler sampler = (cl_sampler) point_sampler;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_point* cl_point_ret_inter = cl_point_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_point_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetSamplerInfo(
                     sampler,
                     param_name,
                     param_value_size,
                     cl_point_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclReleaseSampler(
    const cl_point& point_sampler,
    cl_int* errcode_ret) {
  cl_sampler sampler = (cl_sampler) point_sampler;

  *errcode_ret = clApiImpl->doClReleaseSampler(sampler);
}

void GpuChannel::OnCallclGetImageInfo_cl_int(
    const cl_point& image,
    const cl_image_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
  cl_int* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClGetImageInfo(
      (cl_mem)image,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetImageInfo_size_t(
  const cl_point& image,
  const cl_image_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  size_t* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClGetImageInfo(
      (cl_mem)image,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetImageInfo_cl_point(
  const cl_point& image,
  const cl_image_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  cl_point* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClGetImageInfo(
      (cl_mem)image,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetEventInfo_cl_point(
    const cl_point& point_event,
    const cl_event_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_point* cl_point_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_event clevent = (cl_event) point_event;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_point* cl_point_ret_inter = cl_point_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_point_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetEventInfo(
                     clevent,
                     param_name,
                     param_value_size,
                     cl_point_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetEventInfo_cl_uint(
    const cl_point& point_event,
    const cl_event_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_uint* cl_uint_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_event clevent = (cl_event) point_event;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_uint* cl_uint_ret_inter = cl_uint_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_uint_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetEventInfo(
                     clevent,
                     param_name,
                     param_value_size,
                     cl_uint_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetEventInfo_cl_int(
    const cl_point& point_event,
    const cl_event_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_int* cl_int_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_event clevent = (cl_event) point_event;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_int* cl_int_ret_inter = cl_int_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_int_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetEventInfo(
                     clevent,
                     param_name,
                     param_value_size,
                     cl_int_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetEventProfilingInfo_cl_ulong(
    const cl_point& point_event,
    const cl_profiling_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_ulong* cl_ulong_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_event clevent = (cl_event) point_event;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_ulong* cl_ulong_ret_inter = cl_ulong_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_ulong_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetEventProfilingInfo(
                     clevent,
                     param_name,
                     param_value_size,
                     cl_ulong_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclSetEventCallback(
    const cl_point& point_event,
    const cl_int& command_exec_callback_type,
    const std::vector<int>& key_list,
    cl_int* errcode_ret) {
  CLLOG(INFO) << "GpuChannel::OnCallclSetEventCallback";

  cl_event clevent = (cl_event) point_event;

#if defined(OS_ANDROID)
  unsigned* event_callback_keys = new unsigned[5];
  event_callback_keys[0] = point_event;//callback type
  event_callback_keys[1] = (unsigned)key_list[0];//hadler id
  event_callback_keys[2] = (unsigned)key_list[1];//object type
  event_callback_keys[3] = (unsigned)key_list[2];//object type
#elif defined(OS_LINUX)
  cl_point* event_callback_keys = new cl_point[5];
  event_callback_keys[0] = point_event;//callback type
  event_callback_keys[1] = (cl_point)key_list[0];//hadler id
  event_callback_keys[2] = (cl_point)key_list[1];//object type
  event_callback_keys[3] = (cl_point)key_list[2];//object type
#endif

  *errcode_ret = clApiImpl->doClSetEventCallback(
                     clevent,
                     command_exec_callback_type,
                     NULL,
                     event_callback_keys);

}

void GpuChannel::OnCallclReleaseEvent(
    const cl_point& point_event,
    cl_int* errcode_ret) {
  cl_event clevent = (cl_event) point_event;

  *errcode_ret = clApiImpl->doClReleaseEvent(clevent);
}

void GpuChannel::OnCallclGetImageInfo_cl_uint_list(
  const cl_point& image,
  const cl_image_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  std::vector<cl_uint>* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  cl_image_format image_format_inter;

  cl_int err = clApiImpl->doClGetImageInfo(
      (cl_mem)image,
      param_name,
      param_value_size,
      &image_format_inter,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  if(err == CL_SUCCESS && !return_variable_null_status[0]) {
    param_value->push_back(image_format_inter.image_channel_order);
    param_value->push_back(image_format_inter.image_channel_data_type);
  }

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetContextInfo_cl_uint(
    const cl_point& context,
    const cl_image_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
  cl_uint* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  CLLOG(INFO) << "GpuChannel::OnCallclGetContextInfo_cl_uint";

  cl_int err = clApiImpl->doClGetContextInfo(
      (cl_context)context,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetContextInfo_cl_point(
  const cl_point& context,
  const cl_image_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  cl_point* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  CLLOG(INFO) << "GpuChannel::OnCallclGetContextInfo_cl_point";

  cl_int err = clApiImpl->doClGetContextInfo(
      (cl_context)context,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  if(err == CL_SUCCESS) {
    if(param_name == CL_CONTEXT_DEVICES) {
      cl_device_id* ids = (cl_device_id*)param_value;
      CLLOG(INFO) << ">>device id=" << ids[0];
    }
    else {
      CLLOG(INFO) << ">>num deivce=" << *(cl_uint*)param_value;
    }
  }

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetContextInfo_cl_point_list(
  const cl_point& context,
  const cl_image_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  std::vector<cl_point>* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  CLLOG(INFO) << "GpuChannel::OnCallclGetContextInfo_cl_point_list";

  cl_point* param_value_inter = NULL;
  cl_point c;

  if (!return_variable_null_status[0] && param_value_size >= sizeof(cl_point))
    param_value_inter = new cl_point[param_value_size/sizeof(cl_point)];
  else if (!return_variable_null_status[0])
    param_value_inter = &c;

  cl_int err = clApiImpl->doClGetContextInfo(
      (cl_context)context,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value_inter,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  if (!return_variable_null_status[0] && param_value_size >= sizeof(cl_point)) {
    for (cl_uint index = 0; index < param_value_size/sizeof(cl_point); ++index)
      (*param_value).push_back(param_value_inter[index]);
    delete[] param_value_inter;
  }

  *errcode_ret = err;
}

void GpuChannel::OnCallclSetUserEventStatus(
  const cl_point& point_event,
  const cl_int& execution_status,
  cl_int* errcode_ret) {
  cl_event clevent = (cl_event) point_event;

  *errcode_ret = clApiImpl->doClSetUserEventStatus(
      clevent,
      execution_status);
}

void GpuChannel::OnCallclCreateUserEvent(
  const cl_point& point_in_context,
  const std::vector<bool>& return_variable_null_status,
  cl_int* errcode_ret,
  cl_point* point_out_context) {
  cl_context context = (cl_context) point_in_context;
  cl_event event_context_ret;
  cl_int* errcode_ret_inter = errcode_ret;

  if (return_variable_null_status[0])
    errcode_ret_inter = NULL;

  event_context_ret = clApiImpl->doClCreateUserEvent(context, errcode_ret_inter);
  *point_out_context = (cl_point) event_context_ret;
}

void GpuChannel::OnCallclGetSupportedImageFormat(
  const cl_point& context,
  const cl_mem_flags& flags,
  const cl_mem_object_type& image_type,
  const cl_uint& num_entries,
  const std::vector<bool>& return_variable_null_status,
  std::vector<cl_uint>* image_formats,
  cl_uint* num_image_formats,
  cl_int* errcode_ret)
{
  CLLOG(INFO) << "GpuChannel::OnCallclGetSupportedImageFormat";

  cl_image_format* image_formats_inter = NULL;

  if(!return_variable_null_status[0]) {
    image_formats_inter = new cl_image_format[num_entries];
  }

  *errcode_ret = clApiImpl->doClGetSupportedImageFormat(
      (cl_context)context,
      flags,
      image_type,
      num_entries,
      return_variable_null_status[0]?NULL:image_formats_inter,
      return_variable_null_status[1]?NULL:num_image_formats);

  if(*errcode_ret == CL_SUCCESS && !return_variable_null_status[0]) {
    for(unsigned i = 0; i<num_entries; i++) {
      image_formats->push_back((cl_uint)image_formats_inter[i].image_channel_order);
      image_formats->push_back((cl_uint)image_formats_inter[i].image_channel_data_type);
    }

    delete[] image_formats_inter;
  }
}

void GpuChannel::OnCallclReleaseCommon(
  const cl_point& object,
  const int objectType,
  cl_int* errcode_ret)
{
  switch(objectType) {
  case CL_CONTEXT:
    *errcode_ret = clApiImpl->doClReleaseContext(
      (cl_context)object);
    break;
  case CL_PROGRAM:
    *errcode_ret = clApiImpl->doClReleaseProgram(
      (cl_program)object);
    break;
  case CL_KERNEL:
    *errcode_ret = clApiImpl->doClReleaseKernel(
      (cl_kernel)object);
    break;
  case CL_MEMORY_OBJECT:
    *errcode_ret = clApiImpl->doClReleaseMemObject(
      (cl_mem)object);
    break;
  case CL_COMMAND_QUEUE:
    *errcode_ret = clApiImpl->doClReleaseCommandQueue(
      (cl_command_queue)object);
    break;
  default:
    *errcode_ret = CL_SEND_IPC_MESSAGE_FAILURE;
    break;
  }
}

void GpuChannel::OnCallclCreateCommandQueue(
  const cl_point& context,
  const cl_point& device,
  const cl_command_queue_properties& properties,
  const std::vector<bool>& return_variable_null_status,
  cl_int* errcode_ret,
  cl_point* command_queue_ret)
{
  cl_int errcode_inter = 0;

  cl_command_queue command_queue_inter = clApiImpl->doClCreateCommandQueue(
      (cl_context)context,
      (cl_device_id)device,
      properties,
      &errcode_inter
  );

  if(errcode_inter == CL_SUCCESS) {
    *command_queue_ret = (cl_point)command_queue_inter;
  }

  *errcode_ret = errcode_inter;
}

void GpuChannel::OnCallclGetCommandQueueInfo_cl_ulong(
    const cl_point& command_queue,
    const cl_context_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
  cl_ulong* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  CLLOG(INFO) << "GpuChannel::OnCallclGetCommandQueueInfo_cl_ulong";

  cl_int err = clApiImpl->doClGetCommandQueueInfo(
      (cl_command_queue)command_queue,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetCommandQueueInfo_cl_point(
  const cl_point& command_queue,
  const cl_context_info& param_name,
  const size_t& param_value_size,
  const std::vector<bool>& return_variable_null_status,
  cl_point* param_value,
  size_t* param_value_size_ret,
  cl_int* errcode_ret)
{
  CLLOG(INFO) << "GpuChannel::OnCallclGetCommandQueueInfo_cl_point";

  cl_int err = clApiImpl->doClGetCommandQueueInfo(
      (cl_command_queue)command_queue,
      param_name,
      param_value_size,
      return_variable_null_status[0]?NULL:param_value,
      return_variable_null_status[1]?NULL:param_value_size_ret
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallFlush(
      const cl_point& command_queue,
      cl_int* errcode_ret)
{
  cl_int err = clApiImpl->doClFlush(
      (cl_command_queue)command_queue
  );

  *errcode_ret = err;
}

void GpuChannel::OnCallclGetKernelInfo_string(
    const cl_point& point_kernel,
    const cl_kernel_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::string* string_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  char* param_value = NULL;
  char c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  CLLOG(INFO) << "OnCallclGetKernelArgInfo_string :: param_value_size/sizeof(char)" << param_value_size/sizeof(char);
  if (!return_variable_null_status[1] && param_value_size >= sizeof(char))
    param_value = new char[param_value_size/sizeof(char)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetKernelInfo(
                     kernel,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(char)) {
    (*string_ret) = std::string(param_value);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetKernelInfo_cl_uint(
    const cl_point& point_kernel,
    const cl_kernel_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_uint* cl_uint_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_uint* cl_uint_ret_inter = cl_uint_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_uint_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetKernelInfo(
                     kernel,
                     param_name,
                     param_value_size,
                     cl_uint_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetKernelInfo_cl_point(
    const cl_point& point_kernel,
    const cl_kernel_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_point* cl_point_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_point* cl_point_ret_inter = cl_point_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_point_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetKernelInfo(
                     kernel,
                     param_name,
                     param_value_size,
                     cl_point_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetKernelWorkGroupInfo_size_t_list(
    const cl_point& point_kernel,
    const cl_point& point_device,
    const cl_kernel_work_group_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::vector<size_t>* size_t_list_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  cl_device_id device = (cl_device_id) point_device;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  size_t* param_value = NULL;
  size_t c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (!return_variable_null_status[1] && param_value_size >= sizeof(size_t))
    param_value = new size_t[param_value_size/sizeof(size_t)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetKernelWorkGroupInfo(
                     kernel,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(size_t)) {
    for (cl_uint index = 0; index < param_value_size/sizeof(size_t); ++index)
      (*size_t_list_ret).push_back(param_value[index]);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetKernelWorkGroupInfo_size_t(
    const cl_point& point_kernel,
    const cl_point& point_device,
    const cl_kernel_work_group_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    size_t* size_t_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  cl_device_id device = (cl_device_id) point_device;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  size_t* size_t_ret_inter = size_t_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    size_t_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetKernelWorkGroupInfo(
                     kernel,
                     device,
                     param_name,
                     param_value_size,
                     size_t_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetKernelWorkGroupInfo_cl_ulong(
    const cl_point& point_kernel,
    const cl_point& point_device,
    const cl_kernel_work_group_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_ulong* cl_ulong_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  cl_device_id device = (cl_device_id) point_device;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_ulong* cl_ulong_ret_inter = cl_ulong_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_ulong_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetKernelWorkGroupInfo(
                     kernel,
                     device,
                     param_name,
                     param_value_size,
                     cl_ulong_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetKernelArgInfo_string(
    const cl_point& point_kernel,
    const cl_uint& arg_indx,
    const cl_kernel_arg_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::string* string_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  char* param_value = NULL;
  char c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (!return_variable_null_status[1] && param_value_size >= sizeof(char))
    param_value = new char[param_value_size/sizeof(char)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetKernelArgInfo(
                     kernel,
                     arg_indx,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(char)) {
    (*string_ret) = std::string(param_value);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetKernelArgInfo_cl_uint(
    const cl_point& point_kernel,
    const cl_uint& arg_indx,
    const cl_kernel_arg_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_uint* cl_uint_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_uint* cl_uint_ret_inter = cl_uint_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_uint_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetKernelArgInfo(
                     kernel,
                     arg_indx,
                     param_name,
                     param_value_size,
                     cl_uint_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetKernelArgInfo_cl_ulong(
    const cl_point& point_kernel,
    const cl_uint& cl_uint_arg_indx,
    const cl_kernel_arg_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_ulong* cl_ulong_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;
  cl_uint arg_indx = (cl_uint) cl_uint_arg_indx;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_ulong* cl_ulong_ret_inter = cl_ulong_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_ulong_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetKernelArgInfo(
                     kernel,
                     arg_indx,
                     param_name,
                     param_value_size,
                     cl_ulong_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclReleaseKernel(
    const cl_point& point_kernel,
    cl_int* errcode_ret) {
  cl_kernel kernel = (cl_kernel) point_kernel;

  *errcode_ret = clApiImpl->doClReleaseKernel(kernel);
}

void GpuChannel::OnCallclGetProgramInfo_cl_uint(
    const cl_point& point_program,
    const cl_program_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_uint* cl_uint_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_uint* cl_uint_ret_inter = cl_uint_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_uint_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     cl_uint_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetProgramInfo_cl_point(
    const cl_point& point_program,
    const cl_program_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_point* cl_point_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_point* cl_point_ret_inter = cl_point_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_point_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     cl_point_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetProgramInfo_cl_point_list(
    const cl_point& point_program,
    const cl_program_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::vector<cl_point>* cl_point_list_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_point* param_value = NULL;
  cl_point c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (!return_variable_null_status[1] && param_value_size >= sizeof(cl_point))
    param_value = new cl_point[param_value_size/sizeof(cl_point)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(cl_point)) {
    for (cl_uint index = 0; index < param_value_size/sizeof(cl_point); ++index)
      (*cl_point_list_ret).push_back(param_value[index]);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetProgramInfo_string(
    const cl_point& point_program,
    const cl_program_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::string* string_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  char* param_value = NULL;
  char c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (!return_variable_null_status[1] && param_value_size >= sizeof(char))
    param_value = new char[param_value_size/sizeof(char)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(char)) {
    (*string_ret) = std::string(param_value);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetProgramInfo_size_t_list(
    const cl_point& point_program,
    const cl_program_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::vector<size_t>* size_t_list_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  size_t* param_value = NULL;
  size_t c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (!return_variable_null_status[1] && param_value_size >= sizeof(size_t))
    param_value = new size_t[param_value_size/sizeof(size_t)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(size_t)) {
    for (cl_uint index = 0; index < param_value_size/sizeof(size_t); ++index)
      (*size_t_list_ret).push_back(param_value[index]);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetProgramInfo_string_list(
    const cl_point& point_program,
    const cl_program_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::vector<std::string>* string_list_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  char **param_value = new char*[param_value_size/sizeof(char*)];
  std::string c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(std::string)) {
    for (cl_uint index = 0; index < param_value_size/sizeof(std::string); ++index)
      (*string_list_ret).push_back(param_value[index]);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetProgramInfo_size_t(
    const cl_point& point_program,
    const cl_program_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    size_t *size_t_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  size_t *size_t_ret_inter = size_t_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    size_t_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     size_t_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclCreateProgramWithSource(
    const cl_point& point_context,
    const cl_uint& count,
    const std::vector<std::string>& string_list,
    const std::vector<size_t>& length_list,
    const std::vector<bool>& return_variable_null_status,
    cl_int* errcode_ret,
    cl_point* point_program_ret) {
  cl_context context = (cl_context) point_context;
  const char **strings = NULL;
  size_t *lengths = NULL;
  cl_program program_ret;
  cl_int* errcode_ret_inter = errcode_ret;

  if (return_variable_null_status[0])
    errcode_ret_inter = NULL;

  if (count > 0 && !string_list.empty()) {
    strings = new const char*[count];
    for(cl_uint index = 0; index < count; ++index) {
      strings[index] = string_list[index].c_str();
    }
  }
  if (count > 0 && !length_list.empty()) {
    lengths = new size_t[count];
    for(cl_uint index = 0; index < count; ++index) {
      lengths[index] = length_list[index];
    }
  }

  program_ret = clApiImpl->doClCreateProgramWithSource(
                    context,
                    count,
                    strings,
                    lengths,
                    errcode_ret_inter);

  if (count > 0 && !string_list.empty()) {
    delete[] strings;
  }

  if (count > 0 && !length_list.empty()) {
    delete[] lengths;
  }

  *point_program_ret = (cl_point) program_ret;
}

void GpuChannel::OnCallclGetProgramBuildInfo_cl_int(
    const cl_point& point_program,
    const cl_point& point_device,
    const cl_program_build_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_int* cl_int_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  cl_device_id device = (cl_device_id) point_device;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_int* cl_int_ret_inter = cl_int_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_int_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetProgramBuildInfo(
                     program,
                     device,
                     param_name,
                     param_value_size,
                     cl_int_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclGetProgramBuildInfo_string(
    const cl_point& point_program,
    const cl_point& point_device,
    const cl_program_build_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    std::string* string_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  cl_device_id device = (cl_device_id) point_device;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  char* param_value = NULL;
  char c;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (!return_variable_null_status[1] && param_value_size >= sizeof(char))
    param_value = new char[param_value_size/sizeof(char)];
  else if (!return_variable_null_status[1])
    param_value = &c;

  *errcode_ret = clApiImpl->doClGetProgramBuildInfo(
                     program,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret_inter);

  if (!return_variable_null_status[1] && param_value_size >= sizeof(char)) {
    (*string_ret) = std::string(param_value);
    delete[] param_value;
  }
}

void GpuChannel::OnCallclGetProgramBuildInfo_cl_uint(
    const cl_point& point_program,
    const cl_point& point_device,
    const cl_program_build_info& param_name,
    const size_t& param_value_size,
    const std::vector<bool>& return_variable_null_status,
    cl_uint* cl_uint_ret,
    size_t* param_value_size_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  cl_device_id device = (cl_device_id) point_device;
  size_t *param_value_size_ret_inter = param_value_size_ret;
  cl_uint* cl_uint_ret_inter = cl_uint_ret;

  if (return_variable_null_status[0])
    param_value_size_ret_inter = NULL;

  if (return_variable_null_status[1])
    cl_uint_ret_inter = NULL;

  *errcode_ret = clApiImpl->doClGetProgramBuildInfo(
                     program,
                     device,
                     param_name,
                     param_value_size,
                     cl_uint_ret_inter,
                     param_value_size_ret_inter);
}

void GpuChannel::OnCallclBuildProgram(
    const cl_point& point_program,
    const cl_uint& num_devices_inter,
    const std::vector<cl_point>& point_devcie_list,
    const std::string& options_string,
    const std::vector<cl_point>& key_list,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  cl_uint num_devices = num_devices_inter;
  cl_device_id* devices = new cl_device_id[num_devices];
  for(unsigned i=0; i<num_devices; i++) {
    devices[i] = (cl_device_id)point_devcie_list[i];
  }
  const char* options = options_string.c_str();

#if defined(OS_ANDROID)
  unsigned* event_callback_keys = new unsigned[5];
  event_callback_keys[0] = key_list[0];//event_key
  event_callback_keys[1] = 0;//callback type
  event_callback_keys[2] = key_list[1];//hadler id
  event_callback_keys[3] = key_list[2];//object type
#elif defined(OS_LINUX)
  cl_point* event_callback_keys = new cl_point[5];
  event_callback_keys[0] = key_list[0];//event_key
  event_callback_keys[1] = 0;//callback type
  event_callback_keys[2] = key_list[1];//hadler id
  event_callback_keys[3] = key_list[2];//object type
#endif

  *errcode_ret = clApiImpl->doClBuildProgram(
                      program,
                      num_devices,
                      devices,
                      options,
                      NULL,
                      event_callback_keys);
}

void GpuChannel::OnCallclEnqueueMarker(
  const cl_point& command_queue,
  cl_point* event_ret,
  cl_int* errcode_ret)
{
  cl_event event_ret_inter = nullptr;

  *errcode_ret = clApiImpl->doClEnqueueMarker(
      (cl_command_queue)command_queue,
      &event_ret_inter
  );

  *event_ret = (cl_point)event_ret_inter;
}

void GpuChannel::OnCallclEnqueueBarrier(
  const cl_point& command_queue,
  cl_int* errcode_ret)
{
  *errcode_ret = clApiImpl->doClEnqueueBarrier(
      (cl_command_queue)command_queue
  );
}

void GpuChannel::OnCallclEnqueueWaitForEvents(
  const cl_point& command_queue,
  const std::vector<cl_point>& event_list,
  const cl_uint& num_events,
  cl_int* errcode_ret)
{
  cl_event* event_list_inter = new cl_event[num_events];
  for(size_t i=0; i<num_events; i++)
    event_list_inter[i] = (cl_event)event_list[i];

  *errcode_ret = clApiImpl->doClEnqueueWaitForEvents(
      (cl_command_queue)command_queue,
      num_events,
      event_list_inter
  );

  delete[] event_list_inter;
}

void GpuChannel::OnCallclCreateKernel(
    const cl_point& point_program,
    const std::string& string_kernel_name,
    const std::vector<bool>& return_variable_null_status,
    cl_int* errcode_ret,
    cl_point* point_kernel_ret) {
  cl_program program = (cl_program) point_program;
  cl_kernel kernel_ret;
  cl_int* errcode_ret_inter = errcode_ret;

  if (return_variable_null_status[0])
    errcode_ret_inter = NULL;

  kernel_ret = clApiImpl->doClCreateKernel(
                   program,
                   string_kernel_name.c_str(),
                   errcode_ret_inter);

  *point_kernel_ret = (cl_point) kernel_ret;
}

void GpuChannel::OnCallclCreateKernelsInProgram(
    const cl_point& point_program,
    const cl_uint& num_kernels,
    const std::vector<cl_point>& point_kernel_list,
    const std::vector<bool>& return_variable_null_status,
    std::vector<cl_point>* kernel_list_ret,
    cl_uint* num_kernels_ret,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;
  cl_kernel* kernels = NULL;
  cl_uint *num_kernels_ret_inter = num_kernels_ret;

  if (return_variable_null_status[0])
    num_kernels_ret_inter = NULL;

  if (num_kernels > 0) {
    kernels = new cl_kernel[num_kernels];
  }

  *errcode_ret = clApiImpl->doClCreateKernelsInProgram(
                     program,
                     num_kernels,
                     kernels,
                     num_kernels_ret_inter);

  if(num_kernels > 0) {
    kernel_list_ret->resize(num_kernels);

    for(size_t i=0; i<num_kernels; i++)
      (*kernel_list_ret)[i] = (cl_point)kernels[i];
  }

  if (num_kernels > 0)
    delete[] kernels;
}

void GpuChannel::OnCallclReleaseProgram(
    const cl_point& point_program,
    cl_int* errcode_ret) {
  cl_program program = (cl_program) point_program;

  *errcode_ret = clApiImpl->doClReleaseProgram(program);
}

// gl/cl sharing
void GpuChannel::OnCallGetGLContext(
    cl_point* glContext, cl_point* glDisplay) {
  // *glContext = reinterpret_cast<cl_uint>(share_group()->GetContext()->GetHandle());

  gl::GLContext* context = share_group()->GetContext();
  gl::GLContextEGL* eglContext = static_cast<gl::GLContextEGL*>(context);

  CLLOG(INFO) << "OnCallGetGLContext, context : " << context << ", eglContext : " << eglContext;
  CLLOG(INFO) << "OnCallGetGLContext, eglContext->GetDisplayHandle() : " << eglContext->GetDisplayHandle();

  CLLOG(INFO) << "OnCallGetGLContext, share_group()->GetContext()->GetHandle() : " << share_group()->GetContext()->GetHandle();
  CLLOG(INFO) << "OnCallGetGLContext, share_group()->GetSharedContext()->GetHandle() : " << share_group()->GetSharedContext()->GetHandle();

  // CLLOG(INFO) << "OnCallGetGLContext, share_group()->GetContext()->GetHandle() : " << share_group()->GetContext()->GetHandle();

  *glContext = reinterpret_cast<cl_point>(share_group()->GetContext()->GetHandle());
  *glDisplay = reinterpret_cast<cl_point>(eglContext->GetDisplayHandle());
  CLLOG(INFO) << "OnCallGetGLContext, *glContext : " << *glContext << ", *glDisplay : " << *glDisplay;
}

void GpuChannel::OnCallCtrlSetSharedHandles(
	const base::SharedMemoryHandle& data_handle,
	const base::SharedMemoryHandle& operation_handle,
	const base::SharedMemoryHandle& result_handle,
	const base::SharedMemoryHandle& events_handle,
	bool* result)
{
	CLLOG(INFO) << "GpuChannel::OnCallCtrlSetSharedHandles";

	*result = clApiImpl->setSharedMemory(data_handle, operation_handle, result_handle, events_handle);
}

void GpuChannel::OnCallCtrlClearSharedHandles(
	bool* result)
{
	CLLOG(INFO) << "GpuChannel::OnCallCtrlClearSharedHandles";

	*result = clApiImpl->clearSharedMemory();
}

//Vulkan Function
void GpuChannel::OnCallVKCSetSharedHandles(
  const base::SharedMemoryHandle& data_handle,
  const base::SharedMemoryHandle& operation_handle,
  const base::SharedMemoryHandle& result_handle,
  bool* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCSetSharedHandles";

  *result = vkcApiImpl->setSharedMemory(data_handle, operation_handle, result_handle);
}

void GpuChannel::OnCallVKCClearSharedHandles(bool* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCClearSharedHandles";

  *result = vkcApiImpl->clearSharedMemory();
}

void GpuChannel::OnCallVKCCreateInstance(
  const std::vector<std::string>& names,
  const std::vector<uint32_t>& versions,
  const std::vector<std::string>& enabledLayerNames,
  const std::vector<std::string>& enabledExtensionNames,
  VKCPoint* vkcInstance,
  int* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCCreateInstance";

  *result = vkcApiImpl->vkcCreateInstance(names, versions, enabledLayerNames, enabledExtensionNames, vkcInstance);
}

void GpuChannel::OnCallVKCDestroyInstance(const VKCPoint& vkcInstance, int* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCDestroyInstance " << vkcInstance;

  *result = vkcApiImpl->vkcDestroyInstance(vkcInstance);
}

void GpuChannel::OnCallVKCEnumeratePhysicalDevice(const VKCPoint& vkcInstance, VKCuint* physicalDeviceCount, VKCPoint* physicalDeviceList, int* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCEnumeratePhysicalDevice " << vkcInstance;
  *result = vkcApiImpl->vkcEnumeratePhysicalDevice(vkcInstance, physicalDeviceCount, physicalDeviceList);
}

void GpuChannel::OnCallVKCDestroyPhysicalDevice(const VKCPoint& physicalDeviceList, int* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCDestroyPhysicalDevice " << physicalDeviceList;
  *result = vkcApiImpl->vkcDestroyPhysicalDevice(physicalDeviceList);
}

void GpuChannel::OnCallVKCCreateDevice(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, VKCPoint* vkcDevice, VKCPoint* vkcQueue, int* result) {
  VKCLOG(INFO) << "GpuCHannel::OnCallVKCCreateDevice " << vdIndex << ", " << physicalDeviceList;
  *result = vkcApiImpl->vkcCreateDevice(vdIndex, physicalDeviceList, vkcDevice, vkcQueue);
}

void GpuChannel::OnCallVKCDestroyDevice(const VKCPoint& vkcDevice, const VKCPoint& vkcQueue, int* result) {
  VKCLOG(INFO) << "GpuCHannel::OnCallVKCDestroyDevice";

  *result = vkcApiImpl->vkcDestroyDevice(vkcDevice, vkcQueue);
}

void GpuChannel::OnCallVKCGetDeviceInfoUint(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, const VKCuint& name, VKCuint* data_uint, int* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCGetDeviceInfoUint " << vdIndex << ", " << name;

  *result = vkcApiImpl->vkcGetDeviceInfo(vdIndex, physicalDeviceList, name, data_uint);
}

void GpuChannel::OnCallVKCGetDeviceInfoArray(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, const VKCuint& name, std::vector<VKCuint>* data_array, int* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCGetDeviceInfoArray " << vdIndex << ", " << name;

  *result = vkcApiImpl->vkcGetDeviceInfo(vdIndex, physicalDeviceList, name, data_array);
}

void GpuChannel::OnCallVKCGetDeviceInfoString(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, const VKCuint& name, std::string* data_string, int* result) {
  VKCLOG(INFO) << "GpuChannel::OnCallVKCGetDeviceInfoString " << vdIndex << ", " << name;

  *result = vkcApiImpl->vkcGetDeviceInfo(vdIndex, physicalDeviceList, name, data_string);
}

void GpuChannel::OnCallVKCCreateBuffer(const VKCPoint& vkcDevice, const VKCPoint& physicalDeviceList, const VKCuint& vdIndex, const VKCuint& sizeInBytes, VKCPoint* vkcBuffer, VKCPoint* vkcMemory, int* result) {
  VKCLOG(INFO) << "createBuffer : " << vkcDevice << ", " << sizeInBytes;

  *result = vkcApiImpl->vkcCreateBuffer(vkcDevice, physicalDeviceList, vdIndex, sizeInBytes, vkcBuffer, vkcMemory);
}

void GpuChannel::OnCallVKCReleaseBuffer(const VKCPoint& vkcDevice, const VKCPoint& vkcBuffer, const VKCPoint& vkcMemory, int* result) {
  VKCLOG(INFO) << "OnCallVKCReleaseBuffer : " << vkcDevice << ", " << vkcBuffer << ", " << vkcMemory;

  *result = vkcApiImpl->vkcReleaseBuffer(vkcDevice, vkcBuffer, vkcMemory);
}

void GpuChannel::OnCallVKCFillBuffer(const VKCPoint& vkcDevice, const VKCPoint& vkcMemory, const std::vector<VKCuint>& uintVector, int* result) {
  VKCLOG(INFO) << "OnCallVKCFillBuffer : " << vkcDevice << ", " << vkcMemory << ", " << uintVector[0] << ", " << uintVector[1];

  *result = vkcApiImpl->vkcFillBuffer(vkcDevice, vkcMemory, uintVector);
}

void GpuChannel::OnCallVKCCreateCommandQueue(const VKCPoint& vkcDevice, const VKCPoint& physicalDeviceList, const VKCuint& vdIndex, VKCPoint* vkcCMDBuffer, VKCPoint* vkcCMDPool, int* result) {
  VKCLOG(INFO) << "OnCallVKCCreateCommandQueue : " << vkcDevice << ", " << physicalDeviceList << ", " << vdIndex;

  *result = vkcApiImpl->vkcCreateCommandQueue(vkcDevice, physicalDeviceList, vdIndex, vkcCMDBuffer, vkcCMDPool);
}

void GpuChannel::OnCallVKCReleaseCommandQueue(const VKCPoint& vkcDevice, const VKCPoint& vkcCMDBuffer, const VKCPoint& vkcCMDPool, int* result) {
  VKCLOG(INFO) << "OnCallVKCReleaseCommandQueue : " << vkcDevice << ", " << vkcCMDBuffer << ", " << vkcCMDPool;

  *result = vkcApiImpl->vkcReleaseCommandQueue(vkcDevice, vkcCMDBuffer, vkcCMDPool);
}

void GpuChannel::OnCallVKCCreateDescriptorSetLayout(const VKCPoint& vkcDevice, const VKCuint& useBufferCount, VKCPoint* vkcDescriptorSetLayout, int* result) {
  VKCLOG(INFO) << "OnCallVKCCreateDescriptorSetLayout : " << vkcDevice << ", " << useBufferCount;
  *result = vkcApiImpl->vkcCreateDescriptorSetLayout(vkcDevice, useBufferCount, vkcDescriptorSetLayout);
}

void GpuChannel::OnCallVKCReleaseDescriptorSetLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSetLayout, int* result) {
  VKCLOG(INFO) << "OnCallVKCReleaseDescriptorSetLayout : " << vkcDevice << ", " << vkcDescriptorSetLayout;
  *result = vkcApiImpl->vkcReleaseDescriptorSetLayout(vkcDevice, vkcDescriptorSetLayout);
}

void GpuChannel::OnCallVKCCreateDescriptorPool(const VKCPoint& vkcDevice, const VKCuint& useBufferCount, VKCPoint* vkcDescriptorPool, int* result) {
  VKCLOG(INFO) << "OnCallVKCCreateDescriptorPool : " << vkcDevice << ", " << useBufferCount;
  *result = vkcApiImpl->vkcCreateDescriptorPool(vkcDevice, useBufferCount, vkcDescriptorPool);
}

void GpuChannel::OnCallVKCReleaseDescriptorPool(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, int* result) {
  VKCLOG(INFO) << "OnCallVKCReleaseDescriptorPool : " << vkcDevice << ", " << vkcDescriptorPool;
  *result = vkcApiImpl->vkcReleaseDescriptorPool(vkcDevice, vkcDescriptorPool);
}

void GpuChannel::OnCallVKCCreateDescriptorSet(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, const VKCPoint& vkcDescriptorSetLayout, VKCPoint* vkcDescriptorSet, int* result) {
  VKCLOG(INFO) << "OnCallVKCCreateDescriptorSet : " << vkcDevice << ", " << vkcDescriptorPool << ", " << vkcDescriptorSetLayout;
  *result = vkcApiImpl->vkcCreateDescriptorSet(vkcDevice, vkcDescriptorPool, vkcDescriptorSetLayout, vkcDescriptorSet);
}

void GpuChannel::OnCallVKCReleaseDescriptorSet(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, const VKCPoint& vkcDescriptorSet, int* result) {
  VKCLOG(INFO) << "OnCallVKCReleaseDescriptorSet : " << vkcDevice << ", " <<vkcDescriptorPool << ", " << vkcDescriptorSet;
  *result = vkcApiImpl->vkcReleaseDescriptorSet(vkcDevice, vkcDescriptorPool, vkcDescriptorSet);
}

void GpuChannel::OnCallVKCCreatePipelineLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSetLayout, VKCPoint* vkcPipelineLayout, int* result) {
  VKCLOG(INFO) << "OnCallVKCCreatePipelineLayout : " << vkcDevice << ", " << vkcDescriptorSetLayout;
  *result = vkcApiImpl->vkcCreatePipelineLayout(vkcDevice, vkcDescriptorSetLayout, vkcPipelineLayout);
}

void GpuChannel::OnCallVKCReleasePipelineLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineLayout, int* result) {
  VKCLOG(INFO) << "OnCallVKCReleasePipelineLyaout : " << vkcDevice << ", " << vkcPipelineLayout;
  *result = vkcApiImpl->vkcReleasePipelineLayout(vkcDevice, vkcPipelineLayout);
}

void GpuChannel::OnCallVKCCreateShaderModule(const VKCPoint& vkcDevice, const std::string& shaderPath, VKCPoint* vkcShaderModule, int* result) {
  VKCLOG(INFO) << "OnCallVKCCreateShaderModule : " << vkcDevice << ", " << shaderPath;
  *result = vkcApiImpl->vkcCreateShaderModule(vkcDevice, shaderPath, vkcShaderModule);
}
void GpuChannel::OnCallVKCReleaseShaderModule(const VKCPoint& vkcDevice, const VKCPoint& vkcShaderModule, int* result) {
  VKCLOG(INFO) << "OnCallVKCReleaseShaderModule : " << vkcDevice << ", " << vkcShaderModule;
  *result = vkcApiImpl->vkcReleaseShaderModule(vkcDevice, vkcShaderModule);
}

void GpuChannel::OnCallVKCCreatePipeline(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineLayout, const VKCPoint& vkcShaderModule, VKCPoint* vkcPipelineCache, VKCPoint* vkcPipeline, int* result) {
  VKCLOG(INFO) << "OnCallVKCCreatePipeline : " << vkcDevice << ", " << vkcPipelineLayout << ", " << vkcShaderModule;
  *result = vkcApiImpl->vkcCreatePipeline(vkcDevice, vkcPipelineLayout, vkcShaderModule, vkcPipelineCache, vkcPipeline);
}

void GpuChannel::OnCallVKCReleasePipeline(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineCache, const VKCPoint& vkcPipeline, int* result) {
  VKCLOG(INFO) << "OnCallVKCReleasePipeline : " << vkcDevice << ", " << vkcPipelineCache << ", " << vkcPipeline;
  *result = vkcApiImpl->vkcReleasePipeline(vkcDevice, vkcPipelineCache, vkcPipeline);
}

void GpuChannel::OnCallVKCUpdateDescriptorSets(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSet, const std::vector<VKCPoint>& bufferVector, int* result) {
  VKCLOG(INFO) << "OnCallVKCUpdateDescriptorSets : " << vkcDevice << ", " << vkcDescriptorSet << ", "  << bufferVector.size();
  *result = vkcApiImpl->vkcUpdateDescriptorSets(vkcDevice, vkcDescriptorSet, bufferVector);
}

void GpuChannel::OnCallVKCBeginQueue(const VKCPoint& vkcCMDBuffer, const VKCPoint& vkcPipeline, const VKCPoint& vkcPipelineLayout, const VKCPoint& vkcDescriptorSet, int* result) {
  VKCLOG(INFO) << "OnCallVKCBeginQueue : " << vkcCMDBuffer << ", " << vkcPipeline << ", " << vkcPipelineLayout << ", " << vkcDescriptorSet;
  *result = vkcApiImpl->vkcBeginQueue(vkcCMDBuffer, vkcPipeline, vkcPipelineLayout, vkcDescriptorSet);
}

void GpuChannel::OnCallVKCEndQueue(const VKCPoint& vkcCMDBuffer, int* result) {
  VKCLOG(INFO) << "OnCallVKCEndQueue : " << vkcCMDBuffer;
  *result = vkcApiImpl->vkcEndQueue(vkcCMDBuffer);
}

void GpuChannel::OnCallVKCDispatch(const VKCPoint& vkcCMDBuffer, const VKCuint& workGroupX, const VKCuint& workGroupY, const VKCuint& workGroupZ, int* result) {
  // VKCLOG(INFO) << "OnCallVKCDispatch : " << vkcCMDBuffer << ", " << workGroupX;
  *result = vkcApiImpl->vkcDispatch(vkcCMDBuffer, workGroupX, workGroupY, workGroupZ);
}

void GpuChannel::OnCallVKCPipelineBarrier(const VKCPoint& vkcCMDBuffer, int* result) {
  // VKCLOG(INFO) << "OnCallVKCPipelineBarrier : " << vkcCMDBuffer;
  *result = vkcApiImpl->vkcPipelineBarrier(vkcCMDBuffer);
}

void GpuChannel::OnCallVKCCmdCopyBuffer(const VKCPoint& vkcCMDBuffer, const VKCPoint& srcBuffer, const VKCPoint& dstBuffer, const VKCuint& copySize, int* result) {
  VKCLOG(INFO) << "OnCallVKCCmdCopyBuffer : " << vkcCMDBuffer << ", " << srcBuffer << ", " << dstBuffer << ", " << copySize;
  *result = vkcApiImpl->vkcCmdCopyBuffer(vkcCMDBuffer, srcBuffer, dstBuffer, copySize);
}

void GpuChannel::OnCallVKCQueueSubmit(const VKCPoint& vkcQueue, const VKCPoint& vkcCMDBuffer, int* result) {
  VKCLOG(INFO) << "OnCallVKCQueueSubmit : " << vkcQueue << ", " << vkcCMDBuffer;
  *result = vkcApiImpl->vkcQueueSubmit(vkcQueue, vkcCMDBuffer);
}

void GpuChannel::OnCallVKCDeviceWaitIdle(const VKCPoint& vkcDevice, int* result) {
  VKCLOG(INFO) << "OnCallVKCDeviceWaitIdle : " << vkcDevice;
  *result = vkcApiImpl->vkcDeviceWaitIdle(vkcDevice);
}

void GpuChannel::CacheShader(const std::string& key,
                             const std::string& shader) {
  gpu_channel_manager_->delegate()->StoreShaderToDisk(client_id_, key, shader);
}

void GpuChannel::AddFilter(IPC::MessageFilter* filter) {
  io_task_runner_->PostTask(
      FROM_HERE, base::Bind(&GpuChannelMessageFilter::AddChannelFilter,
                            filter_, make_scoped_refptr(filter)));
}

void GpuChannel::RemoveFilter(IPC::MessageFilter* filter) {
  io_task_runner_->PostTask(
      FROM_HERE, base::Bind(&GpuChannelMessageFilter::RemoveChannelFilter,
                            filter_, make_scoped_refptr(filter)));
}

uint64_t GpuChannel::GetMemoryUsage() {
  // Collect the unique memory trackers in use by the |stubs_|.
  std::set<gles2::MemoryTracker*> unique_memory_trackers;
  for (auto& kv : stubs_)
    unique_memory_trackers.insert(kv.second->GetMemoryTracker());

  // Sum the memory usage for all unique memory trackers.
  uint64_t size = 0;
  for (auto* tracker : unique_memory_trackers) {
    size += gpu_channel_manager()->gpu_memory_manager()->GetTrackerMemoryUsage(
        tracker);
  }

  return size;
}

scoped_refptr<gl::GLImage> GpuChannel::CreateImageForGpuMemoryBuffer(
    const gfx::GpuMemoryBufferHandle& handle,
    const gfx::Size& size,
    gfx::BufferFormat format,
    uint32_t internalformat) {
  switch (handle.type) {
    case gfx::SHARED_MEMORY_BUFFER: {
      if (!base::IsValueInRangeForNumericType<size_t>(handle.stride))
        return nullptr;
      scoped_refptr<gl::GLImageSharedMemory> image(
          new gl::GLImageSharedMemory(size, internalformat));
      if (!image->Initialize(handle.handle, handle.id, format, handle.offset,
                             handle.stride)) {
        return nullptr;
      }

      return image;
    }
    default: {
      GpuChannelManager* manager = gpu_channel_manager();
      if (!manager->gpu_memory_buffer_factory())
        return nullptr;

      return manager->gpu_memory_buffer_factory()
          ->AsImageFactory()
          ->CreateImageForGpuMemoryBuffer(handle,
                                          size,
                                          format,
                                          internalformat,
                                          client_id_);
    }
  }
}

}  // namespace gpu
