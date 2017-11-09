// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "WebCLBuffer.h"
#include "WebCLContext.h"
#include "WebCLImage.h"
#include "modules/webcl/WebCLImageDescriptor.h"
#include "WebCLInputChecker.h"
#include "WebCLKernel.h"
#include "WebCL.h"

namespace blink {
namespace WebCLInputChecker {

bool isValidDataSizeForDOMArrayBufferView(unsigned size, DOMArrayBufferView* arrayBufferView)
{
    if (!arrayBufferView) {
        LOG(ERROR) << "arrayBufferView is null";
        return false;
    }
    unsigned bytesPerElement = 1;
    switch (arrayBufferView->GetType()) {
    case DOMArrayBufferView::kTypeInt8:
    case DOMArrayBufferView::kTypeUint8:
    case DOMArrayBufferView::kTypeUint8Clamped:
        bytesPerElement = 1;
        break;
    case DOMArrayBufferView::kTypeInt16:
    case DOMArrayBufferView::kTypeUint16:
        bytesPerElement = 2;
        break;
    case DOMArrayBufferView::kTypeInt32:
    case DOMArrayBufferView::kTypeUint32:
    case DOMArrayBufferView::kTypeFloat32:
        bytesPerElement = 4;
        break;
    case DOMArrayBufferView::kTypeFloat64:
        bytesPerElement = 8;
        break;
    default:
        LOG(ERROR) << "type not match";
        return false;
    }

    return !(size % bytesPerElement);
}

static bool valueInRange(size_t value, size_t minimum, size_t maximum)
{
    return ((value >= minimum) && (value <= maximum));
}

unsigned bytesPerChannelType(unsigned channelType)
{
    switch(channelType) {
    case CL_SNORM_INT8:
    case CL_UNORM_INT8:
    case CL_SIGNED_INT8:
    case CL_UNSIGNED_INT8:
        return 1;
    case CL_SNORM_INT16:
    case CL_UNORM_INT16:
    case CL_SIGNED_INT16:
    case CL_UNSIGNED_INT16:
    case CL_HALF_FLOAT:
        return 2;
    case CL_SIGNED_INT32:
    case CL_UNSIGNED_INT32:
    case CL_FLOAT:
    case WebCL::UNORM_INT24:
        return 4;
    case CL_UNORM_SHORT_565:
    case CL_UNORM_SHORT_555:
    case CL_UNORM_INT_101010:
        break;
    }

    return 0;
}

unsigned numberOfChannelsForChannelOrder(unsigned order)
{
    switch (order) {
    case CL_R:
    case CL_A:
    case CL_INTENSITY:
    case CL_LUMINANCE:
        return 1;
    case CL_RG:
    case CL_RA:
    case CL_Rx:
        return 2;
    case CL_RGB:
    case CL_RGx:
        return 3;
    case CL_RGBA:
    case CL_BGRA:
    case CL_ARGB:
    case CL_RGBx:
    case WebCL::sRGB:
    case WebCL::sRGBx:
    case WebCL::sRGBA:
    case WebCL::sBGRA:
    case WebCL::ABGR:
        return 4;
    }

    return 0;
}

bool isValidRegionForMemoryObject(const Vector<size_t>& origin, const Vector<size_t>& region, size_t rowPitch, size_t slicePitch, size_t length)
{
    size_t regionArea = region[0] * region[1] * region[2];
    if (!regionArea)
        return false;

    if (rowPitch) {
        // Validate User given rowPitch, region read = rowPitch * number of rows * number of slices.
        // The rowPitch is used to move the pointer to the next read the next row. By default its set to
        // row width. With user sent values we must ensure the read is within the bounds.
        size_t maximumReadPtrValue = rowPitch * region[1] * region[2];
        if (maximumReadPtrValue > length)
            return false;
    }

    if (slicePitch) {
        // Validate User given slicePitch , region read = slicePitch * number of slices.
        // The slicePitch is used to move the pointer for the next slice. Default value is size of slice
        // in bytes ( region[1] * rowPitch). Must be validated identical to rowPitch to avoid out of bound memory access.
        size_t maximumReadPtrValue = slicePitch * region[2];
        if (maximumReadPtrValue > length)
            return false;
    }

    // If row_pitch is 0, row_pitch is computed as region[0].
    rowPitch = rowPitch ? rowPitch : region[0];
    if (rowPitch < region[0])
        return false;

    // If slice_pitch is 0, slice_pitch is computed as region[1] * row_pitch.
    slicePitch = slicePitch ? slicePitch : (region[1] * rowPitch);
    if (slicePitch < rowPitch * region[1])
        return false;

    // The offset in bytes is computed as origin[2] * host_slice_pitch + origin[1] * rowPitch + origin[0].
    size_t offset = origin[2] * slicePitch + origin[1]  * rowPitch + origin[0];

    return (regionArea + offset) <= length;
}

bool isValidRegionForImage(const WebCLImageDescriptor& descriptor, const Vector<unsigned>& origin, const Vector<unsigned>& region)
{
    size_t regionArea = region[0] * region[1];
    if (!regionArea)
        return false;

    size_t height = descriptor.height();
    size_t width = descriptor.width();
    size_t offsetFromOrigin = origin[1] * height + origin[0];
    return (offsetFromOrigin + regionArea) <= (height * width);
}

bool isValidRegionForBuffer(const size_t bufferLength, const Vector<unsigned>& region, const size_t offset, const WebCLImageDescriptor& descriptor)
{
    // The size in bytes of the region to be copied from buffer is width * height * bytes/image element.
    size_t bytesCopied = region[0] * region[1] * bytesPerChannelType(descriptor.channelType()) * numberOfChannelsForChannelOrder(descriptor.channelOrder());

    return (offset+ bytesCopied) <= bufferLength;
}

bool isValidRegionForHostPtr(const Vector<unsigned>& region, size_t rowPitch, const WebCLImageDescriptor& descriptor, size_t length)
{
    /*
    *  Validate the hostPtr length passed to enqueue*Image* API's. Since hostPtr are not validated by OpenCL
    *  Out of Bound access may cause crashes. So validating with rowPitch & region being read.
    *  rowPitch is used to move the pointer to next row for write/read.
    */
    size_t imageBytesPerPixel = bytesPerChannelType(descriptor.channelType()) * numberOfChannelsForChannelOrder(descriptor.channelOrder());
    rowPitch = rowPitch ? rowPitch : region[0] * imageBytesPerPixel;
    if (rowPitch * region[1] > length)
        return false;

    size_t regionArea = region[0] * region[1];
    if (!regionArea)
        return false;

    return (regionArea <= length);
}



bool isRegionOverlapping(WebCLImage* source, WebCLImage* destination, const Vector<unsigned>& sourceOrigin, const Vector<unsigned>& destinationOrigin, const Vector<unsigned>& region)
{
    if (!source || !destination)
        return false;

    if (sourceOrigin.size() != 2 || destinationOrigin.size() != 2 || region.size() != 2)
        return false;

    if (source->getCLMem() != destination->getCLMem())
        return false;

    bool xOverlap = valueInRange(destinationOrigin[0], sourceOrigin[0], (region[0] + sourceOrigin[0])) || valueInRange(sourceOrigin[0], destinationOrigin[0], (destinationOrigin[0] + region[0]));
    bool yOverlap = valueInRange(destinationOrigin[1], sourceOrigin[1], (region[1] + sourceOrigin[1])) || valueInRange(sourceOrigin[1], destinationOrigin[1], (destinationOrigin[1] + region[1]));

    return xOverlap && yOverlap;
}

bool isRegionOverlapping(WebCLBuffer* srcBuffer, WebCLBuffer* destBuffer, const unsigned srcOffset, const unsigned dstOffset, const unsigned numBytes)
{
    if (!srcBuffer || !destBuffer)
        return false;

    if (srcBuffer->getCLMem() != destBuffer->getCLMem())
        return false;

    return valueInRange(dstOffset, srcOffset, (srcOffset + numBytes)) || valueInRange(srcOffset, dstOffset, (dstOffset + numBytes));
}

int fromArrayBufferTypeToDataType(int type)
{
	switch (type) {
	case DOMArrayBufferView::kTypeInt8:
	case DOMArrayBufferView::kTypeUint8:
	case DOMArrayBufferView::kTypeUint8Clamped:
		return HOST_PTR_DATA_TYPE::UNSIGNED_CHAR;
	case DOMArrayBufferView::kTypeInt16:
	case DOMArrayBufferView::kTypeUint16:
		return HOST_PTR_DATA_TYPE::SHORT;
	case DOMArrayBufferView::kTypeInt32:
		return HOST_PTR_DATA_TYPE::INT;
	case DOMArrayBufferView::kTypeUint32:
		return HOST_PTR_DATA_TYPE::UNSIGNED;
	case DOMArrayBufferView::kTypeFloat32:
		return HOST_PTR_DATA_TYPE::FLOAT;
	case DOMArrayBufferView::kTypeFloat64:
		return HOST_PTR_DATA_TYPE::DOUBLE;
	default:
		LOG(ERROR) << "type not match";
		return -1;
	}
}

size_t fromArrayBufferSizeToListSize(DOMArrayBufferView* hostPtr)
{
	switch (hostPtr->GetType()) {
	case DOMArrayBufferView::kTypeInt8:
	case DOMArrayBufferView::kTypeUint8:
	case DOMArrayBufferView::kTypeUint8Clamped:
		return hostPtr->byteLength()/sizeof(unsigned char);
	case DOMArrayBufferView::kTypeInt16:
	case DOMArrayBufferView::kTypeUint16:
		return hostPtr->byteLength()/sizeof(short);
	case DOMArrayBufferView::kTypeInt32:
		return hostPtr->byteLength()/sizeof(int);
	case DOMArrayBufferView::kTypeUint32:
		return hostPtr->byteLength()/sizeof(unsigned);
	case DOMArrayBufferView::kTypeFloat32:
		return hostPtr->byteLength()/sizeof(float);
	case DOMArrayBufferView::kTypeFloat64:
		return hostPtr->byteLength()/sizeof(double);
	default:
		LOG(ERROR) << "type not match";
		return -1;
	}
}

} // namespace WebCLInputChecker
} // namespace blink



