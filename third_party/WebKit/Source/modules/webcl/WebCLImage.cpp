// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "WebCLImage.h"

#include "WebCL.h"
#include "core/dom/custom/WebCL/WebCLException.h"


namespace blink {

WebCLImage::WebCLImage(WebCL* context, WebCLContext* clContext, cl_mem memory, bool isShared)
	: WebCLMemoryObject(context, clContext, memory, isShared)
{
	getInfo(mImageDescriptor);
}

WebCLImage::~WebCLImage()
{
}

void WebCLImage::getInfo(WebCLImageDescriptor& descriptor)
{
	CLLOG(INFO) << "WebCLImage::getInfo";

	if(mClMemObj == NULL) {
		return;
	}

	cl_int errCode = WebCLException::FAILURE;

	cl_image_format imageFormat = {
			0,0
	};
	errCode = webcl_clGetImageInfo(webcl_channel_, mClMemObj, CL_IMAGE_FORMAT, sizeof(cl_image_format), &imageFormat, NULL);
	if(errCode != CL_SUCCESS) {
		return;
	}
	descriptor.setChannelOrder(imageFormat.image_channel_order);
	descriptor.setChannelType(imageFormat.image_channel_data_type);

	size_t infoTemp = 0;
	errCode = webcl_clGetImageInfo(webcl_channel_, mClMemObj, CL_IMAGE_WIDTH, sizeof(size_t), &infoTemp, NULL);
	if(errCode != CL_SUCCESS) {
		return;
	}
	descriptor.setWidth(infoTemp);

	errCode = webcl_clGetImageInfo(webcl_channel_, mClMemObj, CL_IMAGE_HEIGHT, sizeof(size_t), &infoTemp, NULL);
	if(errCode != CL_SUCCESS) {
		return;
	}
	descriptor.setHeight(infoTemp);

	errCode = webcl_clGetImageInfo(webcl_channel_, mClMemObj, CL_IMAGE_ROW_PITCH, sizeof(size_t), &infoTemp, NULL);
	if(errCode != CL_SUCCESS) {
		return;
	}
	descriptor.setRowPitch(infoTemp);
}

DEFINE_TRACE(WebCLImage) {
	WebCLMemoryObject::Trace(visitor);
}

}
