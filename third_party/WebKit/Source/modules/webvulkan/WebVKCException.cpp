// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "WebVKCException.h"

namespace blink {

// This should be an array of structs to pair the names and descriptions. ??
static const char* const exceptionNames[] = {
    "VK_NOT_READY",
    "VK_TIMEOUT",
    "VK_ERROR_OUT_OF_HOST_MEMORY",
    "VK_ERROR_OUT_OF_DEVICE_MEMORY",
    "VK_ERROR_INITIALIZATION_FAILED",
    "VK_ERROR_DEVICE_LOST",
    "VK_ERROR_MEMORY_MAP_FAILED",
    "VK_ERROR_LAYER_NOT_PRESENT",
    "VK_ERROR_EXTENSION_NOT_PRESENT",
    "VK_ERROR_FEATURE_NOT_PRESENT",
    "VK_ERROR_INCOMPATIBLE_DRIVER",
    "VK_ERROR_TOO_MANY_OBJECTS",
    "VK_ERROR_FORMAT_NOT_SUPPORTED",
    "VK_ERROR_SURFACE_LOST_KHR",
    "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR",
    "VK_SUBOPTIMAL_KHR",
    "VK_ERROR_OUT_OF_DATE_KHR",
    "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR",
    "VK_ERROR_VALIDATION_FAILED_EXT",
    "VKC_SEND_IPC_MESSAGE_FAILURE",
    "VKC_FAILURE",
    "VKC_ARGUMENT_NOT_VALID",
    "VKC_GET_SHADER_CODE_FAIL",
    "VKC_ALREADY_BEGIN_COMMAND_QUEUE",
    "VKC_COMMAND_QUEUE_NOT_BEGINING",
    "VKC_ERROR_NOT_SETARG_BUFFER_INDEX",
};
// Messages are not proper
static const char* const exceptionDescriptions[] = {
    "VK_NOT_READY",
    "VK_TIMEOUT",
    "VK_ERROR_OUT_OF_HOST_MEMORY",
    "VK_ERROR_OUT_OF_DEVICE_MEMORY",
    "VK_ERROR_INITIALIZATION_FAILED",
    "VK_ERROR_DEVICE_LOST",
    "VK_ERROR_MEMORY_MAP_FAILED",
    "VK_ERROR_LAYER_NOT_PRESENT",
    "VK_ERROR_EXTENSION_NOT_PRESENT",
    "VK_ERROR_FEATURE_NOT_PRESENT",
    "VK_ERROR_INCOMPATIBLE_DRIVER",
    "VK_ERROR_TOO_MANY_OBJECTS",
    "VK_ERROR_FORMAT_NOT_SUPPORTED",
    "VK_ERROR_SURFACE_LOST_KHR",
    "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR",
    "VK_SUBOPTIMAL_KHR",
    "VK_ERROR_OUT_OF_DATE_KHR",
    "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR",
    "VK_ERROR_VALIDATION_FAILED_EXT",
    "VKC_SEND_IPC_MESSAGE_FAILURE",
    "VKC_FAILURE",
    "VKC_ARGUMENT_NOT_VALID",
    "VKC_GET_SHADER_CODE_FAIL",
    "VKC_ALREADY_BEGIN_COMMAND_QUEUE",
    "VKC_COMMAND_QUEUE_NOT_BEGINING",
    "VKC_ERROR_NOT_SETARG_BUFFER_INDEX",
};

void WebVKCException::throwException(int code, ExceptionState& es)
{
    if (isWebVKCException(code)) {
        es.throwDOMException(code, getErrorMessage(code));
    } else {        
        es.throwDOMException(WebVKCException::VKC_FAILURE_ECODE, getErrorMessage(code));
    }
}

void WebVKCException::throwVKCException(int result, ExceptionState& es) {
    switch(result) {
        case VK_NOT_READY: {
            WebVKCException::throwException(WebVKCException::VK_NOT_READY_ECODE, es);
            break;
        }
        case VK_TIMEOUT: {
            WebVKCException::throwException(WebVKCException::VK_TIMEOUT_ECODE, es);
            break;
        }
        case VK_ERROR_OUT_OF_HOST_MEMORY: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_OUT_OF_HOST_MEMORY_ECODE, es);
            break;
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_OUT_OF_DEVICE_MEMORY_ECODE, es);
            break;
        }
        case VK_ERROR_INITIALIZATION_FAILED: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_INITIALIZATION_FAILED_ECODE, es);
            break;
        }
        case VK_ERROR_DEVICE_LOST: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_DEVICE_LOST_ECODE, es);
            break;
        }
        case VK_ERROR_MEMORY_MAP_FAILED: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_MEMORY_MAP_FAILED_ECODE, es);
            break;
        }
        case VK_ERROR_LAYER_NOT_PRESENT: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_LAYER_NOT_PRESENT_ECODE, es);
            break;
        }
        case VK_ERROR_EXTENSION_NOT_PRESENT: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_EXTENSION_NOT_PRESENT_ECODE, es);
            break;
        }
        case VK_ERROR_FEATURE_NOT_PRESENT: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_FEATURE_NOT_PRESENT_ECODE, es);
            break;
        }
        case VK_ERROR_INCOMPATIBLE_DRIVER: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_INCOMPATIBLE_DRIVER_ECODE, es);
            break;
        }
        case VK_ERROR_TOO_MANY_OBJECTS: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_TOO_MANY_OBJECTS_ECODE, es);
            break;
        }
        case VK_ERROR_FORMAT_NOT_SUPPORTED: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_FORMAT_NOT_SUPPORTED_ECODE, es);
            break;
        }
        case VK_ERROR_SURFACE_LOST_KHR: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_SURFACE_LOST_KHR_ECODE, es);
            break;
        }
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR_ECODE, es);
            break;
        }
        case VK_SUBOPTIMAL_KHR: {
            WebVKCException::throwException(WebVKCException::VK_SUBOPTIMAL_KHR_ECODE, es);
            break;
        }
        case VK_ERROR_OUT_OF_DATE_KHR: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_OUT_OF_DATE_KHR_ECODE, es);
            break;
        }
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR_ECODE, es);
            break;
        }
        case VK_ERROR_VALIDATION_FAILED_EXT: {
            WebVKCException::throwException(WebVKCException::VK_ERROR_VALIDATION_FAILED_EXT_ECODE, es);
            break;
        }
        case VKC_SEND_IPC_MESSAGE_FAILURE: {
            WebVKCException::throwException(WebVKCException::VKC_SEND_IPC_MESSAGE_FAILURE_ECODE, es);
            break;
        }
        case VKC_FAILURE: {
            WebVKCException::throwException(WebVKCException::VKC_FAILURE_ECODE, es);
            break;
        }
        case VKC_ARGUMENT_NOT_VALID: {
            WebVKCException::throwException(WebVKCException::VKC_ARGUMENT_NOT_VALID_ECODE, es);
            break;
        }
        case VKC_GET_SHADER_CODE_FAIL: {
            WebVKCException::throwException(WebVKCException::VKC_GET_SHADER_CODE_FAIL_ECODE, es);
            break;
        }
        case VKC_ALREADY_BEGIN_COMMAND_QUEUE: {
            WebVKCException::throwException(WebVKCException::VKC_ALREADY_BEGIN_COMMAND_QUEUE_ECODE, es);
            break;
        }
        case VKC_COMMAND_QUEUE_NOT_BEGINING: {
            WebVKCException::throwException(WebVKCException::VKC_COMMAND_QUEUE_NOT_BEGINING_ECODE, es);
            break;
        }
        case VKC_ERROR_NOT_SETARG_BUFFER_INDEX: {
            WebVKCException::throwException(WebVKCException::VKC_ERROR_NOT_SETARG_BUFFER_INDEX_ECODE, es);
            break;
        }
        default: {
            WebVKCException::throwException(WebVKCException::VKC_FAILURE_ECODE, es);
            break;
        }
    }
}

WebVKCException::WebVKCException(unsigned short code, const String& name, const String& sanitizedMessage, const String& unsanitizedMessage)
{
	m_code = code;
	m_name = name;
	m_sanitizedMessage = sanitizedMessage;
	m_unsanitizedMessage = unsanitizedMessage;
}

WebVKCException* WebVKCException::create(ExceptionCode ec, const String& sanitizedMessage, const String& unsanitizedMessage)
{
	return new WebVKCException(ec, getErrorName(ec), getErrorMessage(ec), unsanitizedMessage);
}

String WebVKCException::getErrorName(int code) {
	int index = code - WebVKCExceptionOffset;
	if (index < 0 || index >= WebVKCExceptionMax) {
		index = WebVKCException::VKC_FAILURE_ECODE;
	}
	VKCLOG(INFO) << "string : " << String(exceptionNames[index]);
	return String(exceptionNames[index]);
}

String WebVKCException::getErrorMessage(int code) {
	VKCLOG(INFO) << "WebVKCException::getErrorMessage, code=" << String::number(code);

	int index = code - WebVKCExceptionOffset;
	if (index < 0 || index >= WebVKCExceptionMax) {
		index = WebVKCException::VKC_FAILURE_ECODE;
	}
	return String(exceptionDescriptions[index]);
}

DEFINE_TRACE(WebVKCException) {
}

} // namespace blink
