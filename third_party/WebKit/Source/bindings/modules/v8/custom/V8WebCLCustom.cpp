// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "bindings/modules/v8/V8WebCL.h"
#include "bindings/modules/v8/V8WebCLPlatform.h"
#include "bindings/modules/v8/V8WebCLDevice.h"
#include "bindings/modules/v8/V8WebGLRenderingContext.h"
#include "bindings/core/v8/IDLTypes.h"
#include "bindings/core/v8/NativeValueTraitsImpl.h"

#include "modules/webcl/WebCL.h"
#include "modules/webcl/WebCLContext.h"
#include "base/logging.h"

namespace blink {

static void createContext1Method(const v8::FunctionCallbackInfo<v8::Value>& info, bool hasGL)
{
    CLLOG(INFO) << "V8WebCLCustom, createContext1Method, hasGL : " << hasGL;

    ExceptionState exceptionState(info.GetIsolate(), ExceptionState::kExecutionContext, "WebCL", "createContext");
    WebCL* impl = V8WebCL::toImpl(info.Holder());
    WebCLContext* result = nullptr;

    if(hasGL) {
        WebGLRenderingContext* renderingContext;
        unsigned deviceType;
        {
            renderingContext = V8WebGLRenderingContext::toImplWithTypeCheck(info.GetIsolate(), info[0]);
            deviceType = ToUInt32(info.GetIsolate(), info[1], kNormalConversion, exceptionState);
            if (exceptionState.HadException()) {
    						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
    						return;
    				}
        }
        result = impl->createContext(renderingContext, deviceType, exceptionState);
        if (exceptionState.HadException()) {
						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
						return;
				}
    } else {
        unsigned deviceType;
        {
            deviceType = ToUInt32(info.GetIsolate(), info[0], kNormalConversion, exceptionState);
            if (exceptionState.HadException()) {
    						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
    						return;
    				}
        }
        result = impl->createContext(deviceType, exceptionState);
        if (exceptionState.HadException()) {
						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
						return;
				}
    }

    if(result) {
        v8::Local<v8::Object> obj = result->Wrap(info.GetIsolate(), info.Holder());
        V8SetReturnValue(info, obj);
    }
}

static void createContext2Method(const v8::FunctionCallbackInfo<v8::Value>& info, bool hasGL)
{
    CLLOG(INFO) << "V8WebCLCustom, createContext2Method, hasGL : " << hasGL;

    ExceptionState exceptionState(info.GetIsolate(), ExceptionState::kExecutionContext, "WebCL", "createContext");
    WebCL* impl = V8WebCL::toImpl(info.Holder());
    WebCLContext* result = nullptr;

    if(hasGL) {
        WebGLRenderingContext* renderingContext;
        WebCLPlatform* platform;
        unsigned deviceType;
        {
            renderingContext = V8WebGLRenderingContext::toImplWithTypeCheck(info.GetIsolate(), info[0]);
            platform = V8WebCLPlatform::toImplWithTypeCheck(info.GetIsolate(), info[1]);
            deviceType = ToUInt32(info.GetIsolate(), info[2], kNormalConversion, exceptionState);
            if (exceptionState.HadException()) {
    						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
    						return;
    				}
        }
        result = impl->createContext(renderingContext, platform, deviceType, exceptionState);
        if (exceptionState.HadException()) {
						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
						return;
				}
    } else {
        WebCLPlatform* platform;
        unsigned deviceType;
        {
            platform = V8WebCLPlatform::toImplWithTypeCheck(info.GetIsolate(), info[0]);
            deviceType = ToUInt32(info.GetIsolate(), info[1], kNormalConversion, exceptionState);
            if (exceptionState.HadException()) {
    						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
    						return;
    				};
        }
        result = impl->createContext(platform, deviceType, exceptionState);
        if (exceptionState.HadException()) {
						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
						return;
				}
    }

    if(result) {
        v8::Local<v8::Object> obj = result->Wrap(info.GetIsolate(), info.Holder());
        V8SetReturnValue(info, obj);
    }
}

static void createContext3Method(const v8::FunctionCallbackInfo<v8::Value>& info, bool hasGL)
{
    CLLOG(INFO) << "V8WebCLCustom, createContext3Method, hasGL : " << hasGL;

    ExceptionState exceptionState(info.GetIsolate(), ExceptionState::kExecutionContext, "WebCL", "createContext");
    WebCL* impl = V8WebCL::toImpl(info.Holder());
    WebCLContext* result = nullptr;

    if(hasGL) {
        WebGLRenderingContext* renderingContext;
        WebCLDevice* device;
        {
            renderingContext = V8WebGLRenderingContext::toImplWithTypeCheck(info.GetIsolate(), info[0]);
            device = V8WebCLDevice::toImplWithTypeCheck(info.GetIsolate(), info[1]);
        }
        result = impl->createContext(renderingContext, device, exceptionState);
        if (exceptionState.HadException()) {
						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
						return;
				}
    } else {
        WebCLDevice* device;
        {
            device = V8WebCLDevice::toImplWithTypeCheck(info.GetIsolate(), info[0]);
        }
        result = impl->createContext(device, exceptionState);
        if (exceptionState.HadException()) {
						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
						return;
				}
    }

    if(result) {
        v8::Local<v8::Object> obj = result->Wrap(info.GetIsolate(), info.Holder());
        V8SetReturnValue(info, obj);
    }
}

static void createContext4Method(const v8::FunctionCallbackInfo<v8::Value>& info, bool hasGL)
{
    CLLOG(INFO) << "V8WebCLCustom, createContext4Method, hasGL : " << hasGL;

    ExceptionState exceptionState(info.GetIsolate(), ExceptionState::kExecutionContext, "WebCL", "createContext");
    WebCL* impl = V8WebCL::toImpl(info.Holder());
    WebCLContext* result = nullptr;

    if(hasGL) {
        WebGLRenderingContext* renderingContext;
        HeapVector<Member<WebCLDevice>> device;
        {
            renderingContext = V8WebGLRenderingContext::toImplWithTypeCheck(info.GetIsolate(), info[0]);
            device = NativeValueTraits<IDLSequence<WebCLDevice>>::NativeValue(info.GetIsolate(), info[1], exceptionState);
            if (exceptionState.HadException()) {
    						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
    						return;
    				}
        }
        result = impl->createContext(renderingContext, device, exceptionState);
        if (exceptionState.HadException()) {
						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
						return;
				}
    } else {
        HeapVector<Member<WebCLDevice>> device;
        {
            device = NativeValueTraits<IDLSequence<WebCLDevice>>::NativeValue(info.GetIsolate(), info[0], exceptionState);
            if (exceptionState.HadException()) {
    						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
    						return;
    				}
        }
        result = impl->createContext(device, exceptionState);
        if (exceptionState.HadException()) {
						V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
						return;
				}
    }

    if(result) {
        v8::Local<v8::Object> obj = result->Wrap(info.GetIsolate(), info.Holder());
        V8SetReturnValue(info, obj);
    }
}

void V8WebCL::createContextMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(info.GetIsolate(), ExceptionState::kExecutionContext, "WebCL", "createContext");
    CLLOG(INFO) << "V8WebCLCustom, V8WebCL::createContextMethodCustom, info.Length() : " << info.Length();

    // gl/cl sharing
    if(info.Length() >= 1) {
        if(V8WebGLRenderingContext::hasInstance(info[0], info.GetIsolate())) {
            switch (std::min(3, info.Length())) {
            case 1:
                if (true) {
                    createContext1Method(info, true);
                    return;
                }
                break;
            case 2:
                if (info[1]->IsUndefined()) {
                    createContext1Method(info, true);
                    return;
                }
                if (V8WebCLPlatform::hasInstance(info[1], info.GetIsolate())) {
                    createContext2Method(info, true);
                    return;
                }
                if (V8WebCLDevice::hasInstance(info[1], info.GetIsolate())) {
                    createContext3Method(info, true);
                    return;
                }
                if (info[1]->IsArray()) {
                    createContext4Method(info, true);
                    return;
                }
                if (true) {
                    createContext1Method(info, true);
                    return;
                }
                break;
            case 3:
                if (true) {
                    createContext2Method(info, true);
                    return;
                }
                break;
            default:
                break;
            }
        }
    }

    switch (std::min(2, info.Length())) {
    case 0:
        if (true) {
            createContext1Method(info, false);
            return;
        }
        break;
    case 1:
        if (info[0]->IsUndefined()) {
            createContext1Method(info, false);
            return;
        }
        if (V8WebCLPlatform::hasInstance(info[0], info.GetIsolate())) {
            createContext2Method(info, false);
            return;
        }
        if (V8WebCLDevice::hasInstance(info[0], info.GetIsolate())) {
            createContext3Method(info, false);
            return;
        }
        if (info[0]->IsArray()) {
            createContext4Method(info, false);
            return;
        }
        if (true) {
            createContext1Method(info, false);
            return;
        }
        break;
    case 2:
        if (true) {
            createContext2Method(info, false);
            return;
        }
        break;
    default:
        break;
    }

    CLLOG(INFO) << "V8WebCLCustom, No function was found that matched the signature provided.";
    exceptionState.ThrowTypeError("No function was found that matched the signature provided.");
    V8ThrowException::ThrowException(info.GetIsolate(), exceptionState.GetException());
    return;
}

} // namespace blink
