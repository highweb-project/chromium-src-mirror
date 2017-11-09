// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "core/html/HTMLCanvasElement.h"
#include "core/html/HTMLImageElement.h"
#include "core/html/HTMLVideoElement.h"
#include "core/html/ImageData.h"
#include "platform/graphics/gpu/WebGLImageConversion.h"
#include "platform/graphics/ImageBuffer.h"
#include "platform/graphics/Image.h"
#include "platform/geometry/IntSize.h"
#include "WebCL.h"
#include "WebCLHTMLUtil.h"
#include "core/dom/custom/WebCL/WebCLException.h"

namespace blink {

bool packImageData(Image* image, WebGLImageConversion::ImageHtmlDomSource domSource, unsigned width, unsigned height, Vector<uint8_t>& data) {
    WebGLImageConversion::ImageExtractor imageExtractor(image, domSource, false, false);
    const void* imagePixelData = imageExtractor.ImagePixelData();
    if(!imagePixelData)
     	return false;

    WebGLImageConversion::DataFormat sourceDataFormat = imageExtractor.ImageSourceFormat();
    WebGLImageConversion::AlphaOp alphaOp = imageExtractor.ImageAlphaOp();
//    const void* imagePixelData = imageExtractor.imagePixelData();
    unsigned imageSourceUnpackAlignment = imageExtractor.ImageSourceUnpackAlignment();

    IntRect subRect;
    if (!image) {
      subRect = IntRect();
    }
    subRect = IntRect(0, 0, width, height);

    // return WebGLImageConversion::packImageData(image, imagePixelData, GL_RGBA, GL_UNSIGNED_BYTE, false, alphaOp, sourceDataFormat, width, height, imageSourceUnpackAlignment, data);
    return WebGLImageConversion::PackImageData(
      image, imagePixelData, GL_RGBA, GL_UNSIGNED_BYTE,
      false, alphaOp, sourceDataFormat, width,
      height, subRect, 1, imageSourceUnpackAlignment, image->height(), data);
}

bool WebCLHTMLUtil::extractDataFromCanvas(HTMLCanvasElement* canvas, Vector<uint8_t>& data, size_t& canvasSize, ExceptionState& es)
{
    if (!canvas) {
        es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    // TODO (aiken)
    // if (!packImageData(canvas->copiedImage(BackBuffer).get(), WebGLImageConversion::HtmlDomCanvas, canvas->width(), canvas->height(), data)) {
    // 	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
    //     return false;
    // }
    if (!packImageData(canvas->CopiedImage(kBackBuffer, AccelerationHint::kPreferAcceleration, kSnapshotReasonGetCopiedImage).Get(), WebGLImageConversion::kHtmlDomCanvas, canvas->width(), canvas->height(), data)) {
      	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    canvasSize = data.size();
    if (!data.data() || !canvasSize) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    return true;
}

bool WebCLHTMLUtil::extractDataFromImage(HTMLImageElement* image, Vector<uint8_t>& data, size_t& imageSize, ExceptionState& es)
{
    if (!image || !image->CachedImage()) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    if (!packImageData(image->CachedImage()->GetImage(), WebGLImageConversion::kHtmlDomImage, image->width(), image->height(), data)) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    imageSize = data.size();
    if (!data.data() || !imageSize) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    return true;
}

bool WebCLHTMLUtil::extractDataFromImageData(ImageData* srcPixels, void*& hostPtr, size_t& pixelSize, ExceptionState& es)
{
    if (!srcPixels && !srcPixels->data() && !srcPixels->data()->Data()) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    pixelSize = srcPixels->data()->length();
    hostPtr = static_cast<void*>(srcPixels->data()->Data());
    if (!hostPtr || !pixelSize) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    return true;
}

bool WebCLHTMLUtil::extractDataFromVideo(HTMLVideoElement* video, Vector<uint8_t>& data, size_t& videoSize, ExceptionState& es)
{
    if (!video) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    RefPtr<Image> image = videoFrameToImage(video);
    if (!image) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    if (!packImageData(image.Get(), WebGLImageConversion::kHtmlDomVideo, video->clientWidth(), video->clientHeight(), data)) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }
    videoSize = data.size();

    if (!data.data() || !videoSize) {
    	es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }
    return true;
}

PassRefPtr<Image> WebCLHTMLUtil::videoFrameToImage(HTMLVideoElement* video)
{
    if (!video || !video->clientWidth() || !video->clientHeight())
        return nullptr;

    IntSize size(video->clientWidth(), video->clientHeight());
    ImageBuffer* imageBufferObject = m_generatedImageCache.imageBuffer(size);
    if (!imageBufferObject)
        return nullptr;

    IntRect destRect(0, 0, size.Width(), size.Height());
    video->PaintCurrentFrame(imageBufferObject->Canvas(), destRect, nullptr);
    return imageBufferObject->NewImageSnapshot();
}

WebCLHTMLUtil::WebCLHTMLUtil(unsigned capacity)
    : m_generatedImageCache(capacity)
{
}

WebCLHTMLUtil::ImageBufferCache::ImageBufferCache(unsigned capacity)
    : m_capacity(capacity)
{
    m_buffers.ReserveCapacity(capacity);
}

ImageBuffer* WebCLHTMLUtil::ImageBufferCache::imageBuffer(const IntSize& size)
{
    unsigned i;
    for (i = 0; i < m_buffers.size(); ++i) {
        ImageBuffer* buf = m_buffers[i].get();
        if (buf->size() != size)
            continue;

        if (i > 0)
            m_buffers[i].swap(m_buffers[0]);

        return buf;
    }

    std::unique_ptr<ImageBuffer> temp = ImageBuffer::Create(size);
    if (!temp)
        return nullptr;

    if (i < m_capacity - 1) {
        m_buffers.push_back(std::move(temp));
    } else {
        m_buffers[m_capacity - 1] = std::move(temp);
        i = m_capacity - 1;
    }

    ImageBuffer* buf = m_buffers[i].get();
    if (i > 0)
        m_buffers[i].swap(m_buffers[0]);

    return buf;
}

}
