// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "platform/bindings/V8Binding.h"
#include "bindings/modules/v8/V8WebCLCommandQueue.h"
#include "WebCLCommandQueue.h"

#include "core/html/HTMLVideoElement.h"
#include "core/html/HTMLImageElement.h"
#include "core/html/HTMLCanvasElement.h"
#include "core/html/ImageData.h"

#include "WebCL.h"
#include "WebCLValueChecker.h"
#include "WebCLContext.h"
#include "WebCLDevice.h"
#include "WebCLBuffer.h"
#include "WebCLContext.h"
#include "modules/webcl/WebCLImageDescriptor.h"
#include "WebCLMemoryObject.h"
#include "WebCLImage.h"
#include "WebCLInputChecker.h"
#include "WebCLKernel.h"
#include "WebCLDevice.h"
#include "WebCLHTMLUtil.h"
#include "core/dom/custom/WebCL/WebCLException.h"

#include "bindings/modules/v8/V8WebCLMemoryObject.h"

namespace blink {

WebCLCommandQueue::WebCLCommandQueue(WebCL* context, cl_context clContext, cl_command_queue commandQueue)
	: mContext(context),
	  mFinishEvent(nullptr),
	  mNativeClContext(clContext),
	  mClCommandQueue(commandQueue)
{
	mHTMLUtil = new WebCLHTMLUtil();
}

WebCLCommandQueue::~WebCLCommandQueue() {
}

void WebCLCommandQueue::enqueueCopyBuffer(
		WebCLBuffer* srcBuffer,
		WebCLBuffer* dstBuffer,
		unsigned srcOffset,
		unsigned dstOffset,
		unsigned numBytes,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueCopyBuffer";

	int waitEventListSize = 0;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(srcBuffer, dstBuffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	if(!handleSizeOfBuffer(srcBuffer, srcOffset, numBytes, ec) || !handleSizeOfBuffer(dstBuffer, dstOffset, numBytes, ec))
		return;

	if(WebCLInputChecker::isRegionOverlapping(srcBuffer, dstBuffer, srcOffset, dstOffset, numBytes)) {
		CLLOG(INFO) << "srcBuffer and dstBuffer have overlapping region.";
		ec.ThrowDOMException(WebCLException::MEM_COPY_OVERLAP, "WebCLException::MEM_COPY_OVERLAP");
		return;
	}

	mContext->startHandling();

	WebCL_Operation_enqueueCopyBuffer operation = WebCL_Operation_enqueueCopyBuffer();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.src_buffer = (cl_point)srcBuffer->getCLMem();
	operation.dst_buffer = (cl_point)dstBuffer->getCLMem();
	operation.src_offset = srcOffset;
	operation.dst_offset = dstOffset;
	operation.num_bytes = numBytes;
	operation.event_length = waitEventListSize;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueCopyBufferRect(
		WebCLBuffer* srcBuffer,
		WebCLBuffer* dstBuffer,
		Vector<unsigned> srcOrigin,
		Vector<unsigned> dstOrigin,
		Vector<unsigned> region,
		unsigned srcRowPitch,
		unsigned srcSlicePitch,
		unsigned dstRowPitch,
		unsigned dstSlicePitch,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueCopyBufferRect";

	int waitEventListSize = 0;

	if(srcOrigin.size() != 3 || dstOrigin.size() != 3 || region.size() != 3) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(srcBuffer, dstBuffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	Vector<size_t> srcOriginCopy, dstOriginCopy, regionCopy;
	srcOriginCopy.AppendVector(srcOrigin);
	dstOriginCopy.AppendVector(dstOrigin);
	regionCopy.AppendVector(region);

	if(!WebCLInputChecker::isValidRegionForMemoryObject(srcOriginCopy, regionCopy, srcRowPitch, srcSlicePitch, srcBuffer->getSize())
		|| !WebCLInputChecker::isValidRegionForMemoryObject(dstOriginCopy, regionCopy, dstRowPitch, dstSlicePitch, dstBuffer->getSize())) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	size_t srcOffset = srcOrigin[2] * srcSlicePitch + srcOrigin[1] * srcRowPitch + srcOrigin[0];
	size_t dstOffset = dstOrigin[2] * dstSlicePitch + dstOrigin[1] * dstRowPitch + dstOrigin[0];
	size_t numBytes = region[2] * region[1] * region[0];
	if(WebCLInputChecker::isRegionOverlapping(srcBuffer, dstBuffer, srcOffset, dstOffset, numBytes)) {
		ec.ThrowDOMException(WebCLException::MEM_COPY_OVERLAP, "WebCLException::MEM_COPY_OVERLAP");
		return;
	}

	mContext->startHandling();

	WebCL_Operation_enqueueCopyBufferRect operation = WebCL_Operation_enqueueCopyBufferRect();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.src_buffer = (cl_point)srcBuffer->getCLMem();
	operation.dst_buffer = (cl_point)dstBuffer->getCLMem();
#if defined(OS_ANDROID)
	memcpy(operation.src_origin, srcOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.dst_origin, dstOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.src_origin[0] = srcOrigin[0];
	operation.src_origin[1] = srcOrigin[1];
	operation.src_origin[2] = srcOrigin[2];

	operation.dst_origin[0] = dstOrigin[0];
	operation.dst_origin[1] = dstOrigin[1];
	operation.dst_origin[2] = dstOrigin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif


	operation.src_row_pitch = srcRowPitch;
	operation.src_slice_pitch = srcSlicePitch;
	operation.dst_row_pitch = dstRowPitch;
	operation.dst_slice_pitch = dstSlicePitch;
	operation.event_length = waitEventListSize;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER_RECT);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueCopyImage(
		WebCLImage* srcImage,
		WebCLImage* dstImage,
		Vector<unsigned> srcOrigin,
		Vector<unsigned> dstOrigin,
		Vector<unsigned> region,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueCopyImage";

	int waitEventListSize = 0;

	if(srcOrigin.size() != 2 || dstOrigin.size() != 2 || region.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(srcImage, dstImage, ec))
		return;

	if(!handleImageArgs(srcImage, srcOrigin, dstImage, dstOrigin, region, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	srcOrigin.push_back(0);
	dstOrigin.push_back(0);
	region.push_back(1);

	mContext->startHandling();

	WebCL_Operation_enqueueCopyImage operation = WebCL_Operation_enqueueCopyImage();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.src_image = (cl_point)srcImage->getCLMem();
	operation.dst_image = (cl_point)dstImage->getCLMem();
#if defined(OS_ANDROID)
	memcpy(operation.src_origin, srcOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.dst_origin, dstOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.src_origin[0] = srcOrigin[0];
	operation.src_origin[1] = srcOrigin[1];
	operation.src_origin[2] = srcOrigin[2];

	operation.dst_origin[0] = dstOrigin[0];
	operation.dst_origin[1] = dstOrigin[1];
	operation.dst_origin[2] = dstOrigin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.event_length = waitEventListSize;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_COPY_IMAGE);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueCopyImageToBuffer(
		WebCLImage* srcImage,
		WebCLBuffer* dstBuffer,
		Vector<unsigned> srcOrigin,
		Vector<unsigned> srcRegion,
		unsigned dstOffset,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueCopyImageToBuffer";

	int waitEventListSize = 0;

	if(srcOrigin.size() != 2 || srcRegion.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(srcImage, dstBuffer, ec))
		return;

	if(!handleImageArgs(srcImage, srcOrigin, srcRegion, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	WebCLImageDescriptor srcDescriptor;
	srcImage->getInfo(srcDescriptor);

	if(!WebCLInputChecker::isValidRegionForImage(srcImage->getImageDescriptor(), srcOrigin, srcRegion)
		|| !WebCLInputChecker::isValidRegionForBuffer(dstBuffer->getSize(), srcRegion, dstOffset, srcDescriptor)) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	srcOrigin.push_back(0);
	srcRegion.push_back(1);

	mContext->startHandling();

	WebCL_Operation_enqueueCopyImageToBuffer operation = WebCL_Operation_enqueueCopyImageToBuffer();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.src_image = (cl_point)srcImage->getCLMem();
	operation.dst_buffer = (cl_point)dstBuffer->getCLMem();
#if defined(OS_ANDROID)
	memcpy(operation.src_origin, srcOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, srcRegion.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.src_origin[0] = srcOrigin[0];
	operation.src_origin[1] = srcOrigin[1];
	operation.src_origin[2] = srcOrigin[2];

	operation.region[0] = srcRegion[0];
	operation.region[1] = srcRegion[1];
	operation.region[2] = srcRegion[2];
#endif
	operation.dst_offset = dstOffset;
	operation.event_length = waitEventListSize;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_COPY_IMAGE_TO_BUFFER);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueCopyBufferToImage(
		WebCLBuffer* srcBuffer,
		WebCLImage* dstImage,
		unsigned srcOffset,
		Vector<unsigned> dstOrigin,
		Vector<unsigned> dstRegion,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueCopyBufferToImage";

	int waitEventListSize = 0;

	if(dstOrigin.size() != 2 || dstRegion.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(srcBuffer, dstImage, ec))
		return;

	if(!handleImageArgs(dstImage, dstOrigin, dstRegion, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;


	if(!WebCLInputChecker::isValidRegionForImage(dstImage->getImageDescriptor(), dstOrigin, dstRegion)
		|| !WebCLInputChecker::isValidRegionForBuffer(srcBuffer->getSize(), dstRegion, srcOffset, dstImage->getImageDescriptor())) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	dstOrigin.push_back(0);
	dstRegion.push_back(1);

	mContext->startHandling();

	WebCL_Operation_enqueueCopyBufferToImage operation = WebCL_Operation_enqueueCopyBufferToImage();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.src_buffer = (cl_point)srcBuffer->getCLMem();
	operation.dst_image = (cl_point)dstImage->getCLMem();
	operation.src_offset = srcOffset;
#if defined(OS_ANDROID)
	memcpy(operation.dst_origin, dstOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, dstRegion.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.dst_origin[0] = dstOrigin[0];
	operation.dst_origin[1] = dstOrigin[1];
	operation.dst_origin[2] = dstOrigin[2];

	operation.region[0] = dstRegion[0];
	operation.region[1] = dstRegion[1];
	operation.region[2] = dstRegion[2];
#endif
	operation.event_length = waitEventListSize;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER_TO_IMAGE);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueReadBuffer(
		WebCLBuffer* buffer,
		bool blockingRead,
		unsigned bufferOffset,
		unsigned numBytes,
		NotShared<DOMArrayBufferView> hostPtr,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueReadBuffer";

	int waitEventListSize = 0;

	std::vector<cl_point> point_list;
	std::vector<size_t> size_list;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointEventListInter = nullptr;
	if((pointEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingRead, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	if(!WebCLInputChecker::isValidDataSizeForDOMArrayBufferView(numBytes,hostPtr.View())) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(hostPtr.View()->byteLength() < numBytes || buffer->getSize() < bufferOffset+numBytes) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	mContext->startHandling();

	WebCL_Operation_enqueueReadBuffer operation = WebCL_Operation_enqueueReadBuffer();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.buffer = (cl_point)buffer->getCLMem();
	operation.blocking = blockingRead;
	operation.buffer_offset = bufferOffset;
	operation.num_bytes = numBytes;
	operation.event_length = waitEventListSize;
	// operation.need_event = !blockingRead && event;
	operation.need_event = event != nullptr;
	operation.data_type = WebCLInputChecker::fromArrayBufferTypeToDataType(hostPtr.View()->GetType());
	operation.data_size = WebCLInputChecker::fromArrayBufferSizeToListSize(hostPtr.View());
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
		mContext->getOperationResultData(hostPtr.View()->BaseAddress(), hostPtr.View()->byteLength());
	}
}

void WebCLCommandQueue::enqueueReadBuffer(
		WebCLBuffer* buffer,
		bool blockingRead,
		unsigned bufferOffset,
		unsigned numBytes,
		HTMLCanvasElement* dstCanvas,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueReadBuffer with HTMLCanvasElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	int waitEventListSize = 0;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointEventListInter = nullptr;
	if((pointEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingRead, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	Vector<uint8_t> canvasData;
	size_t canvasSize = 0;
	if(!mHTMLUtil->extractDataFromCanvas(dstCanvas, canvasData, canvasSize, ec))
		return;

	if(canvasSize < numBytes || buffer->getSize() < bufferOffset+numBytes) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	mContext->startHandling();

	WebCL_Operation_enqueueReadBuffer operation = WebCL_Operation_enqueueReadBuffer();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.buffer = (cl_point)buffer->getCLMem();
	operation.blocking = blockingRead;
	operation.buffer_offset = bufferOffset;
	operation.num_bytes = numBytes;
	operation.event_length = waitEventListSize;
	// operation.need_event = !blockingRead && event;
	operation.need_event = event != nullptr;
	operation.data_type = HOST_PTR_DATA_TYPE::UNSIGNED_CHAR;
	operation.data_size = canvasSize;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
		mContext->getOperationResultData(canvasData.begin(), canvasSize);
	}
}

void WebCLCommandQueue::enqueueReadBufferRect(
		WebCLBuffer* buffer,
		bool blockingRead,
		Vector<unsigned> bufferOrigin,
		Vector<unsigned> hostOrigin,
		Vector<unsigned> region,
		unsigned bufferRowPitch,
		unsigned bufferSlicePitch,
		unsigned hostRowPitch,
		unsigned hostSlicePitch,
		NotShared<DOMArrayBufferView> hostPtr,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueReadBufferRect";

	int waitEventListSize = 0;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingRead, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	if(bufferOrigin.size() != 3 || hostOrigin.size() != 3 || region.size() != 3) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

    Vector<size_t> bufferOriginCopy, hostOriginCopy, regionCopy;
    bufferOriginCopy.AppendVector(bufferOrigin);
    hostOriginCopy.AppendVector(hostOrigin);
    regionCopy.AppendVector(region);

    if (!WebCLInputChecker::isValidRegionForMemoryObject(hostOriginCopy, regionCopy, hostRowPitch, hostSlicePitch, hostPtr.View()->byteLength())
    	|| !WebCLInputChecker::isValidRegionForMemoryObject(bufferOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, buffer->getSize())) {
        ec.ThrowDOMException(WebCLException::INVALID_VALUE, "Invalid Value.");
        return;
    }

	mContext->startHandling();

	WebCL_Operation_enqueueReadBufferRect operation = WebCL_Operation_enqueueReadBufferRect();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.buffer = (cl_point)buffer->getCLMem();
	operation.blocking = blockingRead;
#if defined(OS_ANDROID)
	memcpy(operation.buffer_origin, bufferOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.host_origin, hostOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.buffer_origin[0] = bufferOrigin[0];
	operation.buffer_origin[1] = bufferOrigin[1];
	operation.buffer_origin[2] = bufferOrigin[2];

	operation.host_origin[0] = hostOrigin[0];
	operation.host_origin[1] = hostOrigin[1];
	operation.host_origin[2] = hostOrigin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.buffer_row_pitch = bufferRowPitch;
	operation.buffer_slice_pitch = bufferSlicePitch;
	operation.host_row_pitch = hostRowPitch;
	operation.host_slice_pitch = hostSlicePitch;
	operation.event_length = waitEventListSize;
	operation.data_type = WebCLInputChecker::fromArrayBufferTypeToDataType(hostPtr.View()->GetType());
	operation.data_size = WebCLInputChecker::fromArrayBufferSizeToListSize(hostPtr.View());
	// operation.need_event = !blockingRead && event;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER_RECT);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
		mContext->getOperationResultData(hostPtr.View()->BaseAddress(), hostPtr.View()->byteLength());
	}
}

void WebCLCommandQueue::enqueueReadBufferRect(
		WebCLBuffer* buffer,
		bool blockingRead,
		Vector<unsigned> bufferOrigin,
		Vector<unsigned> dstOrigin,
		Vector<unsigned> region,
		unsigned bufferRowPitch,
		unsigned bufferSlicePitch,
		HTMLCanvasElement* dstCanvas,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueReadBufferRect with HTMLCanvasElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	int waitEventListSize = 0;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingRead, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	if(bufferOrigin.size() != 3 || dstOrigin.size() != 3 || region.size() != 3) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	Vector<uint8_t> canvasData;
	size_t canvasSize = 0;
	if(!mHTMLUtil->extractDataFromCanvas(dstCanvas, canvasData, canvasSize, ec))
		return;

    Vector<size_t> bufferOriginCopy, dstOriginCopy, regionCopy;
    bufferOriginCopy.AppendVector(bufferOrigin);
    dstOriginCopy.AppendVector(dstOrigin);
    regionCopy.AppendVector(region);
    if (!WebCLInputChecker::isValidRegionForMemoryObject(dstOriginCopy, regionCopy, 0, 0, canvasSize)
    	|| !WebCLInputChecker::isValidRegionForMemoryObject(bufferOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, buffer->getSize())) {
        ec.ThrowDOMException(WebCLException::INVALID_VALUE, "Invalid Value.");
        return;
    }

	mContext->startHandling();

	WebCL_Operation_enqueueReadBufferRect operation = WebCL_Operation_enqueueReadBufferRect();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.buffer = (cl_point)buffer->getCLMem();
	operation.blocking = blockingRead;
#if defined(OS_ANDROID)
	memcpy(operation.buffer_origin, bufferOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.host_origin, dstOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.buffer_origin[0] = bufferOrigin[0];
	operation.buffer_origin[1] = bufferOrigin[1];
	operation.buffer_origin[2] = bufferOrigin[2];

	operation.host_origin[0] = dstOrigin[0];
	operation.host_origin[1] = dstOrigin[1];
	operation.host_origin[2] = dstOrigin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.buffer_row_pitch = bufferRowPitch;
	operation.buffer_slice_pitch = bufferSlicePitch;
	operation.host_row_pitch = 0;
	operation.host_slice_pitch = 0;
	operation.event_length = waitEventListSize;
	operation.data_type = HOST_PTR_DATA_TYPE::UNSIGNED_CHAR;
	operation.data_size = canvasSize;
	operation.need_event = !blockingRead && event;
//	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER_RECT);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
		mContext->getOperationResultData(canvasData.begin(), canvasSize);
	}
}

void WebCLCommandQueue::enqueueReadImage(
		WebCLImage* image,
		bool blockingRead,
		Vector<unsigned> origin,
		Vector<unsigned> region,
		unsigned hostRowPitch,
		NotShared<DOMArrayBufferView> hostPtr,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueReadImage";

	int waitEventListSize = 0;

	if(origin.size() != 2 || region.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleImageArgs(image, origin, region, ec)) {
		return;
	}

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingRead, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	if(!WebCLInputChecker::isValidRegionForHostPtr(region, hostRowPitch, image->getImageDescriptor(), hostPtr.View()->byteLength())
		|| !WebCLInputChecker::isValidRegionForImage(image->getImageDescriptor(), origin, region)
		|| !WebCLInputChecker::isValidDataSizeForDOMArrayBufferView(hostRowPitch, hostPtr.View())) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	origin.push_back(0);
	region.push_back(1);

	mContext->startHandling();

	WebCL_Operation_enqueueReadImage operation = WebCL_Operation_enqueueReadImage();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.image = (cl_point)image->getCLMem();
	operation.blocking = blockingRead;
#if defined(OS_ANDROID)
	memcpy(operation.origin, origin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.origin[0] = origin[0];
	operation.origin[1] = origin[1];
	operation.origin[2] = origin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.row_pitch = hostRowPitch;
	operation.slice_pitch = 0;
	operation.event_length = waitEventListSize;
	operation.data_type = WebCLInputChecker::fromArrayBufferTypeToDataType(hostPtr.View()->GetType());
	operation.data_size = WebCLInputChecker::fromArrayBufferSizeToListSize(hostPtr.View());
	// operation.need_event = !blockingRead && event;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_READ_IMAGE);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
		mContext->getOperationResultData(hostPtr.View()->BaseAddress(), hostPtr.View()->byteLength());
	}
}

void WebCLCommandQueue::enqueueReadImage(
		WebCLImage* image,
		bool blockingRead,
		Vector<unsigned> origin,
		Vector<unsigned> region,
		HTMLCanvasElement* dstCanvas,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueReadImage with HTMLCanvasElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	int waitEventListSize = 0;

	if(origin.size() != 2 || region.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleImageArgs(image, origin, region, ec)) {
		return;
	}

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingRead, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	Vector<uint8_t> canvasData;
	size_t canvasSize = 0;
	if(!mHTMLUtil->extractDataFromCanvas(dstCanvas, canvasData, canvasSize, ec))
		return;

	if(!WebCLInputChecker::isValidRegionForHostPtr(region, 0, image->getImageDescriptor(), canvasSize)) {
		ec.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
		return;
	}

	origin.push_back(0);
	region.push_back(1);

	mContext->startHandling();

	WebCL_Operation_enqueueReadImage operation = WebCL_Operation_enqueueReadImage();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.image = (cl_point)image->getCLMem();
	operation.blocking = blockingRead;
#if defined(OS_ANDROID)
	memcpy(operation.origin, origin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.origin[0] = origin[0];
	operation.origin[1] = origin[1];
	operation.origin[2] = origin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.row_pitch = 0;
	operation.slice_pitch = 0;
	operation.event_length = waitEventListSize;
	operation.data_type = HOST_PTR_DATA_TYPE::UNSIGNED_CHAR;
	operation.data_size = canvasSize;
	// operation.need_event = !blockingRead && event;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_READ_IMAGE);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
		mContext->getOperationResultData(canvasData.begin(), canvasSize);
	}
}

void WebCLCommandQueue::enqueueWriteBuffer(
		WebCLBuffer* buffer,
		bool blockingWrite,
		unsigned bufferOffset,
		unsigned numBytes,
		NotShared<DOMArrayBufferView> hostPtr,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueWriteBuffer";

	int waitEventListSize = 0;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	if(!WebCLInputChecker::isValidDataSizeForDOMArrayBufferView(numBytes,hostPtr.View())) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(hostPtr.View()->byteLength() < numBytes || buffer->getSize() < bufferOffset+numBytes) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	mContext->startHandling();

	WebCL_Operation_enqueueWriteBuffer operation = WebCL_Operation_enqueueWriteBuffer();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.buffer = (cl_point)buffer->getCLMem();
	operation.blocking = blockingWrite;
	operation.buffer_offset = bufferOffset;
	operation.num_bytes = numBytes;
	operation.event_length = waitEventListSize;
	operation.need_event = !blockingWrite && event;
	operation.data_type = WebCLInputChecker::fromArrayBufferTypeToDataType(hostPtr.View()->GetType());
	operation.data_size = WebCLInputChecker::fromArrayBufferSizeToListSize(hostPtr.View());
	mContext->setOperationParameter(&operation);

	mContext->setOperationData(hostPtr.View()->BaseAddress(), hostPtr.View()->byteLength());

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueWriteBufferCommonForExtension(
		WebCLBuffer* buffer,
		bool blockingWrite,
		unsigned bufferOffset,
		Vector<unsigned char> rawData,
		size_t rawDataSize,
		cl_point* eventList,
		size_t eventListSize,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteBufferCommonForExtension";

	mContext->startHandling();

	WebCL_Operation_enqueueWriteBuffer operation = WebCL_Operation_enqueueWriteBuffer();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.buffer = (cl_point)buffer->getCLMem();
	operation.blocking = blockingWrite;
	operation.buffer_offset = bufferOffset;
	operation.num_bytes = rawDataSize;
	operation.event_length = eventListSize;
	operation.need_event = !blockingWrite && event;
	operation.data_type = HOST_PTR_DATA_TYPE::UNSIGNED_CHAR;
	operation.data_size = rawDataSize;
	mContext->setOperationParameter(&operation);

	mContext->setOperationData(rawData.data(), rawDataSize);

	if(eventListSize)
		mContext->setOperationEvents(eventList, eventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueWriteBuffer(
		WebCLBuffer* buffer,
		bool blockingWrite,
		unsigned bufferOffset,
		ImageData* srcPixels,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteBuffer with ImageData";

	int waitEventListSize = 0;

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	void* rawImageData;
	size_t imageDataSize = 0;

	if(!mHTMLUtil->extractDataFromImageData(srcPixels, rawImageData, imageDataSize, ec))
		return;

	Vector<uint8_t> imageData;
	imageData.resize(imageDataSize);
	unsigned char* rawTypedData = (unsigned char*)rawImageData;
	std::copy(rawTypedData, rawTypedData+imageDataSize, imageData.begin());

	if(buffer->getSize() < bufferOffset+imageDataSize) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	enqueueWriteBufferCommonForExtension(buffer, blockingWrite, bufferOffset, imageData, imageDataSize, pointWaitEventListInter, (size_t)waitEventListSize, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(
		WebCLBuffer* buffer,
		bool blockingWrite,
		unsigned bufferOffset,
		HTMLCanvasElement* srcCanvas,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteBuffer with HTMLCanvasElement";

	int waitEventListSize = 0;

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	Vector<uint8_t> canvasData;
	size_t canvasSize = 0;
	if(!mHTMLUtil->extractDataFromCanvas(srcCanvas, canvasData, canvasSize, ec))
		return;

	if(buffer->getSize() < bufferOffset+canvasSize) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	enqueueWriteBufferCommonForExtension(buffer, blockingWrite, bufferOffset, canvasData, canvasSize, pointWaitEventListInter, (size_t)waitEventListSize, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(
		WebCLBuffer* buffer,
		bool blockingWrite,
		unsigned bufferOffset,
		HTMLImageElement* srcImage,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteBuffer with HTMLImageElement";

	int waitEventListSize = 0;

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	Vector<uint8_t> imageData;
	size_t imageSize = 0;
	if(!mHTMLUtil->extractDataFromImage(srcImage, imageData, imageSize, ec))
		return;

	if(buffer->getSize() < bufferOffset+imageSize) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	enqueueWriteBufferCommonForExtension(buffer, blockingWrite, bufferOffset, imageData, imageSize, pointWaitEventListInter, (size_t)waitEventListSize, event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRect(
		WebCLBuffer* buffer,
		bool blockingWrite,
		Vector<unsigned> bufferOrigin,
		Vector<unsigned> hostOrigin,
		Vector<unsigned> region,
		unsigned bufferRowPitch,
		unsigned bufferSlicePitch,
		unsigned hostRowPitch,
		unsigned hostSlicePitch,
		NotShared<DOMArrayBufferView> hostPtr,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueWriteBufferRect";

	int waitEventListSize = 0;

	if(bufferOrigin.size() != 3 || hostOrigin.size() != 3 || region.size() != 3) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

    Vector<size_t> bufferOriginCopy, hostOriginCopy, regionCopy;
    bufferOriginCopy.AppendVector(bufferOrigin);
    hostOriginCopy.AppendVector(hostOrigin);
    regionCopy.AppendVector(region);

    if (!WebCLInputChecker::isValidRegionForMemoryObject(hostOriginCopy, regionCopy, hostRowPitch, hostSlicePitch, hostPtr.View()->byteLength())
    	|| !WebCLInputChecker::isValidRegionForMemoryObject(bufferOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, buffer->getSize())) {
        ec.ThrowDOMException(WebCLException::INVALID_VALUE, "Invalid Value.");
        return;
    }

    mContext->startHandling();

	WebCL_Operation_enqueueWriteBufferRect operation = WebCL_Operation_enqueueWriteBufferRect();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.buffer = (cl_point)buffer->getCLMem();
	operation.blocking = blockingWrite;
#if defined(OS_ANDROID)
	memcpy(operation.buffer_origin, bufferOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.host_origin, hostOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.buffer_origin[0] = bufferOrigin[0];
	operation.buffer_origin[1] = bufferOrigin[1];
	operation.buffer_origin[2] = bufferOrigin[2];

	operation.host_origin[0] = hostOrigin[0];
	operation.host_origin[1] = hostOrigin[1];
	operation.host_origin[2] = hostOrigin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.buffer_row_pitch = bufferRowPitch;
	operation.buffer_slice_pitch = bufferSlicePitch;
	operation.host_row_pitch = hostRowPitch;
	operation.host_slice_pitch = hostSlicePitch;
	operation.event_length = waitEventListSize;
	operation.data_type = WebCLInputChecker::fromArrayBufferTypeToDataType(hostPtr.View()->GetType());
	operation.data_size = WebCLInputChecker::fromArrayBufferSizeToListSize(hostPtr.View());
	operation.need_event = !blockingWrite && event;
	mContext->setOperationParameter(&operation);

	mContext->setOperationData(hostPtr.View()->BaseAddress(), hostPtr.View()->byteLength());

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER_RECT);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueWriteBufferRectCommonForExtension(
		WebCLBuffer* buffer,
		bool blockingWrite,
		Vector<unsigned> bufferOrigin,
		Vector<unsigned> srcOrigin,
		Vector<unsigned> region,
		unsigned bufferRowPitch,
		unsigned bufferSlicePitch,
		Vector<unsigned char> rawData,
		size_t rawDataSize,
		cl_point* eventList,
		size_t eventListSize,
		WebCLEvent* event,
		ExceptionState& ec)
{
	mContext->startHandling();

	WebCL_Operation_enqueueWriteBufferRect operation = WebCL_Operation_enqueueWriteBufferRect();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.buffer = (cl_point)buffer->getCLMem();
	operation.blocking = blockingWrite;
#if defined(OS_ANDROID)
	memcpy(operation.buffer_origin, bufferOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.host_origin, srcOrigin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.buffer_origin[0] = bufferOrigin[0];
	operation.buffer_origin[1] = bufferOrigin[1];
	operation.buffer_origin[2] = bufferOrigin[2];

	operation.host_origin[0] = srcOrigin[0];
	operation.host_origin[1] = srcOrigin[1];
	operation.host_origin[2] = srcOrigin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.buffer_row_pitch = bufferRowPitch;
	operation.buffer_slice_pitch = bufferSlicePitch;
	operation.host_row_pitch = 0;
	operation.host_slice_pitch = 0;
	operation.event_length = eventListSize;
	operation.data_type = HOST_PTR_DATA_TYPE::UNSIGNED_CHAR;
	operation.data_size = rawDataSize;
	operation.need_event = !blockingWrite && event;
	mContext->setOperationParameter(&operation);

	mContext->setOperationData(rawData.data(), rawDataSize);

	if(eventListSize)
		mContext->setOperationEvents(eventList, eventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER_RECT);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueWriteBufferRect(
		WebCLBuffer* buffer,
		bool blockingWrite,
		Vector<unsigned> bufferOrigin,
		Vector<unsigned> srcOrigin,
		Vector<unsigned> region,
		unsigned bufferRowPitch,
		unsigned bufferSlicePitch,
		ImageData* srcPixels,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteBufferRect with ImageData";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	int waitEventListSize = 0;

	if(bufferOrigin.size() != 3 || srcOrigin.size() != 3 || region.size() != 3) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

    Vector<size_t> bufferOriginCopy, srcOriginCopy, regionCopy;
    bufferOriginCopy.AppendVector(bufferOrigin);
    srcOriginCopy.AppendVector(srcOrigin);
    regionCopy.AppendVector(region);

    void* rawImageData;
	size_t imageDataSize = 0;

	if(!mHTMLUtil->extractDataFromImageData(srcPixels, rawImageData, imageDataSize, ec))
		return;

	Vector<uint8_t> imageData;
	imageData.resize(imageDataSize);
	unsigned char* rawTypedData = (unsigned char*)rawImageData;
	std::copy(rawTypedData, rawTypedData+imageDataSize, imageData.begin());

    if (!WebCLInputChecker::isValidRegionForMemoryObject(srcOriginCopy, regionCopy, 0, 0, imageDataSize)
    	|| !WebCLInputChecker::isValidRegionForMemoryObject(bufferOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, buffer->getSize())) {
        ec.ThrowDOMException(WebCLException::INVALID_VALUE, "Invalid Value.");
        return;
    }

    enqueueWriteBufferRectCommonForExtension(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, imageData, imageDataSize, pointWaitEventListInter, (size_t)waitEventListSize, event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRect(
		WebCLBuffer* buffer,
		bool blockingWrite,
		Vector<unsigned> bufferOrigin,
		Vector<unsigned> srcOrigin,
		Vector<unsigned> region,
		unsigned bufferRowPitch,
		unsigned bufferSlicePitch,
		HTMLCanvasElement* srcCanvas,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteBufferRect with HTMLCanvasElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	int waitEventListSize = 0;

	if(bufferOrigin.size() != 3 || srcOrigin.size() != 3 || region.size() != 3) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

    Vector<size_t> bufferOriginCopy, srcOriginCopy, regionCopy;
    bufferOriginCopy.AppendVector(bufferOrigin);
    srcOriginCopy.AppendVector(srcOrigin);
    regionCopy.AppendVector(region);

	Vector<uint8_t> canvasData;
	size_t canvasSize = 0;
	if(!mHTMLUtil->extractDataFromCanvas(srcCanvas, canvasData, canvasSize, ec))
		return;

    if (!WebCLInputChecker::isValidRegionForMemoryObject(srcOriginCopy, regionCopy, 0, 0, canvasSize)
    	|| !WebCLInputChecker::isValidRegionForMemoryObject(bufferOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, buffer->getSize())) {
        ec.ThrowDOMException(WebCLException::INVALID_VALUE, "Invalid Value.");
        return;
    }

    enqueueWriteBufferRectCommonForExtension(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, canvasData, canvasSize, pointWaitEventListInter, (size_t)waitEventListSize, event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRect(
		WebCLBuffer* buffer,
		bool blockingWrite,
		Vector<unsigned> bufferOrigin,
		Vector<unsigned> srcOrigin,
		Vector<unsigned> region,
		unsigned bufferRowPitch,
		unsigned bufferSlicePitch,
		HTMLImageElement* srcImage,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteBufferRect with HTMLImageElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	int waitEventListSize = 0;

	if(bufferOrigin.size() != 3 || srcOrigin.size() != 3 || region.size() != 3) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleMemArgs(buffer, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

    Vector<size_t> bufferOriginCopy, srcOriginCopy, regionCopy;
    bufferOriginCopy.AppendVector(bufferOrigin);
    srcOriginCopy.AppendVector(srcOrigin);
    regionCopy.AppendVector(region);

	Vector<uint8_t> imageData;
	size_t imageSize = 0;
	if(!mHTMLUtil->extractDataFromImage(srcImage, imageData, imageSize, ec))
		return;

    if (!WebCLInputChecker::isValidRegionForMemoryObject(srcOriginCopy, regionCopy, 0, 0, imageSize)
    	|| !WebCLInputChecker::isValidRegionForMemoryObject(bufferOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, buffer->getSize())) {
        ec.ThrowDOMException(WebCLException::INVALID_VALUE, "Invalid Value.");
        return;
    }

    enqueueWriteBufferRectCommonForExtension(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, imageData, imageSize, pointWaitEventListInter, (size_t)waitEventListSize, event, ec);
}

void WebCLCommandQueue::enqueueWriteImage(
		WebCLImage* image,
		bool blockingWrite,
		Vector<unsigned> origin,
		Vector<unsigned> region,
		unsigned hostRowPitch,
		DOMArrayBufferView* hostPtr,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueWriteImage";

	int waitEventListSize = 0;

	if(origin.size() != 2 || region.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleImageArgs(image, origin, region, ec)) {
		return;
	}

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	if(!WebCLInputChecker::isValidRegionForHostPtr(region, hostRowPitch, image->getImageDescriptor(), hostPtr->byteLength())
		|| !WebCLInputChecker::isValidRegionForImage(image->getImageDescriptor(), origin, region)
		|| !WebCLInputChecker::isValidDataSizeForDOMArrayBufferView(hostRowPitch, hostPtr)) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	origin.push_back(0);
	region.push_back(1);

	mContext->startHandling();

	WebCL_Operation_enqueueWriteImage operation = WebCL_Operation_enqueueWriteImage();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.image = (cl_point)image->getCLMem();
	operation.blocking = blockingWrite;
#if defined(OS_ANDROID)
	memcpy(operation.origin, origin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.origin[0] = origin[0];
	operation.origin[1] = origin[1];
	operation.origin[2] = origin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.input_row_pitch = hostRowPitch;
	operation.input_slice_pitch = 0;
	operation.event_length = waitEventListSize;
	operation.data_type = WebCLInputChecker::fromArrayBufferTypeToDataType(hostPtr->GetType());
	operation.data_size = WebCLInputChecker::fromArrayBufferSizeToListSize(hostPtr);
	operation.need_event = !blockingWrite && event;
	mContext->setOperationParameter(&operation);

	mContext->setOperationData(hostPtr->BaseAddress(), hostPtr->byteLength());

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_IMAGE);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueWriteImage(
		WebCLImage* image,
		bool blockingWrite,
		HTMLVideoElement* srcVideo,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueWriteImage, with HTMLVideoElement";

	int waitEventListSize = 0;

	if(!isEnabledExtension("WEBCL_html_video")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	Vector<unsigned> origin;
	origin.ReserveCapacity(2);
	origin.push_back(0);
	origin.push_back(0);

	Vector<unsigned> region;
	region.ReserveCapacity(2);
	region.push_back(image->getImageDescriptor().width());
	region.push_back(image->getImageDescriptor().height());

	if(!handleImageArgs(image, origin, region, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	Vector<uint8_t> videoData;
	size_t videoSize = 0;
	if(!mHTMLUtil->extractDataFromVideo(srcVideo, videoData, videoSize, ec))
		return;

	origin.push_back(0);
	region.push_back(1);

	mContext->startHandling();

	WebCL_Operation_enqueueWriteImage operation = WebCL_Operation_enqueueWriteImage();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.image = (cl_point)image->getCLMem();
	operation.blocking = blockingWrite;
#if defined(OS_ANDROID)
	memcpy(operation.origin, origin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.origin[0] = origin[0];
	operation.origin[1] = origin[1];
	operation.origin[2] = origin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.input_row_pitch = 0;
	operation.input_slice_pitch = 0;
	operation.event_length = waitEventListSize;
	operation.data_type = HOST_PTR_DATA_TYPE::UNSIGNED_CHAR;
	operation.data_size = videoSize;
	operation.need_event = !blockingWrite && event;
	mContext->setOperationParameter(&operation);

	mContext->setOperationData(videoData.data(), videoSize);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_IMAGE);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueWriteImageCommonForExtension(
		WebCLImage* image,
		bool blockingWrite,
		Vector<unsigned> origin,
		Vector<unsigned> region,
		Vector<unsigned char> rawData,
 		size_t rawDataSize,
		cl_point* eventList,
		size_t eventListSize,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueWriteImageCommonForExtension";

	if(!WebCLInputChecker::isValidRegionForHostPtr(region, 0, image->getImageDescriptor(), rawDataSize)) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(event != nullptr) {
		if(event->getCLContext() == nullptr) {
				event->setCLContext(mClContext);
		}

		if(mNativeClContext != event->getCLContext()->getClContext()) {
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return;
		}
	}

	origin.push_back(0);
	region.push_back(1);

	mContext->startHandling();

	WebCL_Operation_enqueueWriteImage operation = WebCL_Operation_enqueueWriteImage();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.image = (cl_point)image->getCLMem();
	operation.blocking = blockingWrite;
#if defined(OS_ANDROID)
	memcpy(operation.origin, origin.data(), sizeof(unsigned)*3);
	memcpy(operation.region, region.data(), sizeof(unsigned)*3);
#elif defined(OS_LINUX)
	operation.origin[0] = origin[0];
	operation.origin[1] = origin[1];
	operation.origin[2] = origin[2];

	operation.region[0] = region[0];
	operation.region[1] = region[1];
	operation.region[2] = region[2];
#endif
	operation.input_row_pitch = 0;
	operation.input_slice_pitch = 0;
	operation.event_length = eventListSize;
	operation.data_type = HOST_PTR_DATA_TYPE::UNSIGNED_CHAR;
	operation.data_size = rawDataSize;
	operation.need_event = !blockingWrite && event;
	mContext->setOperationParameter(&operation);

	mContext->setOperationData(rawData.data(), rawDataSize);

	if(eventListSize)
		mContext->setOperationEvents(eventList, eventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_IMAGE);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}
}

void WebCLCommandQueue::enqueueWriteImage(
 		WebCLImage* image,
 		bool blockingWrite,
 		Vector<unsigned> origin,
 		Vector<unsigned> region,
		ImageData* srcPixels,
 		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
 		WebCLEvent* event,
 		ExceptionState& ec)
{
	int waitEventListSize = 0;

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	if(origin.size() != 2 || region.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(srcPixels->width() <= 0 || srcPixels->height() <= 0) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleImageArgs(image, origin, region, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

    void* rawImageData;
	size_t imageDataSize = 0;

	if(!mHTMLUtil->extractDataFromImageData(srcPixels, rawImageData, imageDataSize, ec))
		return;

	Vector<uint8_t> imageData;
	imageData.resize(imageDataSize);
	unsigned char* rawTypedData = (unsigned char*)rawImageData;
	std::copy(rawTypedData, rawTypedData+imageDataSize, imageData.begin());

	enqueueWriteImageCommonForExtension(image, blockingWrite, origin, region, imageData, imageDataSize, pointWaitEventListInter, waitEventListSize, event, ec);
}

 void WebCLCommandQueue::enqueueWriteImage(
 		WebCLImage* image,
 		bool blockingWrite,
 		Vector<unsigned> origin,
 		Vector<unsigned> region,
		HTMLCanvasElement* srcCanvas,
 		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
 		WebCLEvent* event,
 		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteImage with HTMLCanvasElement";

	int waitEventListSize = 0;

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	if(origin.size() != 2 || region.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(srcCanvas->width() <= 0 || srcCanvas->height() <= 0) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleImageArgs(image, origin, region, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	Vector<uint8_t> canvasData;
	size_t canvasSize = 0;
	if(!mHTMLUtil->extractDataFromCanvas(srcCanvas, canvasData, canvasSize, ec))
		return;

	enqueueWriteImageCommonForExtension(image, blockingWrite, origin, region, canvasData, canvasSize, pointWaitEventListInter, waitEventListSize, event, ec);
}

 void WebCLCommandQueue::enqueueWriteImage(
 		WebCLImage* image,
 		bool blockingWrite,
 		Vector<unsigned> origin,
 		Vector<unsigned> region,
		HTMLImageElement* srcImage,
 		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
 		WebCLEvent* event,
 		ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::enqueueWriteImage with HTMLImageElement";

	int waitEventListSize = 0;

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	if(origin.size() != 2 || region.size() != 2) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(srcImage->width() <= 0 || srcImage->height() <= 0) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleImageArgs(image, origin, region, ec))
		return;

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, blockingWrite, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	Vector<uint8_t> imageData;
	size_t imageSize = 0;
	if(!mHTMLUtil->extractDataFromImage(srcImage, imageData, imageSize, ec))
		return;

	enqueueWriteImageCommonForExtension(image, blockingWrite, origin, region, imageData, imageSize, pointWaitEventListInter, waitEventListSize, event, ec);
}

void WebCLCommandQueue::enqueueNDRangeKernel(
		WebCLKernel* kernel,
		unsigned workDim,
		Nullable<Vector<double>> globalWorkOffset,
		Vector<double> globalWorkSize,
		Nullable<Vector<double>> localWorkSize,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec)
{
	CLLOG(INFO) << "enqueueNDRangeKernel";

	int waitEventListSize = 0;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(!handleKernelArg(kernel, ec))
		return;

	if(workDim != 1 && workDim != 2 && workDim != 3) {
		ec.ThrowDOMException(WebCLException::INVALID_WORK_DIMENSION, "WebCLException::INVALID_WORK_DIMENSION");
		return;
	}

	const unsigned long long maxWorkSizeValue = (1ULL << 32) - 1;

	size_t* gWorkSizeList = nullptr;

	if(globalWorkSize.size() != workDim) {
		ec.ThrowDOMException(WebCLException::INVALID_GLOBAL_WORK_SIZE, "WebCLException::INVALID_GLOBAL_WORK_SIZE");
		return;
	}

	gWorkSizeList = new size_t[workDim];
	for(unsigned i=0; i<workDim; i++) {
		if(globalWorkSize[i] > maxWorkSizeValue) {
			ec.ThrowDOMException(WebCLException::INVALID_GLOBAL_WORK_SIZE, "WebCLException::INVALID_GLOBAL_WORK_SIZE");
			return;
		}
		gWorkSizeList[i] = (unsigned)globalWorkSize[i];
	}

	size_t* gWorkOffsetList = nullptr;
	if(!globalWorkOffset.IsNull() && globalWorkOffset.Get().size()) {
		Vector<double> gWorkOffset = globalWorkOffset.Get();

		if(gWorkOffset.size() != workDim) {
			ec.ThrowDOMException(WebCLException::INVALID_GLOBAL_OFFSET, "WebCLException::INVALID_GLOBAL_OFFSET");
			return;
		}

		gWorkOffsetList = new size_t[workDim];

		for(unsigned i=0; i<workDim; i++) {
			if(gWorkSizeList[i] + gWorkOffset[i] > maxWorkSizeValue) {
				ec.ThrowDOMException(WebCLException::INVALID_GLOBAL_OFFSET, "WebCLException::INVALID_GLOBAL_OFFSET");
				return;
			}
			gWorkOffsetList[i] = (unsigned)gWorkOffset[i];
		}
	}

	size_t* lWorkSizeList = nullptr;
	if(!localWorkSize.IsNull() && localWorkSize.Get().size()) {
		Vector<double> lWorkSize = localWorkSize.Get();

		if(lWorkSize.size() != workDim) {
			ec.ThrowDOMException(WebCLException::INVALID_WORK_GROUP_SIZE, "WebCLException::INVALID_WORK_GROUP_SIZE");
			return;
		}

		lWorkSizeList = new size_t[workDim];

		Vector<unsigned> deviceMaxWorkItemSizeVector = mRelatedDevice->getMaxWorkItemSizes();
		Vector<unsigned> kernelCompileWorkGroupSizeVector = kernel->getCompileWorkGroupSizeVector(mRelatedDevice.Get(), ec);

		if (ec.HadException()) {
			return;
		}

		unsigned totalWorkSize = 0;
		for(unsigned i=0; i<workDim; i++) {
			if((isnan(lWorkSize[i]) || !lWorkSize[i])
					|| gWorkSizeList[i] % (unsigned)lWorkSize[i] != 0
					|| (unsigned)lWorkSize[i] > deviceMaxWorkItemSizeVector[i]
					|| (kernelCompileWorkGroupSizeVector[i] && lWorkSize[i] != kernelCompileWorkGroupSizeVector[i])) {
				ec.ThrowDOMException(WebCLException::INVALID_WORK_GROUP_SIZE, "WebCLException::INVALID_WORK_GROUP_SIZE");
				return;
			}
			lWorkSizeList[i] = (unsigned)lWorkSize[i];
			totalWorkSize += lWorkSize[i];
		}

		if(totalWorkSize > mRelatedDevice->getMaxWorkGroupSize()) {
			ec.ThrowDOMException(WebCLException::INVALID_WORK_GROUP_SIZE, "WebCLException::INVALID_WORK_GROUP_SIZE");
			return;
		}
	}
	else {
		Vector<unsigned> kernelCompileWorkGroupSizeVector = kernel->getCompileWorkGroupSizeVector(mRelatedDevice.Get(), ec);

		if (ec.HadException()) {
			return;
		}

		for(unsigned i=0; i<workDim; i++) {
			if(kernelCompileWorkGroupSizeVector[i]) {
				ec.ThrowDOMException(WebCLException::INVALID_WORK_GROUP_SIZE, "WebCLException::INVALID_WORK_GROUP_SIZE");
				return;
			}
		}
	}

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	mContext->startHandling();

	WebCL_Operation_enqueueNDRangeKernel operation = WebCL_Operation_enqueueNDRangeKernel();
	operation.command_queue = (cl_point)mClCommandQueue;
	operation.kernel = (cl_point)kernel->getCLKernel();
	operation.work_dim = workDim;
	operation.global_work_offset = gWorkOffsetList?workDim:0;
	operation.global_work_size = gWorkSizeList?workDim:0;
	operation.local_work_size = lWorkSizeList?workDim:0;
	operation.need_event = event != nullptr;
	operation.event_length = waitEventListSize;
	mContext->setOperationParameter(&operation);

	size_t dataSize = operation.global_work_offset + operation.global_work_size + operation.local_work_size;
	size_t* dataList = new size_t[dataSize];

	if(gWorkOffsetList)
		memcpy(dataList, gWorkOffsetList, sizeof(size_t)*workDim);
	if(gWorkSizeList)
		memcpy(dataList+operation.global_work_offset, gWorkSizeList, sizeof(size_t)*workDim);
	if(lWorkSizeList)
		memcpy(dataList+operation.global_work_offset+operation.global_work_size, lWorkSizeList, sizeof(size_t)*workDim);

	mContext->setOperationData(dataList, sizeof(size_t)*dataSize);

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_NDRANGE_KERNEL);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec))
		handleCreatedEvent((cl_event)result.ret_event, event);

	DELETE_ARRAY(dataList)
}

void WebCLCommandQueue::enqueueMarker(WebCLEvent* event, ExceptionState& ec)
{
	cl_int errcode = WebCL::FAILURE;
	cl_event eventInter = nullptr;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(event != nullptr && event->getCLEvent() != nullptr) {
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return;
	}

	errcode = webcl_clEnqueueMarker(webcl_channel_, mClCommandQueue, &eventInter);

	if(handleNativeErrorCode(errcode, ec))
		handleCreatedEvent(eventInter, event);
}

void WebCLCommandQueue::enqueueBarrier(ExceptionState& ec)
{
	cl_int errcode = WebCL::FAILURE;

	errcode = webcl_clEnqueueBarrier(webcl_channel_, mClCommandQueue);

	handleNativeErrorCode(errcode, ec);
}

void WebCLCommandQueue::enqueueWaitForEvents(HeapVector<Member<WebCLEvent>> eventWaitList, ExceptionState& ec)
{
	cl_int errcode = WebCL::FAILURE;
	cl_event* waitEventListInter = nullptr;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(eventWaitList.size() == 0) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	size_t listSize = eventWaitList.size();
	waitEventListInter = new cl_event[listSize];

	for(size_t i=0; i<listSize; i++) {
		if(eventWaitList[i]->getCLEvent() == nullptr) {
			CLLOG(INFO) << "WebCLEvent is not valid.";
			ec.ThrowDOMException(WebCLException::INVALID_EVENT_WAIT_LIST, "WebCLException::INVALID_EVENT_WAIT_LIST");
			return;
		}

		WebCLEvent* savedEvent = mClContext->findCLEvent((cl_obj_key)eventWaitList[i]->getCLEvent());
		if(savedEvent == nullptr) {
			CLLOG(INFO) << "event is not created by WebCLContext by which this WebCLComnmandQueue is created.";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return;
		}
		waitEventListInter[i] = eventWaitList[i]->getCLEvent();
	}

	errcode = webcl_clEnqueueWaitForEvents(webcl_channel_, mClCommandQueue, eventWaitList.size(), waitEventListInter);

	handleNativeErrorCode(errcode, ec);


	DELETE_ARRAY(waitEventListInter)
}

void WebCLCommandQueue::finish(WebCLCallback* whenFinished, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::finish";

	cl_int errcode = WebCL::FAILURE;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	if(whenFinished) {
		if(mFinishEvent) {
			ec.ThrowDOMException(WebCLException::INVALID_COMMAND_QUEUE, "WebCLException::INVALID_COMMAND_QUEUE");
			return;
		}

		mFinishEvent = WebCLEvent::Create(mContext);

		cl_event finishEvent = nullptr;
		errcode = webcl_clEnqueueMarker(webcl_channel_, mClCommandQueue, &finishEvent);

		if(errcode != CL_SUCCESS || !finishEvent) {
			ec.ThrowDOMException(WebCLException::INVALID_COMMAND_QUEUE, "WebCLException::INVALID_COMMAND_QUEUE");
			return;
		}

		mFinishEvent->setCLEvent(finishEvent);
		mClContext->addCLEvent(mFinishEvent.Get());

		mFinishEvent->setCallback(WebCL::COMPLETE, whenFinished, ec);
	}
	else {
		mContext->startHandling();

		WebCL_Operation_finish operation = WebCL_Operation_finish();
		operation.command_queue = (cl_point)mClCommandQueue;
		mContext->setOperationParameter(&operation);

		mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::FINISH);

		WebCL_Result_finish result = WebCL_Result_finish();
		mContext->getOperationResult(&result);

		errcode = result.result;
	}

	handleNativeErrorCode(errcode, ec);
}

void WebCLCommandQueue::flush(ExceptionState& ec)
{
	if(!handleCmdQueueAndCLContext(ec))
		return;

	cl_int errcode = WebCL::FAILURE;

	errcode = webcl_clFlush(webcl_channel_, mClCommandQueue);

	handleNativeErrorCode(errcode, ec);
}

ScriptValue WebCLCommandQueue::getInfo(ScriptState* scriptState, int name, ExceptionState& ec)
{
	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();

	if(!handleCmdQueueAndCLContext(ec))
		return ScriptValue(scriptState, v8::Null(isolate));

	cl_int errcode = -1;

	if(mClCommandQueue == NULL) {
		CLLOG(INFO) << "cl_command_queue is null.";
		ec.ThrowDOMException(WebCLException::INVALID_COMMAND_QUEUE, "WebCLException::INVALID_COMMAND_QUEUE");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	if(!WebCLValueChecker::isCommandQueueInfoNameValid(name)) {
		CLLOG(INFO) << "param_name=" << name << "is not valid.";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	switch(name) {
	case WebCL::QUEUE_CONTEXT: {
			cl_context queueContext = NULL;
			errcode = webcl_clGetCommandQueueInfo(webcl_channel_, mClCommandQueue, CL_QUEUE_CONTEXT, sizeof(cl_context), &queueContext, NULL);
			if(errcode == CL_SUCCESS) {
				Persistent<WebCLContext> savedContext = mContext.Get()->findCLContext((cl_obj_key)queueContext);
				if(savedContext.Get() == NULL) {
					return ScriptValue(scriptState, v8::Null(isolate));
				}
				else {
					return ScriptValue(scriptState, ToV8(savedContext, creationContext, isolate));
				}
			}
		}
		break;
	case WebCL::QUEUE_DEVICE: {
			cl_device_id queueDevice = NULL;
			errcode = webcl_clGetCommandQueueInfo(webcl_channel_, mClCommandQueue, CL_QUEUE_DEVICE, sizeof(cl_context), &queueDevice, NULL);
			if(errcode == CL_SUCCESS) {
				Persistent<WebCLDevice> deviceInfo;
				Member<WebCLDevice> savedDevice = mContext.Get()->findCLDevice((cl_obj_key)queueDevice, ec);
				if(savedDevice.Get()->getCLDevice() == NULL) {
					deviceInfo = WebCLDevice::create(mContext.Get(), queueDevice);
					deviceInfo->setDefaultValue(ec);
					if (ec.HadException()) {
						deviceInfo = nullptr;
						return ScriptValue(scriptState, v8::Null(isolate));
					}
					return ScriptValue(scriptState, ToV8(deviceInfo, creationContext, isolate));
				}
				else {
					deviceInfo = savedDevice.Get();
					return ScriptValue(scriptState, ToV8(deviceInfo, creationContext, isolate));
				}
			}
		}
		break;
	case WebCL::QUEUE_PROPERTIES: {
			cl_command_queue_properties queueProperties;
			errcode = webcl_clGetCommandQueueInfo(webcl_channel_, mClCommandQueue, CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &queueProperties, NULL);
			if(errcode == CL_SUCCESS) {
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned int>(queueProperties)));
			}
		}
		break;
	}

	handleNativeErrorCode(errcode, ec);
	return ScriptValue(scriptState, v8::Null(isolate));
}

void WebCLCommandQueue::release(ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::release";

	if(mClCommandQueue == nullptr) {
		CLLOG(INFO) << "cl_comand_queue is not valid.";
		ec.ThrowDOMException(WebCLException::INVALID_COMMAND_QUEUE, "WebCLException::INVALID_COMMAND_QUEUE");
		return;
	}

	cl_int errcode = WebCL::FAILURE;

	mContext->startHandling();

	WebCL_Operation_finish operation = WebCL_Operation_finish();
	operation.command_queue = (cl_point)mClCommandQueue;
	mContext->setOperationParameter(&operation);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::FINISH);

	errcode = webcl_clReleaseCommon(webcl_channel_, OPENCL_OBJECT_TYPE::CL_COMMAND_QUEUE, (cl_point)mClCommandQueue);

	if(handleNativeErrorCode(errcode, ec)) {
		WebCLContext* clContext = mContext.Get()->findCLContext((cl_obj_key)mNativeClContext);
		if (clContext != NULL) {
			clContext->removeCLCommandQueue((cl_obj_key)mClCommandQueue);
		} else {
			CLLOG(INFO) << "clContext is NULL";
		}
		mClCommandQueue = nullptr;
	}
}

bool WebCLCommandQueue::handleNativeErrorCode(cl_int errcode, ExceptionState& ec)
{
	CLLOG(INFO) << "error code=" << errcode;

	if(errcode != CL_SUCCESS) {
		WebCLException::throwException(errcode, ec);
		return false;
	}

	return true;
}

bool WebCLCommandQueue::handleCmdQueueAndCLContext(ExceptionState& ec)
{
	if(mClCommandQueue == nullptr) {
		CLLOG(INFO) << "cl_comand_queue is not valid.";
		ec.ThrowDOMException(WebCLException::INVALID_COMMAND_QUEUE, "WebCLException::INVALID_COMMAND_QUEUE");
		return false;
	}

	if(mClContext.Get() == nullptr) {
		mClContext = mContext.Get()->findCLContext((cl_obj_key)mNativeClContext);

		if(mClContext.Get()->getClContext() == nullptr) {
			CLLOG(INFO) << "cl_context is not valid.";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return false;
		}

		mNativeClContext = mClContext.Get()->getClContext();
	}

	return true;
}

bool WebCLCommandQueue::handleMemArgs(WebCLMemoryObject* object1, ExceptionState& ec)
{
	if(object1 == nullptr || object1->getCLMem() == NULL) {
		CLLOG(INFO) << "memory object is not valid.";
		ec.ThrowDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
		return false;
	}

	if(mClContext->findCLMemObj((cl_obj_key)object1->getCLMem()) == nullptr) {
		CLLOG(INFO) << "memory object is created by another WebCLContext.";
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return false;
	}

	return true;
}

bool WebCLCommandQueue::handleMemArgs(WebCLMemoryObject* object1, WebCLMemoryObject* object2, ExceptionState& ec)
{
	if((object1 == nullptr || object1->getCLMem() == NULL)
			|| (object2 == nullptr || object2->getCLMem() == NULL)) {
		CLLOG(INFO) << "memory object is not valid.";
		ec.ThrowDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
		return false;
	}

	if(mClContext->findCLMemObj((cl_obj_key)object1->getCLMem()) == nullptr
			|| mClContext->findCLMemObj((cl_obj_key)object2->getCLMem()) == nullptr) {
		CLLOG(INFO) << "memory object is created by another WebCLContext.";
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return false;
	}

	return true;
}

bool WebCLCommandQueue::handleImageArgs(WebCLImage* image1, Vector<unsigned> img1Origin, Vector<unsigned> region, ExceptionState& ec)
{
	return handleImageArgs(image1, img1Origin, nullptr, Vector<unsigned>(), region, ec);
}

bool WebCLCommandQueue::handleImageArgs(WebCLImage* image1, Vector<unsigned> img1Origin, WebCLImage* image2, Vector<unsigned> img2Origin, Vector<unsigned> region, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::handleImageArgs";

	if(image1 == nullptr)
		return false;

	if(image1->getCLMem() == nullptr) {
		ec.ThrowDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
		return false;
	}

	WebCLImageDescriptor img1Descriptor;
	WebCLImageDescriptor img2Descriptor;

	img1Descriptor = image1->getImageDescriptor();
	if(!WebCLInputChecker::isValidRegionForImage(img1Descriptor, img1Origin, region)) {
		CLLOG(INFO) << "invalid region for img1";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return false;
	}

	if(image2 != nullptr) {
		if(image2->getCLMem() == nullptr) {
			ec.ThrowDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
			return false;
		}

		img2Descriptor = image2->getImageDescriptor();
		if(!WebCLInputChecker::isValidRegionForImage(img2Descriptor, img2Origin, region)) {
			CLLOG(INFO) << "invalid region for img2";
			ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			return false;
		}

		if((img1Descriptor.channelOrder() != img2Descriptor.channelOrder())
				|| (img1Descriptor.channelType() != img2Descriptor.channelType())) {
			CLLOG(INFO) << "two WebCLImage have diffrent image format.";
			ec.ThrowDOMException(WebCLException::IMAGE_FORMAT_MISMATCH, "WebCLException::IMAGE_FORMAT_MISMATCH");
			return false;
		}
	}

	return true;
}

bool WebCLCommandQueue::handleEventArg(WebCLEvent* event, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::handleEventArg";

	if(event != nullptr && event->getCLEvent() != nullptr) {
		CLLOG(INFO) << "event is not empty event.";
		ec.ThrowDOMException(WebCLException::INVALID_EVENT, "WebCLException::INVALID_EVENT");
		return false;
	}

	return true;
}

cl_point* WebCLCommandQueue::handleWaitEventListArgs(Nullable<HeapVector<Member<WebCLEvent>>> argEventNullable, int& listSize, ExceptionState& ec)
{
	return handleWaitEventListArgs(argEventNullable, listSize, false, ec);
}

cl_point* WebCLCommandQueue::handleWaitEventListArgs(Nullable<HeapVector<Member<WebCLEvent>>> argEventNullable, int& listSize, bool blockingMode, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::handleWaitEventListArgs";

	if(argEventNullable.IsNull())
		return nullptr;

	HeapVector<Member<WebCLEvent>> argEventList = argEventNullable.Get();

	cl_point* nativeEventList = nullptr;
	if(argEventList.size() > 0) {
		WebCLContext* clContext = mClContext.Get();

		listSize = argEventList.size();
		nativeEventList = new cl_point[listSize];

		for(int i=0; i<listSize; i++) {
			if(argEventList[i].Get()->getCLEvent() == nullptr) {
				CLLOG(INFO) << "WebCLEvent is not valid.";
				ec.ThrowDOMException(WebCLException::INVALID_EVENT_WAIT_LIST, "WebCLException::INVALID_EVENT_WAIT_LIST");
				return nullptr;
			}

			WebCLEvent* savedEvent = clContext->findCLEvent((cl_obj_key)argEventList[i].Get()->getCLEvent());
			if(savedEvent == nullptr) {
				CLLOG(INFO) << "event is not created by WebCLContext by which this WebCLComnmandQueue is created.";
				ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
				return nullptr;
			}

			if(blockingMode &&
					(argEventList[i].Get()->getEventCommandExecutionStatus(ec) < 0 || argEventList[i].Get()->isUserEvent())) {
				CLLOG(INFO) << "event execution status is negative value although blocking mode set to true.";
				ec.ThrowDOMException(WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST, "WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST");
				return nullptr;
			}

			nativeEventList[i] = (cl_point)argEventList[i].Get()->getCLEvent();
		}
	}
	else {
		listSize = 0;
	}

	return nativeEventList;
}

bool WebCLCommandQueue::handleCreatedEvent(cl_event createdEvent, WebCLEvent* event) {
	if(createdEvent != nullptr && event != nullptr) {
		CLLOG(INFO) << ">> set cl_event=" << createdEvent;
		event->setCLEvent(createdEvent);
		mClContext->addCLEvent(event);
	}
	return true;
}

bool WebCLCommandQueue::handleSizeOfBuffer(WebCLBuffer* buffer, size_t offset, size_t size, ExceptionState& ec)
{
	size_t bufferSize = buffer->getSize();

	if(offset + size > bufferSize) {
		CLLOG(INFO) << "out of buffer size.";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return false;
	}

	return true;
}

bool WebCLCommandQueue::handleKernelArg(WebCLKernel* kernel, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLCommandQueue::handleKernelArg";

	if(kernel == nullptr || kernel->getCLKernel() == nullptr) {
		CLLOG(INFO) << "inut kernel is not valid.";
		ec.ThrowDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return false;
	}

	size_t kernelNumArgs = kernel->getKernelNumArgs(ec);

	if(kernelNumArgs != kernel->associatedArgsNum()) {
		CLLOG(INFO) << "inut kernel is not valid.";
		ec.ThrowDOMException(WebCLException::INVALID_KERNEL_ARGS, "WebCLException::INVALID_KERNEL_ARGS");
		return false;
	}

	if(!mClContext->findCLKernel((cl_obj_key)kernel->getCLKernel())) {
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return false;
	}

	if(mRelatedDevice.Get() == nullptr) {
		mRelatedDevice = getQueueDevice(ec);
		if (ec.HadException()) {
			return false;
		}
	}

	return true;
}

bool WebCLCommandQueue::isEnabledExtension(WTF::String extensionName)
{
	if(mEnabledExtensionList.size() <= 0)
		return false;

	return mEnabledExtensionList.Contains(extensionName);
}

WebCLDevice* WebCLCommandQueue::getQueueDevice(ExceptionState& ec) {

	if(!handleCmdQueueAndCLContext(ec))
		return nullptr;

	cl_int errcode = -1;

	if(mClCommandQueue == NULL) {
		CLLOG(INFO) << "cl_command_queue is null.";
		ec.ThrowDOMException(WebCLException::INVALID_COMMAND_QUEUE, "WebCLException::INVALID_COMMAND_QUEUE");
		return nullptr;
	}

	cl_device_id queueDevice = NULL;
	errcode = webcl_clGetCommandQueueInfo(webcl_channel_, mClCommandQueue, CL_QUEUE_DEVICE, sizeof(cl_context), &queueDevice, NULL);
	if(errcode == CL_SUCCESS) {
		Persistent<WebCLDevice> deviceInfo;
		Member<WebCLDevice> savedDevice = mContext.Get()->findCLDevice((cl_obj_key)queueDevice, ec);
		if(savedDevice.Get()->getCLDevice() == NULL) {
			deviceInfo = WebCLDevice::create(mContext.Get(), queueDevice);
			deviceInfo->setDefaultValue(ec);
			if (ec.HadException()) {
				deviceInfo = nullptr;
				return nullptr;
			}
		}
		else {
			deviceInfo = savedDevice.Get();
		}
		return deviceInfo.Get();
	}

	handleNativeErrorCode(errcode, ec);
	return nullptr;
}

unsigned int WebCLCommandQueue::getQueueProperties(ExceptionState& ec){

	if(!handleCmdQueueAndCLContext(ec))
		return 0;

	cl_int errcode = -1;

	if(mClCommandQueue == NULL) {
		CLLOG(INFO) << "cl_command_queue is null.";
		ec.ThrowDOMException(WebCLException::INVALID_COMMAND_QUEUE, "WebCLException::INVALID_COMMAND_QUEUE");
		return 0;
	}

	cl_command_queue_properties queueProperties;
	errcode = webcl_clGetCommandQueueInfo(webcl_channel_, mClCommandQueue, CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &queueProperties, NULL);
	if(errcode == CL_SUCCESS) {
		return static_cast<unsigned int>(queueProperties);
	}

	handleNativeErrorCode(errcode, ec);
	return 0;
}

void WebCLCommandQueue::enqueueAcquireGLObjects(
		Vector<ScriptValue>  value,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec) {
	if (!mContext->isEnableExtension("cl_khr_gl_sharing")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}
	int waitEventListSize = 0;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	cl_mem* memObjects = new cl_mem[value.size()];
	for(unsigned i = 0; i < value.size(); i++) {
		v8::Isolate* isolate = value[i].GetIsolate();
		v8::Handle<v8::Value> object(value[i].V8Value());
		if (V8WebCLMemoryObject::hasInstance(object, isolate)) {
	 		memObjects[i] = (V8WebCLMemoryObject::toImplWithTypeCheck(isolate, object))->getCLMem();
		} else {
			ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			delete[] memObjects;
			return;
		}
	}

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	mContext->startHandling();

	WebCL_Operation_enqueueCommonGLObjects operation = WebCL_Operation_enqueueCommonGLObjects();
	operation.operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_ACQUIRE_GLOBJECTS;
	operation.commandQueue = (cl_point)mClCommandQueue;
	operation.numObjects = value.size();
	operation.numEventInWaitList = waitEventListSize;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);
	mContext->setOperationData(memObjects, sizeof(cl_mem)*value.size());

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_ACQUIRE_GLOBJECTS);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}

	delete[] memObjects;
}

void WebCLCommandQueue::enqueueReleaseGLObjects(
		Vector<ScriptValue>  value,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec) {
	if (!mContext->isEnableExtension("cl_khr_gl_sharing")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return;
	}

	int waitEventListSize = 0;

	if(!handleCmdQueueAndCLContext(ec))
		return;

	cl_mem* memObjects = new cl_mem[value.size()];
	for(unsigned i = 0; i < value.size(); i++) {
		v8::Isolate* isolate = value[i].GetIsolate();
		v8::Handle<v8::Value> object(value[i].V8Value());
		if (V8WebCLMemoryObject::hasInstance(object, isolate)) {
	 		memObjects[i] = (V8WebCLMemoryObject::toImplWithTypeCheck(isolate, object))->getCLMem();
		} else {
			ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			delete[] memObjects;
			return;
		}
	}

	cl_point* pointWaitEventListInter = nullptr;
	if((pointWaitEventListInter = handleWaitEventListArgs(eventWaitList, waitEventListSize, ec)) == nullptr && ec.HadException())
		return;

	if(!handleEventArg(event, ec))
		return;

	mContext->startHandling();

	WebCL_Operation_enqueueCommonGLObjects operation = WebCL_Operation_enqueueCommonGLObjects();
	operation.operation_type = ENQUEUE_RELEASE_GLOBJECTS;
	operation.commandQueue = (cl_point)mClCommandQueue;
	operation.numObjects = value.size();
	operation.numEventInWaitList = waitEventListSize;
	operation.need_event = event != nullptr;
	mContext->setOperationParameter(&operation);
	mContext->setOperationData(memObjects, sizeof(cl_mem)*value.size());

	if(waitEventListSize)
		mContext->setOperationEvents(pointWaitEventListInter, waitEventListSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::ENQUEUE_RELEASE_GLOBJECTS);

	WebCL_Result_enqueueOperationBase result = WebCL_Result_enqueueOperationBase();
	mContext->getOperationResult(&result);

	if(handleNativeErrorCode(result.result, ec)) {
		handleCreatedEvent((cl_event)result.ret_event, event);
	}

	delete[] memObjects;
}

DEFINE_TRACE(WebCLCommandQueue) {
	visitor->Trace(mContext);
	visitor->Trace(mClContext);
	visitor->Trace(mRelatedDevice);
	visitor->Trace(mFinishEvent);
}

} // namespace blink
