// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLInputChecker_h
#define WebCLInputChecker_h

#include "core/dom/DOMArrayBufferView.h"
#include "WebCLInclude.h"

namespace blink {

class WebCLBuffer;
class WebCLContext;
class WebCLImage;
class WebCLImageDescriptor;
class WebCLKernel;

namespace WebCLInputChecker {

bool isValidDataSizeForDOMArrayBufferView(unsigned size, DOMArrayBufferView*);
bool isValidRegionForMemoryObject(const Vector<size_t>& origin, const Vector<size_t>& region, size_t rowPitch, size_t slicePitch, size_t length);
bool isValidRegionForHostPtr(const Vector<unsigned>& region, size_t rowPitch, const WebCLImageDescriptor&, size_t length);
bool isValidRegionForImage(const WebCLImageDescriptor&, const Vector<unsigned>& origin, const Vector<unsigned>& region);
bool isValidRegionForBuffer(const size_t bufferLength, const Vector<unsigned>& region, const size_t offset, const WebCLImageDescriptor&);
bool isRegionOverlapping(WebCLImage*, WebCLImage*, const Vector<unsigned>& srcOrigin, const Vector<unsigned>& dstOrigin, const Vector<unsigned>& region);
bool isRegionOverlapping(WebCLBuffer*, WebCLBuffer*, const unsigned srcOffset, const unsigned dstOffset, const unsigned numBytes);
unsigned bytesPerChannelType(unsigned channelType);
unsigned numberOfChannelsForChannelOrder(unsigned order);
int fromArrayBufferTypeToDataType(int type);
size_t fromArrayBufferSizeToListSize(DOMArrayBufferView* hostPtr);

} // namespace WebCLInputChecker

} // namespace blink



#endif /* THIRD_PARTY_WEBKIT_SOURCE_MODULES_WEBCL_WEBCLINPUTCHECKER_H_ */
