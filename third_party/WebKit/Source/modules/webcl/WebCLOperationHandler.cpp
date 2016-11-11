// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "wtf/build_config.h"
#include "WebCLOperationHandler.h"

// #include "content/public/renderer/render_thread.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/memory/shared_memory.h"
#include "base/process/process_handle.h"
#include "public/platform/Platform.h"

namespace blink {
namespace {
static int kMaxBufferSize = 1024 * 1024 * 20;
static int kMaxEventNum = 32;
static int kMaxEventSize = 8 * 32;
}

WebCLOperationHandler::WebCLOperationHandler()
	: mIsShared(false)
{
}

WebCLOperationHandler::~WebCLOperationHandler()
{

}

void WebCLOperationHandler::startHandling()
{
	if(!mIsShared) {
		base::SharedMemoryHandle dataHandle;
		base::SharedMemoryHandle operationHandle;
		base::SharedMemoryHandle resultHandle;
		base::SharedMemoryHandle eventsHandle;

#if defined(OS_ANDROID)
		mSharedData = adoptPtr(new base::SharedMemory());
		mSharedData->CreateAndMapAnonymous(kMaxBufferSize);
#elif defined(OS_LINUX)
		// mSharedData = adoptPtr(content::RenderThread::Get()->HostAllocateSharedMemoryBuffer(kMaxBufferSize).release());
		mSharedData = adoptPtr(Platform::current()->getSharedMemoryForWebCL(kMaxBufferSize));
		mSharedData->Map(kMaxBufferSize);
#endif
		mSharedData->ShareToProcess(base::GetCurrentProcessHandle(), &dataHandle);
		mSharedDataPtr = static_cast<char*>(mSharedData->memory());

#if defined(OS_ANDROID)
		mSharedOperation = adoptPtr(new base::SharedMemory());
		mSharedOperation->CreateAndMapAnonymous(sizeof(BaseOperationData));
#elif defined(OS_LINUX)
		// mSharedOperation = adoptPtr(content::RenderThread::Get()->HostAllocateSharedMemoryBuffer(sizeof(BaseOperationData)).release());
		 mSharedOperation = adoptPtr(Platform::current()->getSharedMemoryForWebCL(sizeof(BaseOperationData)));
		mSharedOperation->Map(sizeof(BaseOperationData));
#endif
		mSharedOperation->ShareToProcess(base::GetCurrentProcessHandle(), &operationHandle);
		mSharedOperationPtr = static_cast<BaseOperationData*>(mSharedOperation->memory());

#if defined(OS_ANDROID)
		mSharedResult = adoptPtr(new base::SharedMemory());
		mSharedResult->CreateAndMapAnonymous(sizeof(BaseResultData));
#elif defined(OS_LINUX)
		// mSharedResult = adoptPtr(content::RenderThread::Get()->HostAllocateSharedMemoryBuffer(sizeof(BaseResultData)).release());
		mSharedResult = adoptPtr(Platform::current()->getSharedMemoryForWebCL(sizeof(BaseResultData)));
		mSharedResult->Map(sizeof(BaseResultData));
#endif
		mSharedResult->ShareToProcess(base::GetCurrentProcessHandle(), &resultHandle);
		mSharedResultPtr = static_cast<BaseResultData*>(mSharedResult->memory());

#if defined(OS_ANDROID)
		mSharedEvents = adoptPtr(new base::SharedMemory());
		mSharedEvents->CreateAndMapAnonymous(kMaxEventSize);
#elif defined(OS_LINUX)
		// mSharedEvents = adoptPtr(content::RenderThread::Get()->HostAllocateSharedMemoryBuffer(kMaxEventSize).release());
		mSharedEvents = adoptPtr(Platform::current()->getSharedMemoryForWebCL(kMaxEventSize));
		mSharedEvents->Map(kMaxEventSize);
#endif
		mSharedEvents->ShareToProcess(base::GetCurrentProcessHandle(), &eventsHandle);
		mSharedEventsPtr = static_cast<cl_point*>(mSharedEvents->memory());

		mIsShared = webcl_ctrlSetSharedHandles(webcl_channel_, dataHandle, operationHandle, resultHandle, eventsHandle);
	}
}

bool WebCLOperationHandler::canShareOperation()
{
	return mIsShared;
}

void WebCLOperationHandler::setOperationParameter(WebCL_Operation_Base* paramPtr)
{
	DCHECK(paramPtr);

	switch(paramPtr->operation_type) {
	case OPENCL_OPERATION_TYPE::SET_ARG: {
		WebCL_Operation_setArg* tmp = static_cast<WebCL_Operation_setArg*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->kernel;
		mSharedOperationPtr->ptr_2 = tmp->ptr_arg;
		mSharedOperationPtr->uint_01 = tmp->arg_index;
		mSharedOperationPtr->uint_02 = tmp->arg_size;
		mSharedOperationPtr->int_01 = tmp->data_type;
		mSharedOperationPtr->bool_01 = tmp->is_local;
	}
	break;
	case OPENCL_OPERATION_TYPE::CREATE_BUFFER: {
		WebCL_Operation_createBuffer* tmp = static_cast<WebCL_Operation_createBuffer*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->context;
		mSharedOperationPtr->ulong_01 = tmp->flags;
		mSharedOperationPtr->uint_01 = tmp->size;
		mSharedOperationPtr->bool_01 = tmp->use_host_ptr;
	}
	break;
	// gl/cl sharing
	case OPENCL_OPERATION_TYPE::CREATE_BUFFER_FROM_GL_BUFFER: {
		WebCL_Operation_createBufferFromGLBuffer* tmp = static_cast<WebCL_Operation_createBufferFromGLBuffer*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->context;
		mSharedOperationPtr->ulong_01 = tmp->flags;
		mSharedOperationPtr->uint_01 = tmp->bufobj;
	}
	break;
	case OPENCL_OPERATION_TYPE::CREATE_IMAGE_FROM_GL_RENDER_BUFFER: {
		WebCL_Operation_createImageFromGLRenderbuffer* tmp = static_cast<WebCL_Operation_createImageFromGLRenderbuffer*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->context;
		mSharedOperationPtr->ulong_01 = tmp->flags;
		mSharedOperationPtr->uint_01 = tmp->bufobj;
	}
	break;
	case OPENCL_OPERATION_TYPE::CREATE_IMAGE_FROM_GL_TEXTURE: {
		WebCL_Operation_createImageFromGLTexture* tmp = static_cast<WebCL_Operation_createImageFromGLTexture*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->context;
		mSharedOperationPtr->ulong_01 = tmp->flags;
		mSharedOperationPtr->ulong_02 = tmp->textureTarget;
		mSharedOperationPtr->int_01 = tmp->miplevel;
		mSharedOperationPtr->uint_01 = tmp->textureobj;
	}
	break;
	case OPENCL_OPERATION_TYPE::CREATE_IMAGE: {
		WebCL_Operation_createImage* tmp = static_cast<WebCL_Operation_createImage*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->context;
		mSharedOperationPtr->ulong_01 = tmp->flags;
		mSharedOperationPtr->uint_01 = tmp->channel_order;
		mSharedOperationPtr->uint_02 = tmp->channel_type;
		mSharedOperationPtr->uint_03 = tmp->image_width;
		mSharedOperationPtr->uint_04 = tmp->image_height;
		mSharedOperationPtr->uint_05 = tmp->image_row_pitch;
		mSharedOperationPtr->bool_01 = tmp->use_host_ptr;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER: {
		WebCL_Operation_enqueueCopyBuffer* tmp = static_cast<WebCL_Operation_enqueueCopyBuffer*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->src_buffer;
		mSharedOperationPtr->ptr_3 = tmp->dst_buffer;
		mSharedOperationPtr->uint_01 = tmp->src_offset;
		mSharedOperationPtr->uint_02 = tmp->dst_offset;
		mSharedOperationPtr->uint_03 = tmp->num_bytes;
		mSharedOperationPtr->uint_04 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->int_01 = tmp->data_type;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER_RECT: {
		WebCL_Operation_enqueueCopyBufferRect* tmp = static_cast<WebCL_Operation_enqueueCopyBufferRect*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->src_buffer;
		mSharedOperationPtr->ptr_3 = tmp->dst_buffer;
		memcpy(mSharedOperationPtr->uint_arr_01, tmp->src_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_02, tmp->dst_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_03, tmp->region, 3*sizeof(size_t));
		mSharedOperationPtr->uint_01 = tmp->src_row_pitch;
		mSharedOperationPtr->uint_02 = tmp->src_slice_pitch;
		mSharedOperationPtr->uint_03 = tmp->dst_row_pitch;
		mSharedOperationPtr->uint_04 = tmp->dst_slice_pitch;
		mSharedOperationPtr->uint_05 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->int_01 = tmp->data_type;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_IMAGE: {
		WebCL_Operation_enqueueCopyImage* tmp = static_cast<WebCL_Operation_enqueueCopyImage*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->src_image;
		mSharedOperationPtr->ptr_3 = tmp->dst_image;
		memcpy(mSharedOperationPtr->uint_arr_01, tmp->src_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_02, tmp->dst_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_03, tmp->region, 3*sizeof(size_t));
		mSharedOperationPtr->uint_01= tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->int_01 = tmp->data_type;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER_TO_IMAGE: {
		WebCL_Operation_enqueueCopyBufferToImage* tmp = static_cast<WebCL_Operation_enqueueCopyBufferToImage*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->src_buffer;
		mSharedOperationPtr->ptr_3 = tmp->dst_image;
		mSharedOperationPtr->uint_01 = tmp->src_offset;
		memcpy(mSharedOperationPtr->uint_arr_01, tmp->dst_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_02, tmp->region, 3*sizeof(size_t));
		mSharedOperationPtr->uint_02 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->int_01 = tmp->data_type;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_COPY_IMAGE_TO_BUFFER: {
		WebCL_Operation_enqueueCopyImageToBuffer* tmp = static_cast<WebCL_Operation_enqueueCopyImageToBuffer*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->src_image;
		mSharedOperationPtr->ptr_3 = tmp->dst_buffer;
		memcpy(mSharedOperationPtr->uint_arr_01, tmp->src_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_02, tmp->region, 3*sizeof(size_t));
		mSharedOperationPtr->uint_01 = tmp->dst_offset;
		mSharedOperationPtr->uint_02 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->int_01 = tmp->data_type;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER: {
		WebCL_Operation_enqueueReadBuffer* tmp = static_cast<WebCL_Operation_enqueueReadBuffer*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->buffer;
		mSharedOperationPtr->bool_01 = tmp->blocking;
		mSharedOperationPtr->uint_01 = tmp->buffer_offset;
		mSharedOperationPtr->uint_02 = tmp->num_bytes;
		mSharedOperationPtr->uint_03 = tmp->event_length;
		mSharedOperationPtr->bool_02 = tmp->need_event;
		mSharedOperationPtr->int_01 = tmp->data_type;
		mSharedOperationPtr->uint_04 = tmp->data_size;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER_RECT: {
		WebCL_Operation_enqueueReadBufferRect* tmp = static_cast<WebCL_Operation_enqueueReadBufferRect*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->buffer;
		memcpy(mSharedOperationPtr->uint_arr_01, tmp->buffer_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_02, tmp->host_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_03, tmp->region, 3*sizeof(size_t));
		mSharedOperationPtr->uint_01 = tmp->buffer_row_pitch;
		mSharedOperationPtr->uint_02 = tmp->buffer_slice_pitch;
		mSharedOperationPtr->uint_03 = tmp->host_row_pitch;
		mSharedOperationPtr->uint_04 = tmp->host_slice_pitch;
		mSharedOperationPtr->uint_05 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->bool_02 = tmp->blocking;
		mSharedOperationPtr->int_01 = tmp->data_type;
		mSharedOperationPtr->uint_06 = tmp->data_size;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_READ_IMAGE: {
		WebCL_Operation_enqueueReadImage* tmp = static_cast<WebCL_Operation_enqueueReadImage*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->image;
		memcpy(mSharedOperationPtr->uint_arr_01, tmp->origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_02, tmp->region, 3*sizeof(size_t));
		mSharedOperationPtr->uint_01 = tmp->row_pitch;
		mSharedOperationPtr->uint_02 = tmp->slice_pitch;
		mSharedOperationPtr->uint_03 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->bool_02 = tmp->blocking;
		mSharedOperationPtr->int_01 = tmp->data_type;
		mSharedOperationPtr->uint_04 = tmp->data_size;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER: {
		WebCL_Operation_enqueueWriteBuffer* tmp = static_cast<WebCL_Operation_enqueueWriteBuffer*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->buffer;
		mSharedOperationPtr->uint_01 = tmp->buffer_offset;
		mSharedOperationPtr->uint_02 = tmp->num_bytes;
		mSharedOperationPtr->uint_03 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->bool_02 = tmp->blocking;
		mSharedOperationPtr->int_01 = tmp->data_type;
		mSharedOperationPtr->uint_04 = tmp->data_size;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER_RECT: {
		WebCL_Operation_enqueueWriteBufferRect* tmp = static_cast<WebCL_Operation_enqueueWriteBufferRect*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->buffer;
		memcpy(mSharedOperationPtr->uint_arr_01, tmp->buffer_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_02, tmp->host_origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_03, tmp->region, 3*sizeof(size_t));
		mSharedOperationPtr->uint_01 = tmp->buffer_row_pitch;
		mSharedOperationPtr->uint_02 = tmp->buffer_slice_pitch;
		mSharedOperationPtr->uint_03 = tmp->host_row_pitch;
		mSharedOperationPtr->uint_04 = tmp->host_slice_pitch;
		mSharedOperationPtr->uint_05 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->bool_02 = tmp->blocking;
		mSharedOperationPtr->int_01 = tmp->data_type;
		mSharedOperationPtr->uint_06 = tmp->data_size;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_IMAGE: {
		WebCL_Operation_enqueueWriteImage* tmp = static_cast<WebCL_Operation_enqueueWriteImage*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->image;
		memcpy(mSharedOperationPtr->uint_arr_01, tmp->origin, 3*sizeof(size_t));
		memcpy(mSharedOperationPtr->uint_arr_02, tmp->region, 3*sizeof(size_t));
		mSharedOperationPtr->uint_01 = tmp->input_row_pitch;
		mSharedOperationPtr->uint_02 = tmp->input_slice_pitch;
		mSharedOperationPtr->uint_03 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
		mSharedOperationPtr->bool_02 = tmp->blocking;
		mSharedOperationPtr->int_01 = tmp->data_type;
		mSharedOperationPtr->uint_04 = tmp->data_size;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_NDRANGE_KERNEL: {
		WebCL_Operation_enqueueNDRangeKernel* tmp = static_cast<WebCL_Operation_enqueueNDRangeKernel*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
		mSharedOperationPtr->ptr_2 = tmp->kernel;
		mSharedOperationPtr->uint_01 = tmp->work_dim;
		mSharedOperationPtr->uint_02 = tmp->global_work_offset;
		mSharedOperationPtr->uint_03 = tmp->global_work_size;
		mSharedOperationPtr->uint_04 = tmp->local_work_size;
		mSharedOperationPtr->uint_05 = tmp->event_length;
		mSharedOperationPtr->bool_01 = tmp->need_event;
	}
	break;
	case OPENCL_OPERATION_TYPE::FINISH: {
		WebCL_Operation_finish* tmp = static_cast<WebCL_Operation_finish*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->command_queue;
	}
	break;
	case OPENCL_OPERATION_TYPE::GET_GL_OBJECT_INFO: {
		WebCL_Operation_getGLObjectInfo* tmp = static_cast<WebCL_Operation_getGLObjectInfo*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->memobj;
		mSharedOperationPtr->bool_01 = tmp->needObjectType;
		mSharedOperationPtr->bool_02 = tmp->needObjectName;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_ACQUIRE_GLOBJECTS:
	case OPENCL_OPERATION_TYPE::ENQUEUE_RELEASE_GLOBJECTS: {
		WebCL_Operation_enqueueCommonGLObjects* tmp = static_cast<WebCL_Operation_enqueueCommonGLObjects*>(paramPtr);
		mSharedOperationPtr->ptr_1 = tmp->commandQueue;
		mSharedOperationPtr->uint_01 = tmp->numObjects;
		mSharedOperationPtr->uint_02 = tmp->numEventInWaitList;
		mSharedOperationPtr->bool_01 = tmp->need_event;
	}
	break;
	}
}

void WebCLOperationHandler::setOperationData(void* dataPtr, size_t sizeInBytes)
{
	DCHECK(dataPtr);
	DCHECK(sizeInBytes<(size_t)kMaxBufferSize);

	memcpy(mSharedDataPtr, dataPtr, sizeInBytes);
}

void WebCLOperationHandler::setOperationEvents(cl_point* events, size_t numEvents)
{
	CLLOG(INFO) << "WebCLOperationHandler::setOperationEvents, events=" << events << ", numEvents=" << numEvents*sizeof(cl_point);

	DCHECK(events);
	DCHECK(numEvents<(size_t)kMaxEventNum);

	memcpy(mSharedEventsPtr, events, numEvents*sizeof(cl_point));
}

void WebCLOperationHandler::getOperationResult(WebCL_Result_Base* resultPtr)
{
	switch(resultPtr->operation_type) {
	case OPENCL_OPERATION_TYPE::SET_ARG: {
		resultPtr->result = mSharedResultPtr->int_01;
	}
	break;
	case OPENCL_OPERATION_TYPE::ENQUEUE_BASE: {
		WebCL_Result_enqueueOperationBase* tmp = static_cast<WebCL_Result_enqueueOperationBase*>(resultPtr);
		tmp->result = mSharedResultPtr->int_01;
		tmp->ret_event = mSharedResultPtr->ptr_1;
	}
	break;
	case OPENCL_OPERATION_TYPE::CREATE_BUFFER: {
		WebCL_Result_createBuffer* tmp = static_cast<WebCL_Result_createBuffer*>(resultPtr);
		tmp->result = mSharedResultPtr->int_01;
		tmp->buffer = mSharedResultPtr->ptr_1;
	}
	break;
	case OPENCL_OPERATION_TYPE::CREATE_IMAGE: {
		WebCL_Result_createImage* tmp = static_cast<WebCL_Result_createImage*>(resultPtr);
		tmp->result = mSharedResultPtr->int_01;
		tmp->image = mSharedResultPtr->ptr_1;
	}
	break;
	case OPENCL_OPERATION_TYPE::FINISH: {
		WebCL_Result_enqueueOperationBase* tmp = static_cast<WebCL_Result_enqueueOperationBase*>(resultPtr);
		tmp->result = mSharedResultPtr->int_01;
	}
	break;
	case OPENCL_OPERATION_TYPE::CREATE_BUFFER_FROM_GL_BUFFER: {
		WebCL_Result_createFromGLBuffer* tmp = static_cast<WebCL_Result_createFromGLBuffer*>(resultPtr);
		tmp->result = mSharedResultPtr->int_01;
		tmp->buffer = mSharedResultPtr->ptr_1;
		tmp->serviceId = mSharedResultPtr->uint_01;
	}
	break;
	case OPENCL_OPERATION_TYPE::GET_GL_OBJECT_INFO: {
		WebCL_Result_getGLObjectInfo* tmp = static_cast<WebCL_Result_getGLObjectInfo*>(resultPtr);
		tmp->result = mSharedResultPtr->int_01;
		tmp->glObjectType = mSharedResultPtr->uint_01;
		tmp->glObjectName = mSharedResultPtr->uint_02;
		tmp->clGLTextureTarget = mSharedResultPtr->uint_03;
		tmp->clGLMipMapLevel = mSharedResultPtr->uint_04;
	}
	break;
	}
}

void WebCLOperationHandler::sendOperationSignal(int operation)
{
	DCHECK(mIsShared);

	webcl_ctrlTriggerSharedOperation(webcl_channel_, operation);
}

void WebCLOperationHandler::getOperationResultData(void* resultDataPtr, size_t sizeInBytes)
{
	memcpy(resultDataPtr, mSharedDataPtr, sizeInBytes);
}

bool WebCLOperationHandler::finishHandling()
{
	bool result = false;
	if(mIsShared) {
		result = webcl_ctrlClearSharedHandles(webcl_channel_);

		mSharedData->Unmap();
		mSharedOperation->Unmap();
		mSharedResult->Unmap();
		mSharedEvents->Unmap();

		mSharedData->Close();
		mSharedOperation->Close();
		mSharedResult->Close();
		mSharedEvents->Close();

		mSharedData.reset();
		mSharedOperation.reset();
		mSharedResult.reset();
		mSharedEvents.reset();

		mIsShared = false;
	}
	return result;
}

}
