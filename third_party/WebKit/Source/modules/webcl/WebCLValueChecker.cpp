/*
 * WebCLValueChecker.cpp
 *
 *  Created on: 2015. 7. 6.
 *      Author: azureskybox
 */

#include "platform/wtf/build_config.h"
#include "WebCLValueChecker.h"

#include "WebCL.h"
#include "modules/webcl/WebCLImageDescriptor.h"

namespace blink {

	bool WebCLValueChecker::isImageChannelOrderValid(unsigned channelOrder) {
		switch(channelOrder) {
		case WebCL::RGBA:
		case WebCL::R:
		case WebCL::A:
		case WebCL::RG:
		case WebCL::RA:
		case WebCL::RGB:
		case WebCL::BGRA:
		case WebCL::ARGB:
		case WebCL::INTENSITY:
		case WebCL::LUMINANCE:
		case WebCL::Rx:
		case WebCL::RGx:
		case WebCL::RGBx:
		//OpenCL 2.0 Spec
		case WebCL::DEPTH:
		case WebCL::DEPTH_STENCIL:
		case WebCL::sRGB:
		case WebCL::sRGBx:
		case WebCL::sRGBA:
		case WebCL::sBGRA:
		case WebCL::ABGR:
			return true;
		}
		return false;
	}

	bool WebCLValueChecker::isImageChannelTypeValid(unsigned channelType) {
		switch(channelType) {
		case WebCL::SNORM_INT8:
		case WebCL::SNORM_INT16:
		case WebCL::UNORM_INT8:
		case WebCL::UNORM_INT16:
		case WebCL::UNORM_SHORT_565:
		case WebCL::UNORM_SHORT_555:
		case WebCL::UNORM_INT_101010:
		case WebCL::SIGNED_INT8:
		case WebCL::SIGNED_INT16:
		case WebCL::SIGNED_INT32:
		case WebCL::UNSIGNED_INT8:
		case WebCL::UNSIGNED_INT16:
		case WebCL::UNSIGNED_INT32:
		case WebCL::HALF_FLOAT:
		case WebCL::FLOAT:
		//OpenCL 2.0 Spec
		case WebCL::UNORM_INT24:
			return true;
		}
		return false;
	}

	bool WebCLValueChecker::isImageDescriptorValid(WebCLImageDescriptor descriptor) {
		return isImageChannelOrderValid(descriptor.channelOrder()) & isImageChannelOrderValid(descriptor.channelType());
	}

	bool WebCLValueChecker::isDeviceTypeValid(int deviceType) {
		switch(deviceType) {
		case WebCL::DEVICE_TYPE_ACCELERATOR:
		case WebCL::DEVICE_TYPE_ALL:
		case WebCL::DEVICE_TYPE_DEFAULT:
		case WebCL::DEVICE_TYPE_CPU:
		case WebCL::DEVICE_TYPE_GPU:
			return true;
		}

		return false;
	}

	bool WebCLValueChecker::isCommandQueuePropertyValid(int property)
	{
		switch(property) {
		case WebCL::QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
		case WebCL::QUEUE_PROFILING_ENABLE:
			return true;
		}

		return false;
	}

	bool WebCLValueChecker::isCommandQueueInfoNameValid(int name) {
		switch(name) {
		case WebCL::QUEUE_CONTEXT:
		case WebCL::QUEUE_DEVICE:
		case WebCL::QUEUE_PROPERTIES:
			return true;
		}

		return false;
	}
}


