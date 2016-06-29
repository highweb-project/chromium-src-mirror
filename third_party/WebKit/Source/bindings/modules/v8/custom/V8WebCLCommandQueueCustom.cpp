// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "bindings/modules/v8/V8WebCLCommandQueue.h"
#include "modules/webcl/WebCLInclude.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/V8ArrayBufferView.h"
#include "bindings/core/v8/V8DOMConfiguration.h"
#include "bindings/core/v8/V8HTMLCanvasElement.h"
#include "bindings/core/v8/V8HTMLVideoElement.h"
#include "bindings/core/v8/V8HTMLImageElement.h"
#include "bindings/core/v8/V8ImageData.h"
#include "bindings/core/v8/V8ObjectConstructor.h"
#include "bindings/modules/v8/V8WebCLBuffer.h"
#include "bindings/modules/v8/V8WebCLCallback.h"
#include "bindings/modules/v8/V8WebCLEvent.h"
#include "bindings/modules/v8/V8WebCLImage.h"
#include "bindings/modules/v8/V8WebCLKernel.h"

namespace blink {

//For basic API
static void enqueueWriteImage1Method(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	CLLOG(INFO) << "enqueueWriteImage1Method";

    ExceptionState exceptionState(ExceptionState::ExecutionContext, "enqueueWriteImage", "WebCLCommandQueue", info.Holder(), info.GetIsolate());
    WebCLCommandQueue* impl = V8WebCLCommandQueue::toImpl(info.Holder());

    WebCLImage* image;
    bool blockingWrite;
    Vector<unsigned> origin;
    Vector<unsigned> region;
    unsigned hostRowPitch;
    DOMArrayBufferView* hostPtr;
    Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList;
    WebCLEvent* event;
    {
    	image = V8WebCLImage::toImplWithTypeCheck(info.GetIsolate(), info[0]);
        if (!image) {
            exceptionState.throwTypeError("parameter 1 is not of type 'WebCLImage'.");
            exceptionState.throwIfNeeded();
            return;
        }
        blockingWrite = toBoolean(info.GetIsolate(), info[1], exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        origin = toImplArray<Vector<unsigned>>(info[2], 3, info.GetIsolate(), exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        region = toImplArray<Vector<unsigned>>(info[3], 4, info.GetIsolate(), exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        hostRowPitch = toUInt32(info.GetIsolate(), info[4], NormalConversion, exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        hostPtr = info[5]->IsArrayBufferView() ? V8ArrayBufferView::toImpl(v8::Local<v8::ArrayBufferView>::Cast(info[5])) : 0;
        if (!hostPtr) {
            exceptionState.throwTypeError("parameter 6 is not of type 'ArrayBufferView'.");
            exceptionState.throwIfNeeded();
            return;
        }

        if (UNLIKELY(info.Length() <= 6)) {
            impl->enqueueWriteImage(image, blockingWrite, origin, region, hostRowPitch, hostPtr, exceptionState);
            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        if (!isUndefinedOrNull(info[6])) {
            eventWaitList = (toMemberNativeArray<WebCLEvent>(info[6], 7, info.GetIsolate(), exceptionState));
            if (exceptionState.throwIfNeeded())
                return;
        }
        if (UNLIKELY(info.Length() <= 7)) {
            impl->enqueueWriteImage(image, blockingWrite, origin, region, hostRowPitch, hostPtr, eventWaitList, exceptionState);
            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        event = V8WebCLEvent::toImplWithTypeCheck(info.GetIsolate(), info[7]);
        if (!event && !isUndefinedOrNull(info[7])) {
            exceptionState.throwTypeError("parameter 8 is not of type 'WebCLEvent'.");
            exceptionState.throwIfNeeded();
            return;
        }
    }
    impl->enqueueWriteImage(image, blockingWrite, origin, region, hostRowPitch, hostPtr, eventWaitList, event, exceptionState);
    if (exceptionState.hadException()) {
        exceptionState.throwIfNeeded();
        return;
    }
}

//For WEBCL_html_video API
static void enqueueWriteImage2Method(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	CLLOG(INFO) << "enqueueWriteImage2Method";

    ExceptionState exceptionState(ExceptionState::ExecutionContext, "enqueueWriteImage", "WebCLCommandQueue", info.Holder(), info.GetIsolate());
    WebCLCommandQueue* impl = V8WebCLCommandQueue::toImpl(info.Holder());
    WebCLImage* image;
    bool blockingWrite;
    HTMLVideoElement* srcVideo;
    Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList;
    WebCLEvent* event;
    {
    	image = V8WebCLImage::toImplWithTypeCheck(info.GetIsolate(), info[0]);
        if (!image) {
            exceptionState.throwTypeError("parameter 1 is not of type 'WebCLImage'.");
            exceptionState.throwIfNeeded();
            return;
        }
        blockingWrite = toBoolean(info.GetIsolate(), info[1], exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        srcVideo = V8HTMLVideoElement::toImplWithTypeCheck(info.GetIsolate(), info[2]);
        if (!srcVideo) {
            exceptionState.throwTypeError("parameter 8 is not of type 'HTMLVideoElement'.");
            exceptionState.throwIfNeeded();
            return;
        }

        if (UNLIKELY(info.Length() <= 3)) {
            impl->enqueueWriteImage(image, blockingWrite, srcVideo, exceptionState);
            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        if (!isUndefinedOrNull(info[3])) {
            eventWaitList = (toMemberNativeArray<WebCLEvent>(info[3], 4, info.GetIsolate(), exceptionState));
            if (exceptionState.throwIfNeeded())
                return;
        }
        if (UNLIKELY(info.Length() <= 4)) {
            impl->enqueueWriteImage(image, blockingWrite, srcVideo, eventWaitList, exceptionState);
            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        event = V8WebCLEvent::toImplWithTypeCheck(info.GetIsolate(), info[4]);
        if (!event && !isUndefinedOrNull(info[4])) {
            exceptionState.throwTypeError("parameter 7 is not of type 'WebCLEvent'.");
            exceptionState.throwIfNeeded();
            return;
        }
    }
    impl->enqueueWriteImage(image, blockingWrite, srcVideo, eventWaitList, event, exceptionState);
    if (exceptionState.hadException()) {
        exceptionState.throwIfNeeded();
        return;
    }
}

//For WEBCL_html_image API
static void enqueueWriteImage3Method(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	CLLOG(INFO) << "enqueueWriteImage3Method";

    ExceptionState exceptionState(ExceptionState::ExecutionContext, "enqueueWriteImage", "WebCLCommandQueue", info.Holder(), info.GetIsolate());
    WebCLCommandQueue* impl = V8WebCLCommandQueue::toImpl((info.Holder()));

    WebCLImage* image;
    bool blockingWrite;
    Vector<unsigned> origin;
    Vector<unsigned> region;
    ImageData* srcPixels = 0;
    HTMLCanvasElement* srcCanvas = 0;
    HTMLImageElement* srcImage = 0;
    Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList;
    WebCLEvent* event;
    {
    	image = V8WebCLImage::toImplWithTypeCheck(info.GetIsolate(), info[0]);
        if (!image) {
            exceptionState.throwTypeError("parameter 1 is not of type 'WebCLImage'.");
            exceptionState.throwIfNeeded();
            return;
        }
        blockingWrite = toBoolean(info.GetIsolate(), info[1], exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        origin = toImplArray<Vector<unsigned>>(info[2], 3, info.GetIsolate(), exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        region = toImplArray<Vector<unsigned>>(info[3], 4, info.GetIsolate(), exceptionState);
        if (exceptionState.throwIfNeeded())
            return;

        if (V8ImageData::hasInstance(info[4], info.GetIsolate())) {
        	srcPixels = V8ImageData::toImplWithTypeCheck(info.GetIsolate(), info[4]);
        }
        else if (V8HTMLCanvasElement::hasInstance(info[4], info.GetIsolate())) {
        	srcCanvas = V8HTMLCanvasElement::toImplWithTypeCheck(info.GetIsolate(), info[4]);
        }
        else if (V8HTMLImageElement::hasInstance(info[4], info.GetIsolate())) {
        	srcImage = V8HTMLImageElement::toImplWithTypeCheck(info.GetIsolate(), info[4]);
        }
        else {
        	exceptionState.throwTypeError("parameter 5 is not of type 'ImageData' or 'HMTLCanvasElement' or 'HTMLImageElement'.");
			exceptionState.throwIfNeeded();
			return;
        }


        if (UNLIKELY(info.Length() <= 5)) {
        	if(srcPixels)
        		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcPixels, exceptionState);
        	else if(srcCanvas)
        		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcCanvas, exceptionState);
        	else if(srcImage)
        		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcImage, exceptionState);
			else {
				exceptionState.throwTypeError("parameter 5 is not of type 'ImageData' or 'HMTLCanvasElement' or 'HTMLImageElement'.");
				exceptionState.throwIfNeeded();
				return;
			}

            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        if (!isUndefinedOrNull(info[5])) {
            eventWaitList = (toMemberNativeArray<WebCLEvent>(info[5], 6, info.GetIsolate(), exceptionState));
            if (exceptionState.throwIfNeeded())
                return;
        }
        if (UNLIKELY(info.Length() <= 6)) {
        	if(srcPixels)
        		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcPixels, eventWaitList, exceptionState);
        	else if(srcCanvas)
        		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcCanvas, eventWaitList, exceptionState);
        	else if(srcImage)
        		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcImage, eventWaitList, exceptionState);
			else {
				exceptionState.throwTypeError("parameter 5 is not of type 'ImageData' or 'HMTLCanvasElement' or 'HTMLImageElement'.");
				exceptionState.throwIfNeeded();
				return;
			}

            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        event = V8WebCLEvent::toImplWithTypeCheck(info.GetIsolate(), info[6]);
        if (!event && !isUndefinedOrNull(info[6])) {
            exceptionState.throwTypeError("parameter 7 is not of type 'WebCLEvent'.");
            exceptionState.throwIfNeeded();
            return;
        }
    }

	if(srcPixels)
		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcPixels, eventWaitList, event, exceptionState);
	else if(srcCanvas)
		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcCanvas, eventWaitList, event, exceptionState);
	else if(srcImage)
		impl->enqueueWriteImage(image, blockingWrite, origin, region, srcImage, eventWaitList, event, exceptionState);
	else {
		exceptionState.throwTypeError("parameter 5 is not of type 'ImageData' or 'HMTLCanvasElement' or 'HTMLImageElement'.");
		exceptionState.throwIfNeeded();
		return;
	}

    if (exceptionState.hadException()) {
        exceptionState.throwIfNeeded();
        return;
    }
    return;
}

void V8WebCLCommandQueue::enqueueWriteImageMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ExecutionContext, "enqueueWriteImage", "WebCLCommandQueue", info.Holder(), info.GetIsolate());

    CLLOG(INFO) << "info length=" << info.Length();

    switch (std::min(8, info.Length())) {
    case 3:
    case 4:
        if (true) {
        	enqueueWriteImage2Method(info);
            return;
        }
        break;
    case 5:
        if (V8ImageData::hasInstance(info[4], info.GetIsolate())) {
        	enqueueWriteImage3Method(info);
            return;
        }
        else if (V8HTMLCanvasElement::hasInstance(info[4], info.GetIsolate())) {
        	enqueueWriteImage3Method(info);
            return;
        }
        else if (V8HTMLImageElement::hasInstance(info[4], info.GetIsolate())) {
        	enqueueWriteImage3Method(info);
            return;
        }
        else {
        	enqueueWriteImage2Method(info);
            return;
        }
        break;
    case 6:
        if (isUndefinedOrNull(info[5]) || !v8::Local<v8::ArrayBufferView>::Cast(info[5])->IsArrayBufferView()) {
        	enqueueWriteImage3Method(info);
            return;
        }
        else {
        	enqueueWriteImage1Method(info);
            return;
        }
        break;
    case 7:
    	if (isUndefinedOrNull(info[6])) {
        	if (isUndefinedOrNull(info[5]) || !v8::Local<v8::ArrayBufferView>::Cast(info[5])->IsArrayBufferView()) {
            	enqueueWriteImage3Method(info);
                return;
            }
            else {
            	enqueueWriteImage1Method(info);
                return;
            }
    	}
    	else {
    		if(V8WebCLEvent::toImplWithTypeCheck(info.GetIsolate(), info[6])) {
    			enqueueWriteImage3Method(info);
				return;
    		}
    		else {
    			enqueueWriteImage1Method(info);
				return;
    		}
    	}
        break;
    case 8:
        if (true) {
        	enqueueWriteImage1Method(info);
            return;
        }
        break;
    default:
        break;
    }
    if (info.Length() < 3) {
        exceptionState.throwTypeError(ExceptionMessages::notEnoughArguments(3, info.Length()));
        exceptionState.throwIfNeeded();
        return;
    }
    exceptionState.throwTypeError("No function was found that matched the signature provided.");
    exceptionState.throwIfNeeded();
    return;
}

void V8WebCLCommandQueue::enqueueNDRangeKernelMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ExecutionContext, "enqueueNDRangeKernel", "WebCLCommandQueue", info.Holder(), info.GetIsolate());
    if (UNLIKELY(info.Length() < 4)) {
        setMinimumArityTypeError(exceptionState, 4, info.Length());
        exceptionState.throwIfNeeded();
        return;
    }
    WebCLCommandQueue* impl = V8WebCLCommandQueue::toImpl(info.Holder());
    WebCLKernel* kernel;
    unsigned workDim;
    Nullable<Vector<double>> globalWorkOffset;
    Vector<double> globalWorkSize;
    Nullable<Vector<double>> localWorkSize;
    Nullable<HeapVector<Member<WebCLEvent>>> eventWaitList;
    WebCLEvent* event;
    {
        kernel = V8WebCLKernel::toImplWithTypeCheck(info.GetIsolate(), info[0]);
        workDim = toUInt32(info.GetIsolate(), info[1], NormalConversion, exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        if (!isUndefinedOrNull(info[2])) {
            globalWorkOffset = toImplArray<Vector<double>>(info[2], 3, info.GetIsolate(), exceptionState);
            if (exceptionState.throwIfNeeded())
                return;
        }
        globalWorkSize = toImplArray<Vector<double>>(info[3], 4, info.GetIsolate(), exceptionState);
        if (exceptionState.throwIfNeeded())
            return;
        if (UNLIKELY(info.Length() <= 4)) {
            impl->enqueueNDRangeKernel(kernel, workDim, globalWorkOffset, globalWorkSize, exceptionState);
            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        if (!isUndefinedOrNull(info[4])) {
            localWorkSize = toImplArray<Vector<double>>(info[4], 5, info.GetIsolate(), exceptionState);
            if (exceptionState.throwIfNeeded())
                return;
        }
        if (UNLIKELY(info.Length() <= 5)) {
            impl->enqueueNDRangeKernel(kernel, workDim, globalWorkOffset, globalWorkSize, localWorkSize, exceptionState);
            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        if (!isUndefinedOrNull(info[5])) {
            eventWaitList = (toMemberNativeArray<WebCLEvent>(info[5], 6, info.GetIsolate(), exceptionState));
            if (exceptionState.throwIfNeeded())
                return;
        }
        if (UNLIKELY(info.Length() <= 6)) {
            impl->enqueueNDRangeKernel(kernel, workDim, globalWorkOffset, globalWorkSize, localWorkSize, eventWaitList, exceptionState);
            if (exceptionState.hadException()) {
                exceptionState.throwIfNeeded();
                return;
            }
            return;
        }
        event = V8WebCLEvent::toImplWithTypeCheck(info.GetIsolate(), info[6]);
    }
    impl->enqueueNDRangeKernel(kernel, workDim, globalWorkOffset, globalWorkSize, localWorkSize, eventWaitList, event, exceptionState);
    if (exceptionState.hadException()) {
        exceptionState.throwIfNeeded();
        return;
    }
}

} // namespace blink
