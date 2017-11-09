// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLCommandQueue_h
#define WebCLCommandQueue_h

#include "core/events/EventTarget.h"
#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/Nullable.h"
#include "platform/heap/Handle.h"
#include "core/typed_arrays/DOMArrayBufferView.h"

#include "WebCLInclude.h"
#include "WebCLEvent.h"
#include "WebCLContext.h"

namespace blink {

class WebCL;
class WebCLBuffer;
class WebCLMemoryObject;
class WebCLImage;
class WebCLKernel;
class WebCLDevice;
class HTMLVideoElement;
class WebCLHTMLUtil;

class WebCLCommandQueue : public GarbageCollectedFinalized<WebCLCommandQueue>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static WebCLCommandQueue* create(WebCL* context, cl_context clContext, cl_command_queue commandQueue)
    {
    	WebCLCommandQueue* webCLCommandQueue = new WebCLCommandQueue(context, clContext, commandQueue);
        return webCLCommandQueue;
    }
    virtual ~WebCLCommandQueue();

    /*
     * JS API Defines
     */
    void enqueueCopyBuffer(
    		WebCLBuffer* srcBuffer,
			WebCLBuffer* dstBuffer,
			unsigned srcOffset,
			unsigned dstOffset,
			unsigned numBytes,
			Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
			WebCLEvent* event,
			ExceptionState& ec);
    void enqueueCopyBuffer(
    		WebCLBuffer* srcBuffer,
			WebCLBuffer* dstBuffer,
			unsigned srcOffset,
			unsigned dstOffset,
			unsigned numBytes,
			ExceptionState& ec){enqueueCopyBuffer(srcBuffer, dstBuffer, srcOffset, dstOffset, numBytes, nullptr, nullptr, ec);};
    void enqueueCopyBuffer(
    		WebCLBuffer* srcBuffer,
			WebCLBuffer* dstBuffer,
			unsigned srcOffset,
			unsigned dstOffset,
			unsigned numBytes,
			Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
			ExceptionState& ec){enqueueCopyBuffer(srcBuffer, dstBuffer, srcOffset, dstOffset, numBytes, eventWaitList, nullptr, ec);};

    void enqueueCopyBufferRect(
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
			ExceptionState& ec);
    void enqueueCopyBufferRect(
    		WebCLBuffer* srcBuffer,
			WebCLBuffer* dstBuffer,
			Vector<unsigned> srcOrigin,
			Vector<unsigned> dstOrigin,
			Vector<unsigned> region,
			unsigned srcRowPitch,
			unsigned srcSlicePitch,
			unsigned dstRowPitch,
			unsigned dstSlicePitch,
			ExceptionState& ec){enqueueCopyBufferRect(srcBuffer, dstBuffer, srcOrigin, dstOrigin, region, srcRowPitch, srcSlicePitch, dstRowPitch, dstSlicePitch, nullptr, nullptr, ec);};
    void enqueueCopyBufferRect(
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
			ExceptionState& ec){enqueueCopyBufferRect(srcBuffer, dstBuffer, srcOrigin, dstOrigin, region, srcRowPitch, srcSlicePitch, dstRowPitch, dstSlicePitch, eventWaitList, nullptr, ec);};

    //!! auto generated !!//

    void enqueueCopyImage(
    		WebCLImage* srcImage,
    		WebCLImage* dstImage,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> dstOrigin,
    		Vector<unsigned> region,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueCopyImage(
    		WebCLImage* srcImage,
    		WebCLImage* dstImage,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> dstOrigin,
    		Vector<unsigned> region,
    		ExceptionState& ec){ enqueueCopyImage(srcImage, dstImage, srcOrigin, dstOrigin, region, nullptr, nullptr, ec);};
    void enqueueCopyImage(
    		WebCLImage* srcImage,
    		WebCLImage* dstImage,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> dstOrigin,
    		Vector<unsigned> region,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueCopyImage(srcImage, dstImage, srcOrigin, dstOrigin, region, eventWaitList, nullptr, ec);};

    void enqueueCopyImageToBuffer(
    		WebCLImage* srcImage,
    		WebCLBuffer* dstBuffer,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> srcRegion,
    		unsigned dstOffset,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueCopyImageToBuffer(
    		WebCLImage* srcImage,
    		WebCLBuffer* dstBuffer,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> srcRegion,
    		unsigned dstOffset,
    		ExceptionState& ec){ enqueueCopyImageToBuffer(srcImage, dstBuffer, srcOrigin, srcRegion, dstOffset, nullptr, nullptr, ec);};
    void enqueueCopyImageToBuffer(
    		WebCLImage* srcImage,
    		WebCLBuffer* dstBuffer,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> srcRegion,
    		unsigned dstOffset,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueCopyImageToBuffer(srcImage, dstBuffer, srcOrigin, srcRegion, dstOffset, eventWaitList, nullptr, ec);};

    void enqueueCopyBufferToImage(
    		WebCLBuffer* srcBuffer,
    		WebCLImage* dstImage,
    		unsigned srcOffset,
    		Vector<unsigned> dstOrigin,
    		Vector<unsigned> dstRegion,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueCopyBufferToImage(
    		WebCLBuffer* srcBuffer,
    		WebCLImage* dstImage,
    		unsigned srcOffset,
    		Vector<unsigned> dstOrigin,
    		Vector<unsigned> dstRegion,
    		ExceptionState& ec){ enqueueCopyBufferToImage(srcBuffer, dstImage, srcOffset, dstOrigin, dstRegion, nullptr, nullptr, ec);};
    void enqueueCopyBufferToImage(
    		WebCLBuffer* srcBuffer,
    		WebCLImage* dstImage,
    		unsigned srcOffset,
    		Vector<unsigned> dstOrigin,
    		Vector<unsigned> dstRegion,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueCopyBufferToImage(srcBuffer, dstImage, srcOffset, dstOrigin, dstRegion, eventWaitList, nullptr, ec);};

    void enqueueReadBuffer(
    		WebCLBuffer* buffer,
    		bool blockingRead,
    		unsigned bufferOffset,
    		unsigned numBytes,
    		NotShared<DOMArrayBufferView> hostPtr,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueReadBuffer(
    		WebCLBuffer* buffer,
    		bool blockingRead,
    		unsigned bufferOffset,
    		unsigned numBytes,
    		NotShared<DOMArrayBufferView> hostPtr,
    		ExceptionState& ec){ enqueueReadBuffer(buffer, blockingRead, bufferOffset, numBytes, hostPtr, nullptr, nullptr, ec);};
    void enqueueReadBuffer(
    		WebCLBuffer* buffer,
    		bool blockingRead,
    		unsigned bufferOffset,
    		unsigned numBytes,
    		NotShared<DOMArrayBufferView> hostPtr,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueReadBuffer(buffer, blockingRead, bufferOffset, numBytes, hostPtr, eventWaitList, nullptr, ec);};

    void enqueueReadBufferRect(
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
    		ExceptionState& ec);
    void enqueueReadBufferRect(
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
    		ExceptionState& ec){ enqueueReadBufferRect(buffer, blockingRead, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, hostPtr, nullptr, nullptr, ec);};
    void enqueueReadBufferRect(
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
    		ExceptionState& ec){ enqueueReadBufferRect(buffer, blockingRead, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, hostPtr, eventWaitList, nullptr, ec);};

    void enqueueReadImage(
    		WebCLImage* image,
    		bool blockingRead,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
    		unsigned hostRowPitch,
    		NotShared<DOMArrayBufferView> hostPtr,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueReadImage(
    		WebCLImage* image,
    		bool blockingRead,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
    		unsigned hostRowPitch,
    		NotShared<DOMArrayBufferView> hostPtr,
    		ExceptionState& ec){ enqueueReadImage(image, blockingRead, origin, region, hostRowPitch, hostPtr, nullptr, nullptr, ec);};
    void enqueueReadImage(
    		WebCLImage* image,
    		bool blockingRead,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
    		unsigned hostRowPitch,
    		NotShared<DOMArrayBufferView> hostPtr,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueReadImage(image, blockingRead, origin, region, hostRowPitch, hostPtr, eventWaitList, nullptr, ec);};

    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
    		unsigned numBytes,
    		NotShared<DOMArrayBufferView> hostPtr,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
    		unsigned numBytes,
    		NotShared<DOMArrayBufferView> hostPtr,
    		ExceptionState& ec){ enqueueWriteBuffer(buffer, blockingWrite, bufferOffset, numBytes, hostPtr, nullptr, nullptr, ec);};
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
    		unsigned numBytes,
    		NotShared<DOMArrayBufferView> hostPtr,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteBuffer(buffer, blockingWrite, bufferOffset, numBytes, hostPtr, eventWaitList, nullptr, ec);};

    void enqueueWriteBufferRect(
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
    		ExceptionState& ec);
    void enqueueWriteBufferRect(
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
    		ExceptionState& ec){ enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, hostPtr, nullptr, nullptr, ec);};
    void enqueueWriteBufferRect(
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
    		ExceptionState& ec){ enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, hostPtr, eventWaitList, nullptr, ec);};

    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
    		unsigned hostRowPitch,
    		DOMArrayBufferView* hostPtr,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
    		unsigned hostRowPitch,
    		DOMArrayBufferView* hostPtr,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, origin, region, hostRowPitch, hostPtr, nullptr, nullptr, ec);};
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
    		unsigned hostRowPitch,
    		DOMArrayBufferView* hostPtr,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, origin, region, hostRowPitch, hostPtr, eventWaitList, nullptr, ec);};

    void enqueueNDRangeKernel(
    		WebCLKernel* kernel,
			unsigned workDim,
			Nullable<Vector<double>> globalWorkOffset,
			Vector<double> globalWorkSize,
			Nullable<Vector<double>> localWorkSize,
			Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
			WebCLEvent* event,
			ExceptionState& ec);
    void enqueueNDRangeKernel(
    		WebCLKernel* kernel,
			unsigned workDim,
			Nullable<Vector<double>> globalWorkOffset,
			Vector<double> globalWorkSize,
			ExceptionState& ec) { enqueueNDRangeKernel(kernel, workDim, globalWorkOffset, globalWorkSize, nullptr, nullptr, nullptr, ec); };
    void enqueueNDRangeKernel(
    		WebCLKernel* kernel,
			unsigned workDim,
			Nullable<Vector<double>> globalWorkOffset,
			Vector<double> globalWorkSize,
			Nullable<Vector<double>> localWorkSize,
			ExceptionState& ec) { enqueueNDRangeKernel(kernel, workDim, globalWorkOffset, globalWorkSize, localWorkSize, nullptr, nullptr, ec); };
    void enqueueNDRangeKernel(
    		WebCLKernel* kernel,
			unsigned workDim,
			Nullable<Vector<double>> globalWorkOffset,
			Vector<double> globalWorkSize,
			Nullable<Vector<double>> localWorkSize,
			Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
			ExceptionState& ec) { enqueueNDRangeKernel(kernel, workDim, globalWorkOffset, globalWorkSize, localWorkSize, eventWaitList, nullptr, ec); };

    //!! auto generated !!//

    void enqueueMarker(WebCLEvent* event, ExceptionState& ec);
    void enqueueBarrier(ExceptionState& ec);
    void enqueueWaitForEvents(HeapVector<Member<WebCLEvent>> eventWaitList, ExceptionState& ec);
    void finish(WebCLCallback* whenFinished, ExceptionState& ec);
    void finish(ExceptionState& ec) { finish(nullptr, ec); };
    void flush(ExceptionState& ec);

    ScriptValue getInfo(ScriptState*, int name, ExceptionState& ec);
    void release(ExceptionState& ec);

    //WEBCL_html_video extension
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
			HTMLVideoElement* srcVideo,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
			HTMLVideoElement* srcVideo,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, srcVideo, nullptr, nullptr, ec);};
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
			HTMLVideoElement* srcVideo,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, srcVideo, eventWaitList, nullptr, ec);};

    //WEBCL_html_video extension
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
    		ImageData* srcPixels,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
    		ImageData* srcPixels,
    		ExceptionState& ec){ enqueueWriteBuffer(buffer, blockingWrite, bufferOffset, srcPixels, nullptr, nullptr, ec); };
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
    		ImageData* srcPixels,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteBuffer(buffer, blockingWrite, bufferOffset, srcPixels, eventWaitList, nullptr, ec); };

    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
    		HTMLCanvasElement* srcCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
			HTMLCanvasElement* srcCanvas,
    		ExceptionState& ec){ enqueueWriteBuffer(buffer, blockingWrite, bufferOffset, srcCanvas, nullptr, nullptr, ec); };
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
			HTMLCanvasElement* srcCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteBuffer(buffer, blockingWrite, bufferOffset, srcCanvas, eventWaitList, nullptr, ec); };

    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
    		HTMLImageElement* srcImage,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
			HTMLImageElement* srcImage,
    		ExceptionState& ec){ enqueueWriteBuffer(buffer, blockingWrite, bufferOffset, srcImage, nullptr, nullptr, ec); };
    void enqueueWriteBuffer(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
			HTMLImageElement* srcImage,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteBuffer(buffer, blockingWrite, bufferOffset, srcImage, eventWaitList, nullptr, ec); };

    void enqueueWriteBufferRect(
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
    		ExceptionState& ec);
    void enqueueWriteBufferRect(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		Vector<unsigned> bufferOrigin,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> region,
    		unsigned bufferRowPitch,
    		unsigned bufferSlicePitch,
			ImageData* srcPixels,
    		ExceptionState& ec){ enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, srcPixels, nullptr, nullptr, ec);};
    void enqueueWriteBufferRect(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		Vector<unsigned> bufferOrigin,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> region,
    		unsigned bufferRowPitch,
    		unsigned bufferSlicePitch,
			ImageData* srcPixels,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, srcPixels, eventWaitList, nullptr, ec);};

    void enqueueWriteBufferRect(
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
    		ExceptionState& ec);
    void enqueueWriteBufferRect(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		Vector<unsigned> bufferOrigin,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> region,
    		unsigned bufferRowPitch,
    		unsigned bufferSlicePitch,
			HTMLCanvasElement* srcCanvas,
    		ExceptionState& ec){ enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, srcCanvas, nullptr, nullptr, ec);};
    void enqueueWriteBufferRect(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		Vector<unsigned> bufferOrigin,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> region,
    		unsigned bufferRowPitch,
    		unsigned bufferSlicePitch,
			HTMLCanvasElement* srcCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, srcCanvas, eventWaitList, nullptr, ec);};

    void enqueueWriteBufferRect(
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
    		ExceptionState& ec);
    void enqueueWriteBufferRect(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		Vector<unsigned> bufferOrigin,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> region,
    		unsigned bufferRowPitch,
    		unsigned bufferSlicePitch,
			HTMLImageElement* srcImage,
    		ExceptionState& ec){ enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, srcImage, nullptr, nullptr, ec);};
    void enqueueWriteBufferRect(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		Vector<unsigned> bufferOrigin,
    		Vector<unsigned> srcOrigin,
    		Vector<unsigned> region,
    		unsigned bufferRowPitch,
    		unsigned bufferSlicePitch,
			HTMLImageElement* srcImage,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, srcOrigin, region, bufferRowPitch, bufferSlicePitch, srcImage, eventWaitList, nullptr, ec);};

    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			ImageData* srcPixels,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			ImageData* srcPixels,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, origin, region, srcPixels, nullptr, nullptr, ec); };
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			ImageData* srcPixels,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, origin, region, srcPixels, eventWaitList, nullptr, ec); };

    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLCanvasElement* srcCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLCanvasElement* srcCanvas,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, origin, region, srcCanvas, nullptr, nullptr, ec); };
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLCanvasElement* srcCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, origin, region, srcCanvas, eventWaitList, nullptr, ec); };

    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLImageElement* srcImage,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLImageElement* srcImage,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, origin, region, srcImage, nullptr, nullptr, ec); };
    void enqueueWriteImage(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLImageElement* srcImage,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueWriteImage(image, blockingWrite, origin, region, srcImage, eventWaitList, nullptr, ec); };

    void enqueueReadBuffer(
    		WebCLBuffer* buffer,
    		bool blockingRead,
    		unsigned bufferOffset,
    		unsigned numBytes,
			HTMLCanvasElement* dstCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueReadBuffer(
    		WebCLBuffer* buffer,
    		bool blockingRead,
    		unsigned bufferOffset,
    		unsigned numBytes,
			HTMLCanvasElement* dstCanvas,
    		ExceptionState& ec){ enqueueReadBuffer(buffer, blockingRead, bufferOffset, numBytes, dstCanvas, nullptr, nullptr, ec);};
    void enqueueReadBuffer(
    		WebCLBuffer* buffer,
    		bool blockingRead,
    		unsigned bufferOffset,
    		unsigned numBytes,
			HTMLCanvasElement* dstCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueReadBuffer(buffer, blockingRead, bufferOffset, numBytes, dstCanvas, eventWaitList, nullptr, ec);};

    void enqueueReadBufferRect(
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
    		ExceptionState& ec);
    void enqueueReadBufferRect(
    		WebCLBuffer* buffer,
    		bool blockingRead,
    		Vector<unsigned> bufferOrigin,
    		Vector<unsigned> dstOrigin,
    		Vector<unsigned> region,
    		unsigned bufferRowPitch,
    		unsigned bufferSlicePitch,
			HTMLCanvasElement* dstCanvas,
    		ExceptionState& ec){ enqueueReadBufferRect(buffer, blockingRead, bufferOrigin, dstOrigin, region, bufferRowPitch, bufferSlicePitch, dstCanvas, nullptr, nullptr, ec);};
    void enqueueReadBufferRect(
    		WebCLBuffer* buffer,
    		bool blockingRead,
    		Vector<unsigned> bufferOrigin,
    		Vector<unsigned> dstOrigin,
    		Vector<unsigned> region,
    		unsigned bufferRowPitch,
    		unsigned bufferSlicePitch,
			HTMLCanvasElement* dstCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueReadBufferRect(buffer, blockingRead, bufferOrigin, dstOrigin, region, bufferRowPitch, bufferSlicePitch, dstCanvas, eventWaitList, nullptr, ec);};

    void enqueueReadImage(
    		WebCLImage* image,
    		bool blockingRead,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLCanvasElement* dstCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueReadImage(
    		WebCLImage* image,
    		bool blockingRead,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLCanvasElement* dstCanvas,
    		ExceptionState& ec){ enqueueReadImage(image, blockingRead, origin, region, dstCanvas, nullptr, nullptr, ec);};
    void enqueueReadImage(
    		WebCLImage* image,
    		bool blockingRead,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			HTMLCanvasElement* dstCanvas,
    		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
    		ExceptionState& ec){ enqueueReadImage(image, blockingRead, origin, region, dstCanvas, eventWaitList, nullptr, ec);};

    /*
	 * Internal API Define
	 */
    cl_command_queue getClComandQueue() { return mClCommandQueue; };
    void setEnableExtensionList(Vector<WTF::String> extensionList) { mEnabledExtensionList = extensionList; };

    unsigned int getQueueProperties(ExceptionState&);

	//cl_gl_sharing
	void enqueueAcquireGLObjects(
		Vector<ScriptValue>  value,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec);
	void enqueueAcquireGLObjects(
		Vector<ScriptValue>  value,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		ExceptionState& ec) { enqueueAcquireGLObjects(value, eventWaitList, nullptr, ec);};
	void enqueueAcquireGLObjects(
		Vector<ScriptValue>  value,
		ExceptionState& ec) { enqueueAcquireGLObjects(value, nullptr, nullptr, ec);};

	void enqueueReleaseGLObjects(
		Vector<ScriptValue>  value,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		WebCLEvent* event,
		ExceptionState& ec);
	void enqueueReleaseGLObjects(
		Vector<ScriptValue>  value,
		Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList,
		ExceptionState& ec) { enqueueReleaseGLObjects(value, eventWaitList, nullptr, ec);};
	void enqueueReleaseGLObjects(
		Vector<ScriptValue>  value,
		ExceptionState& ec) { enqueueReleaseGLObjects(value, nullptr, nullptr, ec);};

    DECLARE_TRACE();

private:
    WebCLCommandQueue(WebCL* context, cl_context clContext, cl_command_queue commandQueue);

    bool handleCmdQueueAndCLContext(ExceptionState& ec);

    bool handleMemArgs(WebCLMemoryObject* object1, ExceptionState& ec);
    bool handleMemArgs(WebCLMemoryObject* object1, WebCLMemoryObject* object2, ExceptionState& ec);

    bool handleImageArgs(WebCLImage* image1, Vector<unsigned> img1Origin, Vector<unsigned> region, ExceptionState& ec);
    bool handleImageArgs(WebCLImage* image1, Vector<unsigned> img1Origin, WebCLImage* image2, Vector<unsigned> img2Origin, Vector<unsigned> region, ExceptionState& ec);

    bool handleEventArg(WebCLEvent* event, ExceptionState& ec);

    cl_point* handleWaitEventListArgs(Nullable<HeapVector<Member<WebCLEvent>>> argEventNullable, int& listSize, ExceptionState& ec);
    cl_point* handleWaitEventListArgs(Nullable<HeapVector<Member<WebCLEvent>>> argEventNullable, int& listSize, bool blockingMode, ExceptionState& ec);

    bool handleCreatedEvent(cl_event createdEvent, WebCLEvent* event);

    bool handleSizeOfBuffer(WebCLBuffer* buffer, size_t offset, size_t size, ExceptionState& ec);

    bool handleKernelArg(WebCLKernel* kernel, ExceptionState& ec);

    bool handleNativeErrorCode(cl_int errcode, ExceptionState& ec);

    bool isValidRegionForMemoryObject(const Vector<size_t>& origin, const Vector<size_t>& region, size_t rowPitch, size_t slicePitch, size_t length);
    bool isEnabledExtension(WTF::String extensionName);

    void enqueueWriteBufferCommonForExtension(
    		WebCLBuffer* buffer,
    		bool blockingWrite,
    		unsigned bufferOffset,
			Vector<unsigned char> rawData,
			size_t rawDataSize,
			cl_point* eventList,
			size_t eventListSize,
			WebCLEvent* event,
    		ExceptionState& ec);
    void enqueueWriteBufferRectCommonForExtension(
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
    		ExceptionState& ec);
    void enqueueWriteImageCommonForExtension(
    		WebCLImage* image,
    		bool blockingWrite,
    		Vector<unsigned> origin,
    		Vector<unsigned> region,
			Vector<unsigned char> rawData,
	 		size_t rawDataSize,
			cl_point* eventList,
			size_t eventListSize,
    		WebCLEvent* event,
    		ExceptionState& ec);

    WebCLDevice* getQueueDevice(ExceptionState&);

    Member<WebCL> mContext;
    Member<WebCLContext> mClContext;
    Member<WebCLDevice> mRelatedDevice;
    Member<WebCLEvent> mFinishEvent;
    cl_context mNativeClContext;
    cl_command_queue mClCommandQueue;
    HashMap<cl_obj_key, void*> mTempMap;
    WebCLHTMLUtil* mHTMLUtil;
    Vector<WTF::String> mEnabledExtensionList;
};

} // namespace blink

#endif // WebCLCommandQueue_h
