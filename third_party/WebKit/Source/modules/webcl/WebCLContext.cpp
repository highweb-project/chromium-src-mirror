// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "WebCLContext.h"

#include "core/html/HTMLVideoElement.h"
#include "core/html/HTMLCanvasElement.h"
#include "core/html/HTMLImageElement.h"
#include "core/html/ImageData.h"
#include "core/typed_arrays/DOMArrayBuffer.h"
#include "platform/bindings/V8Binding.h"

#include "WebCL.h"
#include "WebCLBuffer.h"
#include "WebCLSampler.h"
#include "WebCLImage.h"
#include "WebCLDevice.h"
#include "WebCLValueChecker.h"
#include "modules/webcl/WebCLImageDescriptor.h"
#include "WebCLUserEvent.h"
#include "WebCLCommandQueue.h"
#include "WebCLMemoryObject.h"
#include "WebCLProgram.h"
#include "WebCLInputChecker.h"
#include "WebCLKernel.h"
#include "WebCLAPIBlocker.h"
#include "WebCLMemoryUtil.h"
#include "modules/webgl/WebGLRenderingContext.h"
#include "core/dom/custom/WebCL/WebCLException.h"

// gl/cl sharing
#include "Source/modules/webgl/WebGLBuffer.h"
#include "Source/modules/webgl/WebGLRenderbuffer.h"
#include "Source/modules/webgl/WebGLTexture.h"
#include "Source/modules/webgl/WebGLSharedPlatform3DObject.h"

namespace blink {

WebCLContext::WebCLContext(WebCL* context, cl_context cl_context)
	: mContext(context),
	  mClContext(cl_context),
	  mNumDevices(0),
	  mImage2dMaxWidth(0),
	  mImage2dMaxHeight(0),
	  mMaxMemAllocSize(0)
{
	mDevices.resize(0);

	mCallbackHandler = WebCLCallbackHandler::create(this);
	mCallbackHandler->bindToChannel();

	mHTMLUtil = new WebCLHTMLUtil();

	mMemoryUtil = new WebCLMemoryUtil(mContext, this);

	isReleasing = false;
	mGLContext = nullptr;
}

WebCLContext::~WebCLContext()
{
}

WebCLBuffer* WebCLContext::createBuffer(int memFlags, unsigned sizeInBytes, ExceptionState& ec)
{
	WebCLBuffer* webclBuffer = createBuffer(memFlags, sizeInBytes, NotShared<DOMArrayBufferView>(), ec);
	return webclBuffer;
}

WebCLBuffer* WebCLContext::createBuffer(int memFlags, unsigned sizeInBytes, NotShared<DOMArrayBufferView> hostPtr, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createBuffer";

	if (mClContext == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return NULL;
	}

	if(sizeInBytes == 0)
	{
		CLLOG(INFO) << "Error: Invalid CL buffer size";
		ec.ThrowDOMException(WebCLException::INVALID_BUFFER_SIZE, "WebCLException::INVALID_BUFFER_SIZE");
		return NULL;
	}

	if(mDevices.size() == 0) {
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return NULL;
	}
	for(size_t i=0; i<mDevices.size(); i++) {
		unsigned long maxMemAllocSize = mDevices[i]->getDeviceMaxMemAllocSize();

		if(maxMemAllocSize < (unsigned long)sizeInBytes) {
			CLLOG(INFO) << "Error: Invalid CL buffer size";
			ec.ThrowDOMException(WebCLException::INVALID_BUFFER_SIZE, "WebCLException::INVALID_BUFFER_SIZE");
			return NULL;
		}
	}

	if(hostPtr && hostPtr.View()->byteLength() < sizeInBytes)
	{
		CLLOG(INFO) << "Error: Invalid CL hostPtr";
		ec.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
		return NULL;
	}

	if(hostPtr && !hostPtr.View()->buffer())
	{
		CLLOG(INFO) << "Error: Invalid CL hostPtr";
		ec.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
		return NULL;
	}

	cl_mem_flags memFlagsNative = 0;
	switch(memFlags) {
	case WebCL::MEM_READ_ONLY:
		memFlagsNative |= CL_MEM_READ_ONLY;
		break;
	case WebCL::MEM_WRITE_ONLY:
		memFlagsNative |= CL_MEM_WRITE_ONLY;
		break;
	case WebCL::MEM_READ_WRITE:
		memFlagsNative |= CL_MEM_READ_WRITE;
		break;
	default:
		CLLOG(INFO) << "Error: Unsupported Mem Flsg";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	if(hostPtr)
		memFlagsNative |= WebCL::MEM_COPY_HOST_PTR;

	mContext->startHandling();

	WebCL_Operation_createBuffer operation = WebCL_Operation_createBuffer();
	operation.context = (cl_point)mClContext;
	operation.flags = memFlagsNative;
	operation.size = sizeInBytes;
	hostPtr ? operation.use_host_ptr = true : operation.use_host_ptr = false;
	mContext->setOperationParameter(&operation);

	if(hostPtr)
		mContext->setOperationData(hostPtr.View()->BaseAddress(), hostPtr.View()->byteLength());

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::CREATE_BUFFER);

	WebCL_Result_createBuffer result = WebCL_Result_createBuffer();
	mContext->getOperationResult(&result);

	//add MemObject address check
	if (result.buffer == (cl_point)NULL) {
		result.result = WebCLException::WebCLExceptionCode::MEM_OBJECT_ALLOCATION_FAILURE;
	}

	if(result.result == CL_SUCCESS) {
		WebCLBuffer* o = WebCLBuffer::create(mContext, this, (cl_mem)result.buffer, false);
		CLLOG(INFO) << "created buffer=" << (cl_mem)result.buffer;
		// WebCLMemObjMap::AddResult addResult = mMemObjMap.Set(result.buffer, (WebCLMemoryObject*)o);
		mMemObjMap.Set(result.buffer, (WebCLMemoryObject*)o);

		if(!setBufferInfo(o, ec)) {
			CLLOG(INFO) << "Error: Invaild Error Type";
			ec.ThrowDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");

			return NULL;
		}

		if(o && !hostPtr) {
			mMemoryUtil->bufferCreated(o, ec);
		}

		return o;
	}
	else {
		WebCLException::throwException(result.result, ec);
		return nullptr;
	}

}

WebCLMemoryObject* WebCLContext::createExtensionMemoryObjectBase(int memFlags, void* data, size_t dataSize, int objectType, ExceptionState& ec) {
	return createExtensionMemoryObjectBase(memFlags, data, dataSize, objectType, 0, 0, ec);
}

WebCLMemoryObject* WebCLContext::createExtensionMemoryObjectBase(int memFlags, void* data, size_t dataSize, int objectType, size_t width, size_t height, ExceptionState& ec)
{
	cl_int err = WebCL::FAILURE;
	cl_mem cl_mem_id = nullptr;

	cl_mem_flags memFlagsNative = 0;
	switch(memFlags) {
	case WebCL::MEM_READ_ONLY:
		memFlagsNative |= CL_MEM_READ_ONLY;
		break;
	case WebCL::MEM_WRITE_ONLY:
		memFlagsNative |= CL_MEM_WRITE_ONLY;
		break;
	case WebCL::MEM_READ_WRITE:
		memFlagsNative |= CL_MEM_READ_WRITE;
		break;
	default:
		CLLOG(INFO) << "Error: Unsupported Mem Flsg";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	memFlagsNative |= WebCL::MEM_COPY_HOST_PTR;

	switch(objectType) {
	case OPENCL_OBJECT_TYPE::CL_BUFFER: {
		mContext->startHandling();

		WebCL_Operation_createBuffer operation = WebCL_Operation_createBuffer();
		operation.context = (cl_point)mClContext;
		operation.flags = memFlagsNative;
		operation.size = dataSize;
		operation.use_host_ptr = true;
		mContext->setOperationParameter(&operation);

		mContext->setOperationData(data, dataSize);

		mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::CREATE_BUFFER);

		WebCL_Result_createBuffer result = WebCL_Result_createBuffer();
		mContext->getOperationResult(&result);

		err = result.result;
		cl_mem_id = (cl_mem)result.buffer;

		}
		break;
	case OPENCL_OBJECT_TYPE::CL_IMAGE: {
		mContext->startHandling();

		WebCL_Operation_createImage operation = WebCL_Operation_createImage();
		operation.context = (cl_point)mClContext;
		operation.flags = memFlagsNative;
		operation.channel_order = CL_RGBA;
		operation.channel_type = CL_UNORM_INT8;
		operation.image_width = width;
		operation.image_height = height;
		operation.image_row_pitch = 0;
		operation.use_host_ptr = true;
		mContext->setOperationParameter(&operation);

		mContext->setOperationData(data, dataSize);

		mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::CREATE_IMAGE);

		WebCL_Result_createImage result = WebCL_Result_createImage();
		mContext->getOperationResult(&result);

		err = result.result;
		cl_mem_id = (cl_mem)result.image;

		}
		break;
	}

	if (err != CL_SUCCESS) {
			CLLOG(INFO) << ">>err code = " << err;
			WebCLException::throwException(err, ec);
	} else {
		if(objectType == OPENCL_OBJECT_TYPE::CL_BUFFER) {
			WebCLBuffer* o = WebCLBuffer::create(mContext, this, cl_mem_id, false);
			// WebCLMemObjMap::AddResult result = mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)o);
			mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)o);

			if(!setBufferInfo(o, ec)) {
				CLLOG(INFO) << "Error: Invaild Error Type";
				ec.ThrowDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");

				return nullptr;
			}

			return o;
		}
		else if(objectType == OPENCL_OBJECT_TYPE::CL_IMAGE) {
			WebCLImage* image = WebCLImage::create(mContext.Get(), this, cl_mem_id, false);
			// WebCLMemObjMap::AddResult result = mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)image);
			mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)image);

			return image;
		}
	}

	return nullptr;
}

WebCLBuffer* WebCLContext::createBuffer(int memFlags, ImageData* srcPixels, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createBuffer with ImageData";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	if(memFlags != WebCL::MEM_READ_WRITE && memFlags != WebCL::MEM_READ_ONLY && memFlags != WebCL::MEM_WRITE_ONLY) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	if(!setMaxMemAllocSize(ec))
		return nullptr;

	void* rawImageData;
	size_t imageDataSize = 0;
	if(!mHTMLUtil->extractDataFromImageData(srcPixels, rawImageData, imageDataSize, ec))
		return nullptr;

	if(mMaxMemAllocSize < (unsigned long)imageDataSize) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return nullptr;
	}

	return (WebCLBuffer*)createExtensionMemoryObjectBase(memFlags, rawImageData, imageDataSize, OPENCL_OBJECT_TYPE::CL_BUFFER, ec);
}

WebCLBuffer* WebCLContext::createBuffer(int memFlags, HTMLCanvasElement* srcCanvas, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createBuffer with HTMLCanvasElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	if(memFlags != WebCL::MEM_READ_WRITE && memFlags != WebCL::MEM_READ_ONLY && memFlags != WebCL::MEM_WRITE_ONLY) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	if(!setMaxMemAllocSize(ec))
		return nullptr;

	Vector<uint8_t> canvasData;
	size_t canvasSize = 0;
	if(!mHTMLUtil->extractDataFromCanvas(srcCanvas, canvasData, canvasSize, ec))
		return nullptr;

	if(mMaxMemAllocSize < (unsigned long)canvasSize) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return nullptr;
	}

	return (WebCLBuffer*)createExtensionMemoryObjectBase(memFlags, canvasData.data(), canvasSize, OPENCL_OBJECT_TYPE::CL_BUFFER, ec);
}

WebCLBuffer* WebCLContext::createBuffer(int memFlags, HTMLImageElement* srcImage, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createBuffer with HTMLImageElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	if(memFlags != WebCL::MEM_READ_WRITE && memFlags != WebCL::MEM_READ_ONLY && memFlags != WebCL::MEM_WRITE_ONLY) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	if(!setMaxMemAllocSize(ec))
		return nullptr;

	Vector<uint8_t> imageData;
	size_t imageSize = 0;
	if(!mHTMLUtil->extractDataFromImage(srcImage, imageData, imageSize, ec))
		return nullptr;

	if(mMaxMemAllocSize < (unsigned long)(srcImage->width()*srcImage->height()*4)) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return nullptr;
	}

	return (WebCLBuffer*)createExtensionMemoryObjectBase(memFlags, imageData.data(), imageSize, OPENCL_OBJECT_TYPE::CL_BUFFER, ec);
}

WebCLImage* WebCLContext::createImage(int memFlags, WebCLImageDescriptor descriptor, ExceptionState& ec)
{
	return createImage(memFlags, descriptor, NotShared<DOMArrayBufferView>(), ec);
}

WebCLImage* WebCLContext::createImage(int memFlags, WebCLImageDescriptor descriptor, NotShared<DOMArrayBufferView> hostPtr, ExceptionState& ec)
{
	cl_int err = WebCL::FAILURE;
	cl_mem cl_mem_id = NULL;

	CLLOG(INFO) << "WebCLContext::createImage";
	CLLOG(INFO) << ">>channelOrder=" << descriptor.channelOrder() << ", channelType=" << descriptor.channelType() << ", width=" << descriptor.width() << ", height=" << descriptor.height();


	if (mClContext == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return NULL;
	}

	if(mImage2dMaxWidth == 0 && mImage2dMaxHeight == 0) {
		if(mDevices.size() == 0) {
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return NULL;
		}

		Member<WebCLDevice> device = mDevices[0];
		mImage2dMaxWidth = device->getImage2DMaxWidth();
		mImage2dMaxHeight = device->getImage2DMaxHeight();
	}

	unsigned width = descriptor.width();
	unsigned height = descriptor.height();
	unsigned rowPitch = descriptor.rowPitch();
	unsigned channelOrder = descriptor.channelOrder();
	unsigned channelType = descriptor.channelType();
	unsigned numberOfChannels = WebCLInputChecker::numberOfChannelsForChannelOrder(channelOrder);
	unsigned bytesPerChannel = WebCLInputChecker::bytesPerChannelType(channelType);

	if((height == 0 || (height > mImage2dMaxHeight))
			||(width == 0 || (width > mImage2dMaxWidth))) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return NULL;
	}

	if(!hostPtr && rowPitch != 0) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return NULL;
	}

	if(hostPtr && rowPitch > 0 && rowPitch < width * numberOfChannels * bytesPerChannel) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return NULL;
	}

	if(hostPtr &&
			((rowPitch && hostPtr.View()->byteLength() < rowPitch * height) || hostPtr.View()->byteLength() < (width*height*numberOfChannels*bytesPerChannel))
		) {
		ec.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
		return NULL;
	}

	if(!WebCLValueChecker::isImageChannelOrderValid(descriptor.channelOrder())
		|| !WebCLValueChecker::isImageChannelTypeValid(descriptor.channelType())) {
		CLLOG(INFO) << "WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR, order=" << descriptor.channelOrder() << ", type=" << descriptor.channelType();
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR, "WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR");
		return NULL;
	}

	cl_mem_flags memFlagsNative = 0;
	switch(memFlags) {
	case WebCL::MEM_READ_ONLY:
		memFlagsNative |= CL_MEM_READ_ONLY;
		break;
	case WebCL::MEM_WRITE_ONLY:
		memFlagsNative |= CL_MEM_WRITE_ONLY;
		break;
	case WebCL::MEM_READ_WRITE:
		memFlagsNative |= CL_MEM_READ_WRITE;
		break;
	default:
		CLLOG(INFO) << "Error: Unsupported Mem Flsg";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	memFlagsNative |= WebCL::MEM_COPY_HOST_PTR;

	mContext->startHandling();

	WebCL_Operation_createImage operation = WebCL_Operation_createImage();
	operation.context = (cl_point)mClContext;
	operation.flags = memFlagsNative;
	operation.channel_order = descriptor.channelOrder();
	operation.channel_type = descriptor.channelType();
	operation.image_width = width;
	operation.image_height = height;
	operation.image_row_pitch = 0;
	operation.use_host_ptr = true;
	mContext->setOperationParameter(&operation);

	if(hostPtr)
		mContext->setOperationData(hostPtr.View()->BaseAddress(), hostPtr.View()->byteLength());
	else {
		DOMArrayBuffer* buffer = DOMArrayBuffer::Create(width * height * numberOfChannels * bytesPerChannel, 1);
		mContext->setOperationData(buffer->Data(), buffer->ByteLength());
	}

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::CREATE_IMAGE);

	WebCL_Result_createImage result = WebCL_Result_createImage();
	mContext->getOperationResult(&result);

	err = result.result;
	cl_mem_id = (cl_mem)result.image;

	if(err == CL_SUCCESS) {
		WebCLImage* image = WebCLImage::create(mContext.Get(), this, cl_mem_id, false);
		// WebCLMemObjMap::AddResult result = mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)image);
		mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)image);

		return image;
	}
	else {
		CLLOG(INFO) << ">>err code = " << err;
		WebCLException::throwException(err, ec);
		return nullptr;
	}

}

WebCLImage* WebCLContext::createImage(int memFlags, HTMLVideoElement* srcVideo, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createImage with HTMLVideoElement";
	cl_int err = WebCL::FAILURE;
	cl_mem cl_mem_id = nullptr;

	if(!isEnabledExtension("WEBCL_html_video")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	if (mClContext == NULL) {
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return nullptr;
	}

	if(memFlags != WebCL::MEM_READ_ONLY) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	unsigned width = srcVideo->clientWidth();
	unsigned height = srcVideo->clientHeight();

	if(!setMaxImageSize(ec))
		return nullptr;

	if((height == 0 || (height > mImage2dMaxHeight))
			||(width == 0 || (width > mImage2dMaxWidth))) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return nullptr;
	}

	Vector<uint8_t> videoData;
	size_t videoSize = 0;
	if(!mHTMLUtil->extractDataFromVideo(srcVideo, videoData, videoSize, ec))
		return nullptr;

	cl_mem_flags memFlagsNative = 0;
	switch(memFlags) {
	case WebCL::MEM_READ_ONLY:
		memFlagsNative |= CL_MEM_READ_ONLY;
		break;
	case WebCL::MEM_WRITE_ONLY:
		memFlagsNative |= CL_MEM_WRITE_ONLY;
		break;
	case WebCL::MEM_READ_WRITE:
		memFlagsNative |= CL_MEM_READ_WRITE;
		break;
	default:
		CLLOG(INFO) << "Error: Unsupported Mem Flsg";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	memFlagsNative |= WebCL::MEM_COPY_HOST_PTR;

	mContext->startHandling();

	WebCL_Operation_createImage operation = WebCL_Operation_createImage();
	operation.context = (cl_point)mClContext;
	operation.flags = memFlagsNative;
	operation.channel_order = CL_RGBA;
	operation.channel_type = CL_UNORM_INT8;
	operation.image_width = width;
	operation.image_height = height;
	operation.image_row_pitch = 0;
	operation.use_host_ptr = true;
	mContext->setOperationParameter(&operation);

	mContext->setOperationData(videoData.data(), videoSize);

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::CREATE_IMAGE);

	WebCL_Result_createImage result = WebCL_Result_createImage();
	mContext->getOperationResult(&result);

	err = result.result;
	cl_mem_id = (cl_mem)result.image;

	if(err == CL_SUCCESS) {
		WebCLImage* image = WebCLImage::create(mContext.Get(), this, cl_mem_id, false);
		// WebCLMemObjMap::AddResult result = mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)image);
		mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)image);
		return image;
	}
	else {
		CLLOG(INFO) << ">>err code = " << err;
		WebCLException::throwException(err, ec);
		return nullptr;
	}

}

WebCLImage* WebCLContext::createImage(int memFlags, ImageData* srcPixels, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createImage with ImageData";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	if(memFlags != WebCL::MEM_READ_WRITE && memFlags != WebCL::MEM_READ_ONLY && memFlags != WebCL::MEM_WRITE_ONLY) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	if(!setMaxImageSize(ec))
		return nullptr;

	size_t width = (size_t)srcPixels->width();
	size_t height = (size_t)srcPixels->height();
	if(width > mImage2dMaxWidth || height > mImage2dMaxHeight) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return nullptr;
	}

	void* rawImageData;
	size_t imageDataSize = 0;
	if(!mHTMLUtil->extractDataFromImageData(srcPixels, rawImageData, imageDataSize, ec))
		return nullptr;

	return (WebCLImage*)createExtensionMemoryObjectBase(memFlags, rawImageData, imageDataSize, OPENCL_OBJECT_TYPE::CL_IMAGE, width, height, ec);
}

WebCLImage* WebCLContext::createImage(int memFlags, HTMLCanvasElement* srcCanvas, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createImage with HTMLCanvasElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	if(memFlags != WebCL::MEM_READ_WRITE && memFlags != WebCL::MEM_READ_ONLY && memFlags != WebCL::MEM_WRITE_ONLY) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	if(!setMaxImageSize(ec))
		return nullptr;

	size_t width = (size_t)srcCanvas->width();
	size_t height = (size_t)srcCanvas->height();
	if(width > mImage2dMaxWidth || height > mImage2dMaxHeight) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return nullptr;
	}

	Vector<uint8_t> canvasData;
	size_t canvasSize = 0;
	if(!mHTMLUtil->extractDataFromCanvas(srcCanvas, canvasData, canvasSize, ec))
		return nullptr;

	return (WebCLImage*)createExtensionMemoryObjectBase(memFlags, canvasData.data(), canvasSize, OPENCL_OBJECT_TYPE::CL_IMAGE, width, height, ec);
}

WebCLImage* WebCLContext::createImage(int memFlags, HTMLImageElement* srcImage, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createImage with HTMLImageElement";

	if(!isEnabledExtension("WEBCL_html_image")) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	if(memFlags != WebCL::MEM_READ_WRITE && memFlags != WebCL::MEM_READ_ONLY && memFlags != WebCL::MEM_WRITE_ONLY) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	if(!setMaxImageSize(ec))
		return nullptr;

	size_t width = (size_t)srcImage->width();
	size_t height = (size_t)srcImage->height();
	if(width > mImage2dMaxWidth || height > mImage2dMaxHeight) {
		ec.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, "WebCLException::INVALID_IMAGE_SIZE");
		return nullptr;
	}

	Vector<uint8_t> imageData;
	size_t imageSize = 0;
	if(!mHTMLUtil->extractDataFromImage(srcImage, imageData, imageSize, ec))
		return nullptr;

	return (WebCLImage*)createExtensionMemoryObjectBase(memFlags, imageData.data(), imageSize, OPENCL_OBJECT_TYPE::CL_IMAGE, width, height, ec);
}

WebCLSampler* WebCLContext::createSampler(CLboolean normalizedCoords, CLenum addressingMode, CLenum filterMode, ExceptionState& ec) {
	cl_sampler m_cl_sampler = NULL;

	if (mClContext == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return NULL;
	}

	if (WebCL::ADDRESS_CLAMP != addressingMode && WebCL::ADDRESS_CLAMP_TO_EDGE != addressingMode && WebCL::ADDRESS_REPEAT != addressingMode
		&& WebCL::ADDRESS_MIRRORED_REPEAT != addressingMode)
	{
		CLLOG(INFO) << "WebCLSampler::WebCLSampler : addressingMode not match";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
	}
	else if (WebCL::FILTER_NEAREST != filterMode && WebCL::FILTER_LINEAR != filterMode)
	{
		CLLOG(INFO) << "WebCLSampler::WebCLSampler : filterMode not match";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
	}
	else if (false == normalizedCoords && (WebCL::ADDRESS_REPEAT == addressingMode || WebCL::ADDRESS_MIRRORED_REPEAT == addressingMode))
	{
		CLLOG(INFO) << "WebCLSampler::WebCLSampler : normalizedCoords is false and addressingMode is ADDRESS_REPEAT or ADDRESS_MIRRORED_REPEAT";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
	}
	else
	{
		cl_int errcode_ret = 0;

		m_cl_sampler = webcl_clCreateSampler(webcl_channel_, mClContext, normalizedCoords, addressingMode, filterMode, &errcode_ret);

		if (NULL == m_cl_sampler)
		{
			CLLOG(INFO) << "WebCLSampler::WebCLSampler : m_cl_sampler is null";
			WebCLException::throwException(errcode_ret, ec);
		}
		else {
			WebCLSampler* o = WebCLSampler::create(mContext, m_cl_sampler);
			o->setCLContext(this);
			mSamplerMap.Set((cl_obj_key)m_cl_sampler, o);
			return o;
		}
	}
	return NULL;
}

WebCLUserEvent* WebCLContext::createUserEvent(ExceptionState& ec)
{
	cl_int err = -1;
	cl_event event = NULL;
	if (mClContext == NULL) {
			printf("Error: Invalid CL Context\n");
			ec.ThrowDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");
			return NULL;
	}
	event =  webcl_clCreateUserEvent(webcl_channel_, mClContext, &err);
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	} else {
		WebCLUserEvent* o = WebCLUserEvent::create(mContext, event);
		o->setIsUserEvent();
		o->setHandlerKey(mCallbackHandler->getHandlerId());
		o->setCLContext(this);
		// WebCLEventMap::AddResult result = mEventMap.Set((cl_obj_key)event, o);
		mEventMap.Set((cl_obj_key)event, o);
		return o;
	}
	return NULL;
}

WebCLCommandQueue* WebCLContext::createCommandQueue(ExceptionState& ec)
{
	return createCommandQueue(NULL, 0, ec);
}

WebCLCommandQueue* WebCLContext::createCommandQueue(WebCLDevice* device, ExceptionState& ec)
{
	return createCommandQueue(device, 0, ec);
}

WebCLCommandQueue* WebCLContext::createCommandQueue(WebCLDevice* device, int properties, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createCommandQueue, properties=" << properties;
#if defined(OS_ANDROID)
	if(device && device->getDeviceType() == WebCL::DEVICE_TYPE_CPU && mContext->isAPIBlocked(WEBCL_BLOCKED_API_ARG1(createCommandQueue, WebCLDevice))) {
		ec.ThrowDOMException(WebCLException::WEBCL_API_NOT_PERMITTED, "WebCLException::WEBCL_API_NOT_PERMITTED");
		return nullptr;
	}
#endif
	cl_int errcode = WebCL::FAILURE;
	cl_device_id deviceId = NULL;
	cl_command_queue commandQueue = NULL;

	if (mClContext == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return NULL;
	}

	if(device != NULL) {
		CLLOG(INFO) << "device=" << device->getCLDevice();

		bool isValidDevice = false;
		unsigned inputDeviceInfo = device->getDeviceType();
		cl_device_id inputDeviceId = device->getCLDevice();
		for(size_t i=0; i<mDevices.size(); i++) {
			unsigned originDeviceInfo = mDevices[i].Get()->getDeviceType();
			cl_device_id originDeviceId = mDevices[i].Get()->getCLDevice();
			if(originDeviceId == inputDeviceId
					|| originDeviceInfo == inputDeviceInfo) {
				isValidDevice = true;
				break;
			}
		}

		if(!isValidDevice) {
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return NULL;
		}
		else {
			deviceId = device->getCLDevice();
		}
	}
	else {
		deviceId = mDevices[0].Get()->getCLDevice();
	}

	if(properties!=0 && !WebCLValueChecker::isCommandQueuePropertyValid(properties)) {
		CLLOG(INFO) << "Error: Invalid CommandQueue properties, input propertie=" << properties;
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return NULL;
	}

	switch(properties) {
	case WebCL::QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
		commandQueue = webcl_clCreateCommandQueue(webcl_channel_, mClContext, deviceId, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &errcode);
		break;
	case WebCL::QUEUE_PROFILING_ENABLE:
		commandQueue = webcl_clCreateCommandQueue(webcl_channel_, mClContext, deviceId, CL_QUEUE_PROFILING_ENABLE, &errcode);
		break;
	default:
		commandQueue = webcl_clCreateCommandQueue(webcl_channel_, mClContext, deviceId, 0, &errcode);
		break;
	}

	if(errcode != CL_SUCCESS) {
		CLLOG(INFO) << "native error=" << errcode;
		WebCLException::throwException(errcode, ec);
	}
	else {
		WebCLCommandQueue* commandQueueObj =  WebCLCommandQueue::create(mContext.Get(), mClContext, commandQueue);
		commandQueueObj->setEnableExtensionList(mEnabledExtensionList);
		// WebCLCommandQueueMap::AddResult result = mCommandQueueMap.Set((cl_obj_key)commandQueue, commandQueueObj);
		mCommandQueueMap.Set((cl_obj_key)commandQueue, commandQueueObj);

		mMemoryUtil->commandQueueCreated(commandQueueObj, ec);

		return commandQueueObj;
	}

	return NULL;
}

ScriptValue WebCLContext::getInfo(ScriptState* scriptState, int name, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::getInfo, name=" << name;

	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();

	if (mClContext == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return ScriptValue(scriptState, v8::Null(isolate));
	}

	cl_int errcode = CL_INVALID_VALUE;

	switch(name) {
	case WebCL::CONTEXT_DEVICES: {
			if(mDevices.size()  != 0)
				return ScriptValue(scriptState, ToV8(mDevices, creationContext, isolate));

			if(mNumDevices <= 0) {
				cl_uint numDevices = 0;
				errcode = webcl_clGetContextInfo(webcl_channel_, mClContext, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &numDevices, NULL);
				if(errcode == CL_SUCCESS) {
					mNumDevices = numDevices;
				}
			}

			if(mNumDevices >0) {
				cl_device_id* devices = new cl_device_id[mNumDevices];
				errcode = webcl_clGetContextInfo(webcl_channel_, mClContext, CL_CONTEXT_DEVICES, sizeof(cl_device_id)*mNumDevices, devices, NULL);

				if(errcode == CL_SUCCESS) {
					for (unsigned int i = 0; i < mNumDevices; i++) {
						Member<WebCLDevice> o = mContext->findCLDevice((cl_obj_key)devices[i], ec);
						if (o == NULL) {
							o = WebCLDevice::create(mContext.Get(), devices[i]);
							o->setDefaultValue(ec);
							if (ec.HadException()) {
								continue;
							}
						}
						mDevices.push_back(o);
					}
					return ScriptValue(scriptState, ToV8(mDevices, creationContext, isolate));
				}
			}
		}
		break;
	case WebCL::CONTEXT_NUM_DEVICES: {
			cl_uint numDevices = 0;
			CLLOG(INFO) << "CONTEXT_NUM_DEVICES " << mNumDevices;
			if(mNumDevices <= 0) {
				errcode = webcl_clGetContextInfo(webcl_channel_, mClContext, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &numDevices, NULL);

				if(errcode == CL_SUCCESS) {
					mNumDevices = numDevices;
				}
			}
			return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(mNumDevices)));
		}
		break;
	default:
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	CLLOG(INFO) << ">>errcode=" << errcode;
	WebCLException::throwException(errcode, ec);

	return ScriptValue(scriptState, v8::Null(isolate));
}

HeapVector<WebCLImageDescriptor> WebCLContext::getSupportedImageFormats(ExceptionState& ec)
{
	return getSupportedImageFormats(WebCL::MEM_READ_WRITE, ec);
}

HeapVector<WebCLImageDescriptor> WebCLContext::getSupportedImageFormats(unsigned memFlags, ExceptionState& ec)
{
	HeapVector<WebCLImageDescriptor> descriptorList;

	if (mClContext == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return descriptorList;
	}

	cl_int errcode = CL_INVALID_VALUE;
	cl_image_format* imageFormats;
	cl_uint numImageFormats = 0;

	switch(memFlags) {
	case WebCL::MEM_READ_WRITE:
		errcode = webcl_clGetSupportedImageFormats(webcl_channel_, mClContext, memFlags, CL_MEM_OBJECT_IMAGE2D, 0, NULL, &numImageFormats);
		break;
	case WebCL::MEM_READ_ONLY:
		errcode = webcl_clGetSupportedImageFormats(webcl_channel_, mClContext, memFlags, CL_MEM_OBJECT_IMAGE2D, 0, NULL, &numImageFormats);
		break;
	case WebCL::MEM_WRITE_ONLY:
		errcode = webcl_clGetSupportedImageFormats(webcl_channel_, mClContext, memFlags, CL_MEM_OBJECT_IMAGE2D, 0, NULL, &numImageFormats);
		break;
	default:
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return descriptorList;
	}

	if(errcode != CL_SUCCESS) {
		WebCLException::throwException(errcode, ec);
	}

	imageFormats = new cl_image_format[numImageFormats];

	switch(memFlags) {
	case WebCL::MEM_READ_WRITE:
		errcode = webcl_clGetSupportedImageFormats(webcl_channel_, mClContext, memFlags, CL_MEM_OBJECT_IMAGE2D, numImageFormats, imageFormats, NULL);
		break;
	case WebCL::MEM_READ_ONLY:
		errcode = webcl_clGetSupportedImageFormats(webcl_channel_, mClContext, memFlags, CL_MEM_OBJECT_IMAGE2D, numImageFormats, imageFormats, NULL);
		break;
	case WebCL::MEM_WRITE_ONLY:
		errcode = webcl_clGetSupportedImageFormats(webcl_channel_, mClContext, memFlags, CL_MEM_OBJECT_IMAGE2D, numImageFormats, imageFormats, NULL);
		break;
	default:
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return descriptorList;
	}

	if(errcode == CL_SUCCESS) {
		WebCLImageDescriptor descriptor;
		for(unsigned i=0; i<numImageFormats; i++) {
			descriptor = WebCLImageDescriptor();
			descriptor.setChannelOrder(imageFormats[i].image_channel_order);
			descriptor.setChannelType(imageFormats[i].image_channel_data_type);

			descriptorList.push_back(descriptor);
		}
	}
	else {
		WebCLException::throwException(errcode, ec);
	}

	return descriptorList;
}

void WebCLContext::release(ExceptionState& ec)
{
	int errcode = CL_INVALID_VALUE;

	if (mClContext == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return;
	}

	errcode = webcl_clReleaseCommon(webcl_channel_, OPENCL_OBJECT_TYPE::CL_CONTEXT, (cl_point)mClContext);

	if(errcode != CL_SUCCESS) {
		WebCLException::throwException(errcode, ec);
	}
	else {
		if (mContext != NULL) {
			mContext.Get()->deleteCLContext((cl_obj_key)mClContext);
		}
		mClContext = NULL;
		mGLContext = nullptr;
	}
}

void WebCLContext::releaseAll(ExceptionState& ec)
{
	isReleasing = true;
	CLLOG(INFO) << "WebCLKernelMap size = " << mKernelMap.size();
	if (mKernelMap.size() > 0) {
		for(WebCLKernelMap::iterator kernel_iter = mKernelMap.begin(); kernel_iter != mKernelMap.end(); ++kernel_iter) {
			WebCLKernel* kernel = (WebCLKernel*)kernel_iter->value;
			CLLOG(INFO) << "WebCLKernel = " << kernel;
			if (kernel != NULL)
				kernel->release(ec);
		}
	}

	CLLOG(INFO) << "WebCLMemObjMap size = " << mMemObjMap.size();
	if (mMemObjMap.size() > 0) {
		for(WebCLMemObjMap::iterator mem_iter = mMemObjMap.begin(); mem_iter != mMemObjMap.end(); ++mem_iter) {
			WebCLMemoryObject* memory = (WebCLMemoryObject*)mem_iter->value;

			if (memory != NULL) {
				if(memory->isBuffer()) {
					((WebCLBuffer*)memory)->release(ec);
				}
				else {
					memory->release(ec);
				}
			}
		}
	}

	CLLOG(INFO) << "WebCLSamplerMap size = " << mSamplerMap.size();
	if (mSamplerMap.size() > 0) {
		for(WebCLSamplerMap::iterator sampler_iter = mSamplerMap.begin(); sampler_iter != mSamplerMap.end(); ++sampler_iter) {
			WebCLSampler* sampler = (WebCLSampler*)sampler_iter->value;
			if (sampler != NULL)
				sampler->release(ec);
		}
	}

	CLLOG(INFO) << "WebCLEventMap size = " << mEventMap.size();
	if (mEventMap.size() > 0) {
		for(WebCLEventMap::iterator event_iter = mEventMap.begin(); event_iter != mEventMap.end(); ++event_iter) {
			WebCLEvent* event = (WebCLEvent*)event_iter->value;
			if (event != NULL)
				event->release(ec);
		}
	}

	CLLOG(INFO) << "WebCLProgramMap size = " << mProgramMap.size();
	if (mProgramMap.size() > 0) {
		for(WebCLProgramMap::iterator program_iter = mProgramMap.begin(); program_iter != mProgramMap.end(); ++program_iter) {
			WebCLProgram* program = (WebCLProgram*)program_iter->value;
			if (program != NULL)
				program->release(ec);
		}
	}

	CLLOG(INFO) << "WebCLCommandQueueMap size = " << mCommandQueueMap.size();
	if (mCommandQueueMap.size() > 0) {
		for(WebCLCommandQueueMap::iterator commandqueue_iter = mCommandQueueMap.begin(); commandqueue_iter != mCommandQueueMap.end(); ++commandqueue_iter) {
			WebCLCommandQueue* queue = (WebCLCommandQueue*)commandqueue_iter->value;
			if (queue != NULL)
				queue->release(ec);
		}
	}

	mMemObjMap.clear();
	mSamplerMap.clear();
	mEventMap.clear();
	mProgramMap.clear();
	mCommandQueueMap.clear();
	mKernelMap.clear();
	mGlBufferMap.clear();

	release(ec);

	mClContext = NULL;
	isReleasing = false;

	mCallbackHandler->unbindToChannel();
}

WebCLCommandQueue* WebCLContext::findCLCommandQueue(cl_obj_key key)
{
	CLLOG(INFO) << "findCLCommandQueue, command queue key=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mCommandQueueMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key;
		return nullptr;
	}

	Member<WebCLCommandQueue> commandQueue = mCommandQueueMap.at(key);
	return commandQueue.Get();
}

WebCLBuffer* WebCLContext::findCLBuffer(cl_obj_key key)
{
	CLLOG(INFO) << "findCLBuffer, buffer key=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mMemObjMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key;
		return nullptr;
	}

	Member<WebCLBuffer> buffer = (WebCLBuffer*)mMemObjMap.at(key);
	return buffer.Get();
}

WebCLMemoryObject* WebCLContext::findCLMemObj(cl_obj_key key)
{
	CLLOG(INFO) << "findCLMemObj, buffer key=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mMemObjMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key;
		return nullptr;
	}

	Member<WebCLMemoryObject> buffer = mMemObjMap.at(key);
	return buffer.Get();
}

WebCLUserEvent* WebCLContext::findCLUserEvent(cl_obj_key key)
{
	CLLOG(INFO) << "findCLUserEvent, buffer key=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mEventMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key;
		return nullptr;
	}

	Member<WebCLUserEvent> userEvent = (WebCLUserEvent*)mEventMap.at(key);
	return userEvent.Get();
}

WebCLEvent* WebCLContext::findCLEvent(cl_obj_key key)
{
	CLLOG(INFO) << "findCLEvent, buffer key=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mEventMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key;
		return nullptr;
	}

	Member<WebCLEvent> event = mEventMap.at(key);
	return event.Get();
}

WebCLProgram* WebCLContext::findCLProgram(cl_obj_key key)
{
	CLLOG(INFO) << "findCLProgram, buffer key=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mProgramMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key;
		return nullptr;
	}

	Member<WebCLProgram> program = mProgramMap.at(key);
	return program.Get();
}

WebCLKernel* WebCLContext::findCLKernel(cl_obj_key key)
{
	CLLOG(INFO) << "findCLKernel, kernel key=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mKernelMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key;
		return nullptr;
	}

	Member<WebCLKernel> kernel = mKernelMap.at(key);
	return kernel.Get();
}

#if defined(OS_ANDROID)
void WebCLContext::OnCallback(unsigned eventKey, unsigned callbackKey, unsigned objectType)
#elif defined(OS_LINUX)
void WebCLContext::OnCallback(cl_point eventKey, unsigned callbackKey, unsigned objectType)
#endif
{
	switch(objectType)
	{
		case OPENCL_OBJECT_TYPE::CL_EVENT:
		{
			WebCLEvent* targetEvent = findCLEvent(eventKey);

			if(targetEvent != nullptr) {
				targetEvent->triggerCallback(callbackKey);
			}
			break;
		}
		case OPENCL_OBJECT_TYPE::CL_PROGRAM:
		{
			WebCLProgram* targetEvent = findCLProgram(eventKey);

			if (targetEvent != nullptr) {
				targetEvent->triggerCallback();
			}
			break;
		}
		default:
		break;
	}

}

void  WebCLContext::addCLEvent(WebCLEvent* event)
{
	if(event->getCLEvent() == nullptr)
		return;

	// WebCLEventMap::AddResult result = mEventMap.Set((cl_obj_key)event->getCLEvent(), event);
	mEventMap.Set((cl_obj_key)event->getCLEvent(), event);
	event->setHandlerKey(mCallbackHandler->getHandlerId());
	event->setCLContext(this);
}

void WebCLContext::addCLKernel(cl_obj_key key, WebCLKernel* clKernel)
{
	mKernelMap.Set(key, clKernel);
}

void WebCLContext::addCLBuffer(cl_obj_key key, WebCLBuffer* clBuffer)
{
	mMemObjMap.Set(key, clBuffer);
}

void WebCLContext::addCLProgram(cl_obj_key key, WebCLProgram* clProgram)
{
	mProgramMap.Set(key, clProgram);
}

void WebCLContext::removeCLEvent(cl_obj_key key)
{
	if (!isReleasing) {
		if (mEventMap.Contains(key)) {
			mEventMap.erase(key);
		}
	}
}

void WebCLContext::removeCLProgram(cl_obj_key key)
{
	if (!isReleasing) {
		if(mProgramMap.Contains(key)) {
			mEventMap.erase(key);
		}
	}
}

void WebCLContext::removeCLKernel(cl_obj_key key)
{
	if (!isReleasing) {
		if (mKernelMap.Contains(key)) {
			mKernelMap.erase(key);
		}
	}
}

void WebCLContext::removeCLSampler(cl_obj_key key)
{
	if (!isReleasing){
		if (mSamplerMap.Contains(key)) {
			mSamplerMap.erase(key);
		}
	}
}

void WebCLContext::removeCLMemObj(cl_obj_key key)
{
	if (!isReleasing){
		if (mMemObjMap.Contains(key)) {
			mMemObjMap.erase(key);
		}
	}
}

void WebCLContext::removeCLCommandQueue(cl_obj_key key)
{
	if (!isReleasing) {
		if (mCommandQueueMap.Contains(key)) {
			mCommandQueueMap.erase(key);
		}
	}
}

void WebCLContext::convertHostPtr(DOMArrayBufferView* arrayBufferView, void* convertedHostPtr, int* convertedSizeInBytes) {

	convertedHostPtr = NULL;

	if(arrayBufferView == NULL)
		return;

	unsigned int totlaLength = arrayBufferView->byteLength();
	*convertedSizeInBytes = totlaLength;

	switch(arrayBufferView->GetType()) {
	case DOMArrayBufferView::kTypeInt8:
		convertedHostPtr = (void*)(new signed char[totlaLength/sizeof(signed char)]);
		break;
	case DOMArrayBufferView::kTypeUint8:
	case DOMArrayBufferView::kTypeUint8Clamped:
		convertedHostPtr = (void*)new unsigned char[totlaLength/sizeof(unsigned char)];
		break;
	case DOMArrayBufferView::kTypeInt16:
		convertedHostPtr = (void*)new short[totlaLength/sizeof(short)];
		break;
	case DOMArrayBufferView::kTypeUint16:
		convertedHostPtr = (void*)new signed char[totlaLength/sizeof(signed char)];
		break;
	case DOMArrayBufferView::kTypeInt32:
		convertedHostPtr = (void*)new int[totlaLength/sizeof(int)];
		break;
	case DOMArrayBufferView::kTypeUint32:
		convertedHostPtr = (void*)new unsigned[totlaLength/sizeof(unsigned)];
		break;
	case DOMArrayBufferView::kTypeFloat32:
		convertedHostPtr = (void*)new float[totlaLength/sizeof(float)];
		break;
	case DOMArrayBufferView::kTypeFloat64:
		convertedHostPtr = (void*)new double[totlaLength/sizeof(double)];
		break;
	case DOMArrayBufferView::kTypeDataView:
	default:
		break;
	}

}

bool WebCLContext::setBufferInfo(WebCLBuffer* buffer, ExceptionState& ec)
{
	buffer->getMemFlag(ec);
	buffer->getMemSize(ec);

	return true;
}

WebCLProgram* WebCLContext::createProgram(const String& kernelSource, ExceptionState& ec)
{
	cl_int err = 0;
	cl_program cl_program_id = NULL;
	if (mClContext == NULL) {
		printf("Error: Invalid CL Context\n");
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return NULL;
	}

	if (kernelSource.IsEmpty()) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return NULL;
	}

	const char* source = strdup(kernelSource.Utf8().data());
	cl_program_id = webcl_clCreateProgramWithSource(webcl_channel_, mClContext, 1, (const char**)&source,
					NULL, &err);
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	} else {
		CLLOG(INFO) << "created program=" << cl_program_id;
		if (cl_program_id == NULL) {
			printf("Error: Invaild Error Type\n");
			ec.ThrowDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");
			return NULL;
		}
		WebCLProgram* o = WebCLProgram::create(mContext, cl_program_id);
		o->setSource(kernelSource);
		o->setHandlerKey((unsigned)mCallbackHandler->getHandlerId());
		o->setCLContext(this);

		// WebCLProgramMap::AddResult result = mProgramMap.Set((cl_obj_key)cl_program_id, o);
		mProgramMap.Set((cl_obj_key)cl_program_id, o);
		return o;
	}
	return NULL;
}

bool WebCLContext::setMaxImageSize(ExceptionState& ec)
{
	if(mImage2dMaxWidth == 0 && mImage2dMaxHeight == 0) {
		if(mDevices.size() == 0) {
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return false;
		}

		Member<WebCLDevice> device = mDevices[0];
		mImage2dMaxWidth = device->getImage2DMaxWidth();
		mImage2dMaxHeight = device->getImage2DMaxHeight();
	}

	return true;
}

bool WebCLContext::setMaxMemAllocSize(ExceptionState& ec)
{
	if(mMaxMemAllocSize == 0) {
		if(mDevices.size() == 0) {
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return false;
		}

		Member<WebCLDevice> device = mDevices[0];
		cl_ulong maxAllocSize = device->getDeviceMaxMemAllocSize();

		mMaxMemAllocSize = maxAllocSize;
	}

	return true;
}

bool WebCLContext::isEnabledExtension(WTF::String extensionName)
{
	if(mEnabledExtensionList.size() <= 0)
		return false;

	return mEnabledExtensionList.Contains(extensionName);
}

Vector<cl_point> WebCLContext::getClDevices(ExceptionState& ec)
{
	Vector<cl_point> list;
	CLLOG(INFO) << "mDevices.size() = " << mDevices.size();
	if(mDevices.size() == 0) {
		ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
		return list;
	}
	for(unsigned int i = 0; i < mDevices.size(); i++) {
		list.push_back((cl_point)mDevices[i]->getCLDevice());
	}
	return list;
}

HeapVector<Member<WebCLDevice>> WebCLContext::getDevices(ExceptionState& ec)
{
	//HeapVector<cl_device_id> list;
	CLLOG(INFO) << "mDevices.size() = " << mDevices.size();
	return mDevices;
}

void WebCLContext::initializeContextDevice(ExceptionState& ec) {
	if (mClContext == NULL) {
			CLLOG(INFO) << "Error: Invalid CL Context";
			ec.ThrowDOMException(WebCLException::INVALID_CONTEXT, "WebCLException::INVALID_CONTEXT");
			return;
	}

	cl_int errcode = CL_INVALID_VALUE;

	if(mDevices.size()  != 0)
		return;

	if(mNumDevices <= 0) {
		cl_uint numDevices = 0;
		CLLOG(INFO) << "initializeContextDevices numDevices";
		errcode = webcl_clGetContextInfo(webcl_channel_, mClContext, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &numDevices, NULL);
		CLLOG(INFO) << "errcode : " << errcode << ", " << numDevices;
		if(errcode == CL_SUCCESS) {
			mNumDevices = numDevices;
		}

		if(mNumDevices >0) {
		cl_device_id* devices = new cl_device_id[mNumDevices];
		errcode = webcl_clGetContextInfo(webcl_channel_, mClContext, CL_CONTEXT_DEVICES, sizeof(cl_device_id)*mNumDevices, devices, NULL);

		if(errcode == CL_SUCCESS) {
			for (unsigned int i = 0; i < mNumDevices; i++) {
				CLLOG(INFO) << "CONTEXT_DEVICE[" << i << "]=" << devices[i];
				Member<WebCLDevice> o = mContext->findCLDevice((cl_obj_key)devices[i], ec);
				if (o != NULL) {
					mDevices.push_back(o);
				} else {
					o = WebCLDevice::create(mContext.Get(), devices[i]);
					o->setDefaultValue(ec);
					if (ec.HadException()) {
						continue;
					}
					mDevices.push_back(o);
					}
				}
				return;
			}
		}
	}
}

// gl/cl sharing
WebGLRenderingContext* WebCLContext::getGLContext(ExceptionState& ec)
{
	if(!mGLContext) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	return mGLContext;
}

WebCLBuffer* WebCLContext::createFromGLBuffer(int memFlags, WebGLBuffer* glBuffer, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createFromGLBuffer, memFlags : " << memFlags << ", glBuffer : " << glBuffer;

	if(!mGLContext) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	if (!glBuffer->HasEverBeenBound()) {
		ec.ThrowDOMException(WebCLException::INVALID_GL_OBJECT, "WebCLException::INVALID_GL_OBJECT");
		return nullptr;
	}

	cl_mem_flags memFlagsNative = 0;
	switch(memFlags) {
	case WebCL::MEM_READ_ONLY:
		memFlagsNative |= CL_MEM_READ_ONLY;
		break;
	case WebCL::MEM_WRITE_ONLY:
		memFlagsNative |= CL_MEM_WRITE_ONLY;
		break;
	case WebCL::MEM_READ_WRITE:
		memFlagsNative |= CL_MEM_READ_WRITE;
		break;
	default:
		CLLOG(INFO) << "WebCLContext::createFromGLBuffer, Error: Unsupported Mem Flag";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	mContext->startHandling();

	WebCL_Operation_createBufferFromGLBuffer operation = WebCL_Operation_createBufferFromGLBuffer();
	operation.context = (cl_point)mClContext;
	operation.flags = memFlagsNative;
	operation.bufobj = ObjectOrZero(glBuffer);

	mContext->setOperationParameter(&operation);
	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::CREATE_BUFFER_FROM_GL_BUFFER);

	WebCL_Result_createFromGLBuffer result = WebCL_Result_createFromGLBuffer();
	mContext->getOperationResult(&result);
	CLLOG(INFO) << "WebCLContext::createFromGLBuffer, result.result : " << result.result << ", result.buffer : " << result.buffer;

	if(result.result == CL_SUCCESS && result.buffer) {
		CLLOG(INFO) << "WebCLContext::createFromGLBuffer, CL_SUCCESS";
		WebCLBuffer* o = WebCLBuffer::create(mContext, this, (cl_mem)result.buffer, false);
		o->setGLBuffer(true);
		o->setGLServiceID((int)result.serviceId);

		CLLOG(INFO) << "created buffer=" << (cl_mem)result.buffer;

		if(!setBufferInfo(o, ec)) {
			CLLOG(INFO) << "Error: Invaild Error Type";
			ec.ThrowDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");

			return NULL;
		}

		// WebCLMemObjMap::AddResult addResult = mMemObjMap.Set(result.buffer, (WebCLMemoryObject*)o);
		mMemObjMap.Set(result.buffer, (WebCLMemoryObject*)o);

		mGlBufferMap.Set((cl_obj_key)result.serviceId, glBuffer);

		return o;
	}

	CLLOG(INFO) << "WebCLContext::createFromGLBuffer, throwException";
	WebCLException::throwException(result.result, ec);
	return nullptr;
}

WebCLImage* WebCLContext::createFromGLRenderbuffer(int memFlags, WebGLRenderbuffer* renderbuffer, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createFromGLRenderbuffer, memFlags : " << memFlags << ", renderbuffer : " << renderbuffer;

	WebGLRenderingContext* glContext = mGLContext;
	CLLOG(INFO) << "WebCLContext::createFromGLRenderbuffer, glContext : " << glContext;

	if(!glContext) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	cl_mem_flags memFlagsNative = 0;
	switch(memFlags) {
	case WebCL::MEM_READ_ONLY:
		memFlagsNative |= CL_MEM_READ_ONLY;
		break;
	case WebCL::MEM_WRITE_ONLY:
		memFlagsNative |= CL_MEM_WRITE_ONLY;
		break;
	case WebCL::MEM_READ_WRITE:
		memFlagsNative |= CL_MEM_READ_WRITE;
		break;
	default:
		CLLOG(INFO) << "WebCLContext::createFromGLRenderbuffer, Error: Unsupported Mem Flag";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	mContext->startHandling();

	WebCL_Operation_createImageFromGLRenderbuffer operation = WebCL_Operation_createImageFromGLRenderbuffer();
	operation.context = (cl_point)mClContext;
	operation.flags = memFlagsNative;
	operation.bufobj = ObjectOrZero(renderbuffer);

	mContext->setOperationParameter(&operation);
	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::CREATE_IMAGE_FROM_GL_RENDER_BUFFER);

	WebCL_Result_createImage result = WebCL_Result_createImage();
	mContext->getOperationResult(&result);
	CLLOG(INFO) << "WebCLContext::createFromGLRenderbuffer, result.result : " << result.result << ", result.image : " << result.image;

	if(result.result == CL_SUCCESS && result.image) {
		CLLOG(INFO) << "WebCLContext::createFromGLRenderbuffer, CL_SUCCESS";
		return nullptr;
	}

	CLLOG(INFO) << "WebCLContext::createFromGLRenderbuffer, throwException";
	WebCLException::throwException(result.result, ec);

	return nullptr;
}

WebCLImage* WebCLContext::createFromGLTexture(int memFlags, int textureTarget, int miplevel, WebGLTexture* texture, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLContext::createFromGLTexture, memFlags : " << memFlags << ", textureTarget : " << textureTarget << ", miplevel : " << miplevel << ", texture : " << texture << ", ObjectOrZero(texture) : " << ObjectOrZero(texture);

	WebGLRenderingContext* glContext = mGLContext;
	CLLOG(INFO) << "WebCLContext::createFromGLTexture, glContext : " << glContext;

	if(!glContext) {
		ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return nullptr;
	}

	cl_mem_flags memFlagsNative = 0;
	switch(memFlags) {
	case WebCL::MEM_READ_ONLY:
		memFlagsNative |= CL_MEM_READ_ONLY;
		break;
	case WebCL::MEM_WRITE_ONLY:
		memFlagsNative |= CL_MEM_WRITE_ONLY;
		break;
	case WebCL::MEM_READ_WRITE:
		memFlagsNative |= CL_MEM_READ_WRITE;
		break;
	default:
		CLLOG(INFO) << "WebCLContext::createFromGLTexture, Error: Unsupported Mem Flag";
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return nullptr;
	}

	mContext->startHandling();

	WebCL_Operation_createImageFromGLTexture operation = WebCL_Operation_createImageFromGLTexture();
	operation.context = (cl_point)mClContext;
	operation.flags = memFlagsNative;
	operation.textureTarget = textureTarget;
	operation.miplevel = miplevel;
	operation.textureobj = ObjectOrZero(texture);

	mContext->setOperationParameter(&operation);
	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::CREATE_IMAGE_FROM_GL_TEXTURE);

	WebCL_Result_createImage result = WebCL_Result_createImage();
	mContext->getOperationResult(&result);
	CLLOG(INFO) << "WebCLContext::createFromGLTexture, result.result : " << result.result << ", result.image : " << result.image;

	if(result.result == CL_SUCCESS && result.image) {
		CLLOG(INFO) << "WebCLContext::createFromGLTexture, CL_SUCCESS";

		cl_mem cl_mem_id = (cl_mem)result.image;
		WebCLImage* image = WebCLImage::create(mContext.Get(), this, cl_mem_id, false);
		// WebCLMemObjMap::AddResult result = mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)image);
		mMemObjMap.Set((cl_obj_key)cl_mem_id, (WebCLMemoryObject*)image);

		return image;
	}

	CLLOG(INFO) << "WebCLContext::createFromGLTexture, throwException";
	WebCLException::throwException(result.result, ec);

	return nullptr;
}

WebGLSharedPlatform3DObject* WebCLContext::findGLBuffer(cl_obj_key key)
{
	CLLOG(INFO) << "findGLBuffer, gl_service_id=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mGlBufferMap.Contains(key)) {
		CLLOG(INFO) << ">> no object hashed, key=" << key;
		return nullptr;
	}

	Member<WebGLSharedPlatform3DObject> glBuffer = mGlBufferMap.at(key);
	return glBuffer.Get();
}

void WebCLContext::removeGLBuffer(cl_obj_key key)
{
	if (!isReleasing) {
		if (mGlBufferMap.Contains(key)) {
			mGlBufferMap.erase(key);
		}
	}
}

void WebCLContext::setGLContext(WebGLRenderingContext* mGL) {
	mGLContext = mGL;
}

DEFINE_TRACE(WebCLContext) {
	visitor->Trace(mContext);
	visitor->Trace(mDevices);
	visitor->Trace(mCommandQueueMap);
	visitor->Trace(mEventMap);
	visitor->Trace(mMemObjMap);
	visitor->Trace(mProgramMap);
	visitor->Trace(mKernelMap);
	visitor->Trace(mSamplerMap);
	visitor->Trace(mGlBufferMap);
	visitor->Trace(mGLContext);
	visitor->Trace(mMemoryUtil);
}

}
