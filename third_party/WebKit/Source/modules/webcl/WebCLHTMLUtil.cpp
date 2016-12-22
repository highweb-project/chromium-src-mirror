// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"

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
    const void* imagePixelData = imageExtractor.imagePixelData();
    if(!imagePixelData)
     	return false;

    WebGLImageConversion::DataFormat sourceDataFormat = imageExtractor.imageSourceFormat();
    WebGLImageConversion::AlphaOp alphaOp = imageExtractor.imageAlphaOp();
//    const void* imagePixelData = imageExtractor.imagePixelData();
    unsigned imageSourceUnpackAlignment = imageExtractor.imageSourceUnpackAlignment();

    return WebGLImageConversion::packImageData(image, imagePixelData, GL_RGBA, GL_UNSIGNED_BYTE, false, alphaOp, sourceDataFormat, width, height, imageSourceUnpackAlignment, data);
}

bool WebCLHTMLUtil::extractDataFromCanvas(HTMLCanvasElement* canvas, Vector<uint8_t>& data, size_t& canvasSize, ExceptionState& es)
{
    if (!canvas) {
        es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    // TODO (aiken)
    // if (!packImageData(canvas->copiedImage(BackBuffer).get(), WebGLImageConversion::HtmlDomCanvas, canvas->width(), canvas->height(), data)) {
    // 	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
    //     return false;
    // }
    if (!packImageData(canvas->copiedImage(BackBuffer, AccelerationHint::PreferAcceleration).get(), WebGLImageConversion::HtmlDomCanvas, canvas->width(), canvas->height(), data)) {
      	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    canvasSize = data.size();
    if (!data.data() || !canvasSize) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    return true;
}

bool WebCLHTMLUtil::extractDataFromImage(HTMLImageElement* image, Vector<uint8_t>& data, size_t& imageSize, ExceptionState& es)
{
    if (!image || !image->cachedImage()) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    if (!packImageData(image->cachedImage()->getImage(), WebGLImageConversion::HtmlDomImage, image->width(), image->height(), data)) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    imageSize = data.size();
    if (!data.data() || !imageSize) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    return true;
}

bool WebCLHTMLUtil::extractDataFromImageData(ImageData* srcPixels, void*& hostPtr, size_t& pixelSize, ExceptionState& es)
{
    if (!srcPixels && !srcPixels->data() && !srcPixels->data()->data()) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    pixelSize = srcPixels->data()->length();
    hostPtr = static_cast<void*>(srcPixels->data()->data());
    if (!hostPtr || !pixelSize) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    return true;
}

bool WebCLHTMLUtil::extractDataFromVideo(HTMLVideoElement* video, Vector<uint8_t>& data, size_t& videoSize, ExceptionState& es)
{
    if (!video) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    RefPtr<Image> image = videoFrameToImage(video);
    if (!image) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }

    if (!packImageData(image.get(), WebGLImageConversion::HtmlDomVideo, video->clientWidth(), video->clientHeight(), data)) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
        return false;
    }
    videoSize = data.size();

    if (!data.data() || !videoSize) {
    	es.throwDOMException(WebCLException::INVALID_HOST_PTR, "WebCLException::INVALID_HOST_PTR");
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

    IntRect destRect(0, 0, size.width(), size.height());
    video->paintCurrentFrame(imageBufferObject->canvas(), destRect, nullptr);
    return imageBufferObject->newImageSnapshot();
}

WebCLHTMLUtil::WebCLHTMLUtil(unsigned capacity)
    : m_generatedImageCache(capacity)
{
}

WebCLHTMLUtil::ImageBufferCache::ImageBufferCache(unsigned capacity)
    : m_capacity(capacity)
{
    m_buffers.reserveCapacity(capacity);
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

    OwnPtr<ImageBuffer> temp = ImageBuffer::create(size);
    if (!temp)
        return nullptr;

    if (i < m_capacity - 1) {
        m_buffers.append(std::move(temp));
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
