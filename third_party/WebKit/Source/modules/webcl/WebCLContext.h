// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCLContext_H
#define WebCLContext_H

#include "platform/bindings/ScriptWrappable.h"
#include "bindings/core/v8/ExceptionState.h"
#include "platform/heap/Handle.h"
#include "core/typed_arrays/DOMArrayBufferView.h"
#include "core/typed_arrays/ArrayBufferViewHelpers.h"
#include "platform/wtf/text/WTFString.h"

#include "WebCLInclude.h"
#include "WebCLCallbackHandler.h"
#include "WebCLHTMLUtil.h"
#include "WebCLMemoryUtil.h"

namespace blink {

class WebCL;
class WebCLBuffer;
class WebCLSampler;
class WebCLImage;
class WebCLDevice;
class WebCLCommandQueue;
class WebCLImageDescriptor;
class WebCLEvent;
class WebCLUserEvent;
class WebCLMemoryObject;
class WebCLProgram;

class HTMLVideoElement;
class WebCLKernel;
class ImageData;
class HTMLCanvasElement;
class HTMLImageElement;
class WebGLRenderingContext;
class WebGLBuffer;
class WebGLRenderbuffer;
class WebGLTexture;
class WebGLSharedPlatform3DObject;

typedef HeapHashMap<cl_obj_key, Member<WebCLCommandQueue>> WebCLCommandQueueMap;
typedef HeapHashMap<cl_obj_key, Member<WebCLBuffer>> WebCLBufferMap;
typedef HeapHashMap<cl_obj_key, Member<WebCLUserEvent>> WebCLUserEventMap;
typedef HeapHashMap<cl_obj_key, Member<WebCLEvent>> WebCLEventMap;
typedef HeapHashMap<cl_obj_key, Member<WebCLMemoryObject>> WebCLMemObjMap;
typedef HeapHashMap<cl_obj_key, Member<WebCLProgram>> WebCLProgramMap;
typedef HeapHashMap<cl_obj_key, Member<WebCLKernel>> WebCLKernelMap;
typedef HeapHashMap<cl_obj_key, Member<WebCLSampler>> WebCLSamplerMap;
typedef HeapHashMap<cl_obj_key, Member<WebGLSharedPlatform3DObject>> WebGLBufferMap;

class WebCLContext : public GarbageCollectedFinalized<WebCLContext>, public ScriptWrappable, public WebCLCallbackHandler::CallbackListener {
    DEFINE_WRAPPERTYPEINFO();
public:
    static WebCLContext* create(WebCL* context, cl_context cl_context) {
    	return new WebCLContext(context, cl_context);
    }
    virtual ~WebCLContext();

    /*
     * JS API define
     */
    WebCLBuffer* createBuffer(int memFlags, unsigned sizeInBytes, ExceptionState& ec);
    WebCLBuffer* createBuffer(int memFlags, unsigned sizeInBytes, NotShared<DOMArrayBufferView> hostPtr, ExceptionState& ec);
    WebCLBuffer* createBuffer(int memFlags, ImageData* srcPixels, ExceptionState& ec);
    WebCLBuffer* createBuffer(int memFlags, HTMLCanvasElement* srcCanvas, ExceptionState& ec);
    WebCLBuffer* createBuffer(int memFlags, HTMLImageElement* srcImage, ExceptionState& ec);

    WebCLImage* createImage(int memFlags, WebCLImageDescriptor descriptor, ExceptionState& ec);
    WebCLImage* createImage(int memFlags, WebCLImageDescriptor descriptor, NotShared<DOMArrayBufferView> hostPtr, ExceptionState& ec);
    WebCLImage* createImage(int memFlags, HTMLVideoElement* srcVideo, ExceptionState& ec);
    WebCLImage* createImage(int memFlags, ImageData* srcPixels, ExceptionState& ec);
    WebCLImage* createImage(int memFlags, HTMLCanvasElement* srcCanvas, ExceptionState& ec);
    WebCLImage* createImage(int memFlags, HTMLImageElement* srcImage, ExceptionState& ec);

    WebCLSampler* createSampler(CLboolean normalizedCoords, CLenum addressingMode, CLenum filterMode, ExceptionState& ec);

    WebCLUserEvent* createUserEvent(ExceptionState& ec);

    WebCLCommandQueue* createCommandQueue(ExceptionState& ec);
    WebCLCommandQueue* createCommandQueue(WebCLDevice* device, ExceptionState& ec);
    WebCLCommandQueue* createCommandQueue(WebCLDevice* device, int properties, ExceptionState& ec);

    HeapVector<WebCLImageDescriptor> getSupportedImageFormats(ExceptionState& ec);
    HeapVector<WebCLImageDescriptor> getSupportedImageFormats(unsigned memFlags, ExceptionState& ec);

    WebCLProgram* createProgram(const String& kernelSource, ExceptionState& ec);

    ScriptValue getInfo(ScriptState*, int name, ExceptionState& ec);

    void release(ExceptionState& ec);
    void releaseAll(ExceptionState& ec);

	/*
	 * Internal API Define
	 */
    cl_context getClContext() { return mClContext;}

    WebCLCommandQueue* findCLCommandQueue(cl_obj_key key);
    WebCLBuffer* findCLBuffer(cl_obj_key key);
    WebCLUserEvent* findCLUserEvent(cl_obj_key key);
    WebCLEvent* findCLEvent(cl_obj_key key);
    WebCLMemoryObject* findCLMemObj(cl_obj_key key);
    WebCLProgram* findCLProgram(cl_obj_key key);
    WebCLKernel* findCLKernel(cl_obj_key key);

#if defined(OS_ANDROID)
    void OnCallback(unsigned eventKey, unsigned callbackKey, unsigned objectType) override;
#elif defined(OS_LINUX)
    void OnCallback(cl_point eventKey, unsigned callbackKey, unsigned objectType) override;
#endif

    void addCLEvent(WebCLEvent* event);
    void addCLKernel(cl_obj_key key, WebCLKernel* clKernel);
    void addCLBuffer(cl_obj_key key, WebCLBuffer* clBuffer);
    void addCLProgram(cl_obj_key key, WebCLProgram* clProgram);

    void removeCLEvent(cl_obj_key key);
    void removeCLProgram(cl_obj_key key);
	void removeCLKernel(cl_obj_key key);
	void removeCLSampler(cl_obj_key key);
	void removeCLMemObj(cl_obj_key key);
	void removeCLCommandQueue(cl_obj_key key);

    void setEnableExtensionList(Vector<WTF::String> extensionList) { mEnabledExtensionList = extensionList; };

	Vector<cl_point> getClDevices(ExceptionState&);
	HeapVector<Member<WebCLDevice>> getDevices(ExceptionState&);

    void initializeContextDevice(ExceptionState&);

    // gl/cl sharing
    WebGLRenderingContext* getGLContext(ExceptionState&);
    WebCLBuffer* createFromGLBuffer(int memFlags, WebGLBuffer* glBuffer, ExceptionState& ec);
    WebCLImage* createFromGLRenderbuffer(int memFlags, WebGLRenderbuffer* renderbuffer, ExceptionState& ec);
    WebCLImage* createFromGLTexture(int memFlags, int textureTarget, int miplevel, WebGLTexture* texture, ExceptionState& ec);
    void removeGLBuffer(cl_obj_key key);

    WebGLSharedPlatform3DObject* findGLBuffer(cl_obj_key key);
    void setGLContext(WebGLRenderingContext* mGL);

    DECLARE_TRACE();
private:
    WebCLContext(WebCL* context, cl_context cl_context);

    void convertHostPtr(DOMArrayBufferView* arrayBufferView, void* convertedHostPtr, int* convertedSizeInBytes);
    bool setBufferInfo(WebCLBuffer* buffer, ExceptionState& ec);
    bool setMaxImageSize(ExceptionState& ec);
    bool setMaxMemAllocSize(ExceptionState& ec);
    bool isEnabledExtension(WTF::String extensionName);

    WebCLMemoryObject* createExtensionMemoryObjectBase(int memFlags, void* data, size_t dataSize, int objectType, ExceptionState& ec);
    WebCLMemoryObject* createExtensionMemoryObjectBase(int memFlags, void* data, size_t dataSize, int objectType, size_t width, size_t height, ExceptionState& ec);

    bool isReleasing = false;

    Member<WebCL> mContext;
    cl_context mClContext;
    cl_uint mNumDevices;
    HeapVector<Member<WebCLDevice>> mDevices;
    size_t mImage2dMaxWidth;
    size_t mImage2dMaxHeight;
    unsigned long mMaxMemAllocSize;

    WebCLHTMLUtil* mHTMLUtil;
    Member<WebCLMemoryUtil> mMemoryUtil;
    WebCLCallbackHandler* mCallbackHandler;

    WebCLCommandQueueMap mCommandQueueMap;
    WebCLEventMap mEventMap;
    WebCLMemObjMap mMemObjMap;
    WebCLProgramMap mProgramMap;
    WebCLKernelMap mKernelMap;
    WebCLSamplerMap mSamplerMap;

    //cl/gl sharing
    WebGLBufferMap mGlBufferMap;
    Member<WebGLRenderingContext> mGLContext;

    Vector<WTF::String> mEnabledExtensionList;
};

}

#endif
