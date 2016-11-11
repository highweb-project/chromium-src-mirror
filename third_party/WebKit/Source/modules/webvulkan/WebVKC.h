// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebVKC_H
#define WebVKC_H

#include "core/events/EventTarget.h"
#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "WebVKCInclude.h"

extern gpu::GpuChannelHost* webvkc_channel_;

namespace blink {

class ExecutionContext;

class WebVKCOperationHandler;
class WebVKCDevice;

class WebVKC : public GarbageCollectedFinalized<WebVKC>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static WebVKC* create(ExecutionContext* context)
    {
        WebVKC* webVKC = new WebVKC(context);
        return webVKC;
    }
    virtual ~WebVKC();

    /*
     * Constants Defines
     */
    enum {
        /* VkResult Code */
        VK_SUCCESS = 0,
        VK_NOT_READY = 1,
        VK_TIMEOUT = 2,
        VK_EVENT_SET = 3,
        VK_EVENT_RESET = 4,
        VK_INCOMPLETE = 5,
        VK_ERROR_OUT_OF_HOST_MEMORY = -1,
        VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
        VK_ERROR_INITIALIZATION_FAILED = -3,
        VK_ERROR_DEVICE_LOST = -4,
        VK_ERROR_MEMORY_MAP_FAILED = -5,
        VK_ERROR_LAYER_NOT_PRESENT = -6,
        VK_ERROR_EXTENSION_NOT_PRESENT = -7,
        VK_ERROR_FEATURE_NOT_PRESENT = -8,
        VK_ERROR_INCOMPATIBLE_DRIVER = -9,
        VK_ERROR_TOO_MANY_OBJECTS = -10,
        VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
        VK_ERROR_SURFACE_LOST_KHR = -1000000000,
        VK_ERROR_NATIVE_WINDOW_IN_USE_KHR = -1000000001,
        VK_SUBOPTIMAL_KHR = 1000001003,
        VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
        VK_ERROR_INCOMPATIBLE_DISPLAY_KHR = -1000003001,
        VK_ERROR_VALIDATION_FAILED_EXT = -1000011001,
        VK_RESULT_BEGIN_RANGE = VK_ERROR_FORMAT_NOT_SUPPORTED,
        VK_RESULT_END_RANGE = VK_INCOMPLETE,
        VK_RESULT_RANGE_SIZE = (VK_INCOMPLETE - VK_ERROR_FORMAT_NOT_SUPPORTED + 1),
        VK_RESULT_MAX_ENUM = 0x7FFFFFFF,

        /* VKPhysicalDeviceType */
        VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
        VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
        VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
        VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
        VK_PHYSICAL_DEVICE_TYPE_BEGIN_RANGE = VK_PHYSICAL_DEVICE_TYPE_OTHER,
        VK_PHYSICAL_DEVICE_TYPE_END_RANGE = VK_PHYSICAL_DEVICE_TYPE_CPU,
        VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE = (VK_PHYSICAL_DEVICE_TYPE_CPU - VK_PHYSICAL_DEVICE_TYPE_OTHER + 1),
        VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM = 0x7FFFFFFF,

        /* WebVKDevice getInfo name */
        VKC_apiVersion = 0,
        VKC_driverVersion = 1,
        VKC_vendorID = 2,
        VKC_deviceID = 3,
        VKC_deviceType = 4,
        VKC_deviceName = 5,
        VKC_maxMemoryAllocationCount = 6,
        VKC_maxComputeWorkGroupCount = 7,
        VKC_maxComputeWorkGroupInvocations = 8,
        VKC_maxComputeWorkGroupSize = 9,
    };

    /*
     * JS API Defines
     */
    void initialize(ExceptionState& ec);
    bool isInitialized() {return initialized;};
    unsigned getPhysicalDeviceCount(ExceptionState& ec);
    HeapVector<Member<WebVKCDevice>> getDevices(ExceptionState& ec);
    Member<WebVKCDevice> createDevice(VKCuint vdIndex, ExceptionState& ec);
    void releaseAll(ExceptionState& ec);

    void startHandling();
    void setOperationParameter(WebVKC_Operation_Base* paramPtr);
    void setOperationData(void* dataPtr, size_t sizeInBytes);
    void sendOperationSignal(int operation);
    void getOperationResult(WebVKC_Result_Base* resultPtr);
    void getOperationResultData(void* resultDataPtr, size_t sizeInBytes);

    void removeDevice(Member<WebVKCDevice> device);

    DECLARE_TRACE();

private:
    WebVKC(ExecutionContext* context);

    OwnPtr<WebVKCOperationHandler> mOperationHandler;

    HeapVector<Member<WebVKCDevice>> mDeviceList;

    bool initialized = false;
    bool releasing = false;
    VKCPoint vkcInstance = 0;
    VKCPoint mPhysicalDeviceList = 0;
    VKCuint mPhysicalDeviceCount = 0;
};

} // namespace blink

#endif
