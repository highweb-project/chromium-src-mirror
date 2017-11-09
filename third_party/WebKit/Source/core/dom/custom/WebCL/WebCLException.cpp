// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WebCLException.h"

#include "gpu/opencl/opencl_include.h"

namespace blink {

// This should be an array of structs to pair the names and descriptions. ??
static const char* const exceptionNames[] = {
   "SUCCESS",
	"DEVICE_NOT_FOUND",
	"DEVICE_NOT_AVAILABLE",
	"COMPILER_NOT_AVAILABLE",
	"MEM_OBJECT_ALLOCATION_FAILURE",
	"OUT_OF_RESOURCES",
	"OUT_OF_HOST_MEMORY",
	"PROFILING_INFO_NOT_AVAILABLE",
	"MEM_COPY_OVERLAP",
	"IMAGE_FORMAT_MISMATCH",
	"IMAGE_FORMAT_NOT_SUPPORTED",
	"BUILD_PROGRAM_FAILURE",
	"MAP_FAILURE",
	"MISALIGNED_SUB_BUFFER_OFFSET",
	"EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST",
	"INVALID_VALUE",
	"INVALID_DEVICE_TYPE",
	"INVALID_PLATFORM",
	"INVALID_DEVICE",
	"INVALID_CONTEXT",
	"INVALID_QUEUE_PROPERTIES",
	"INVALID_COMMAND_QUEUE",
	"INVALID_HOST_PTR",
	"INVALID_MEM_OBJECT",
	"INVALID_IMAGE_FORMAT_DESCRIPTOR",
	"INVALID_IMAGE_SIZE",
	"INVALID_SAMPLER",
	"INVALID_BINARY",
	"INVALID_BUILD_OPTIONS",
	"INVALID_PROGRAM",
	"INVALID_PROGRAM_EXECUTABLE",
	"INVALID_KERNEL_NAME",
	"INVALID_KERNEL_DEFINITION",
	"INVALID_KERNEL",
	"INVALID_ARG_INDEX",
	"INVALID_ARG_VALUE",
	"INVALID_ARG_SIZE",
	"INVALID_KERNEL_ARGS",
	"INVALID_WORK_DIMENSION",
	"INVALID_WORK_GROUP_SIZE",
	"INVALID_WORK_ITEM_SIZE",
	"INVALID_GLOBAL_OFFSET",
	"INVALID_EVENT_WAIT_LIST",
	"INVALID_EVENT",
	"INVALID_OPERATION",
	"INVALID_GL_OBJECT",
	"INVALID_BUFFER_SIZE",
	"INVALID_MIP_LEVEL",
	"INVALID_GLOBAL_WORK_SIZE",
	"INVALID_PROPERTY",
	"FAILURE",
	"WEBCL_EXTENSION_NOT_ENABLED",
	"WEBCL_API_NOT_PERMITTED",
	"INVALID_GL_SHAREGROUP_REFERENCE_KHR",
};
// Messages are not proper
static const char* const exceptionDescriptions[] = {
	"WEBCL_IMPL_SUCCESS",
	"WEBCL_IMPL_DEVICE_NOT_FOUND",
	"WEBCL_IMPL_DEVICE_NOT_AVAILABLE",
	"WEBCL_IMPL_COMPILER_NOT_AVAILABLE",
	"WEBCL_IMPL_MEM_OBJECT_ALLOCATION_FAILURE",
	"WEBCL_IMPL_OUT_OF_RESOURCES",
	"WEBCL_IMPL_OUT_OF_HOST_MEMORY",
	"WEBCL_IMPL_PROFILING_INFO_NOT_AVAILABLE",
	"WEBCL_IMPL_MEM_COPY_OVERLAP",
	"WEBCL_IMPL_IMAGE_FORMAT_MISMATCH",
	"WEBCL_IMPL_IMAGE_FORMAT_NOT_SUPPORTED",
	"WEBCL_IMPL_BUILD_PROGRAM_FAILURE",
	"WEBCL_IMPL_MAP_FAILURE",
	"WEBCL_IMPL_MISALIGNED_SUB_BUFFER_OFFSET",
	"WEBCL_IMPL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST",
	"WEBCL_IMPL_INVALID_VALUE",
	"WEBCL_IMPL_INVALID_DEVICE_TYPE",
	"WEBCL_IMPL_INVALID_PLATFORM",
	"WEBCL_IMPL_INVALID_DEVICE",
	"WEBCL_IMPL_INVALID_CONTEXT",
	"WEBCL_IMPL_INVALID_QUEUE_PROPERTIES",
	"WEBCL_IMPL_INVALID_COMMAND_QUEUE",
	"WEBCL_IMPL_INVALID_HOST_PTR",
	"WEBCL_IMPL_INVALID_MEM_OBJECT",
	"WEBCL_IMPL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
	"WEBCL_IMPL_INVALID_IMAGE_SIZE",
	"WEBCL_IMPL_INVALID_SAMPLER",
	"WEBCL_IMPL_INVALID_BINARY",
	"WEBCL_IMPL_INVALID_BUILD_OPTIONS",
	"WEBCL_IMPL_INVALID_PROGRAM",
	"WEBCL_IMPL_INVALID_PROGRAM_EXECUTABLE",
	"WEBCL_IMPL_INVALID_KERNEL_NAME",
	"WEBCL_IMPL_INVALID_KERNEL_DEFINITION",
	"WEBCL_IMPL_INVALID_KERNEL",
	"WEBCL_IMPL_INVALID_ARG_INDEX",
	"WEBCL_IMPL_INVALID_ARG_VALUE",
	"WEBCL_IMPL_INVALID_ARG_SIZE",
	"WEBCL_IMPL_INVALID_KERNEL_ARGS",
	"WEBCL_IMPL_INVALID_WORK_DIMENSION",
	"WEBCL_IMPL_INVALID_WORK_GROUP_SIZE",
	"WEBCL_IMPL_INVALID_WORK_ITEM_SIZE",
	"WEBCL_IMPL_INVALID_GLOBAL_OFFSET",
	"WEBCL_IMPL_INVALID_EVENT_WAIT_LIST",
	"WEBCL_IMPL_INVALID_EVENT",
	"WEBCL_IMPL_INVALID_OPERATION",
	"WEBCL_IMPL_INVALID_GL_OBJECT",
	"WEBCL_IMPL_INVALID_BUFFER_SIZE",
	"WEBCL_IMPL_INVALID_MIP_LEVEL",
	"WEBCL_IMPL_INVALID_GLOBAL_WORK_SIZE",
	"WEBCL_IMPL_INVALID_PROPERTY",
	"WEBCL_IMPL_FAILURE",
	"WEBCL_IMPL_WEBCL_EXTENSION_NOT_ENABLED",
	"WEBCL_IMPL_WEBCL_API_NOT_PERMITTED	",
	"WEBCL_IMPL_INVALID_GL_SHAREGROUP_REFERENCE_KHR",
};

void WebCLException::throwException(int& code, ExceptionState& es)
{
    switch(code) {
    case CL_DEVICE_NOT_FOUND:
        es.ThrowDOMException(WebCLException::DEVICE_NOT_FOUND, getErrorMessage(code));
        break;
    case CL_DEVICE_NOT_AVAILABLE:
        es.ThrowDOMException(WebCLException::DEVICE_NOT_AVAILABLE, getErrorMessage(code));
        break;
    case CL_COMPILER_NOT_AVAILABLE:
        es.ThrowDOMException(WebCLException::COMPILER_NOT_AVAILABLE, getErrorMessage(code));
        break;
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        es.ThrowDOMException(WebCLException::MEM_OBJECT_ALLOCATION_FAILURE, getErrorMessage(code));
        break;
    case CL_OUT_OF_RESOURCES:
        es.ThrowDOMException(WebCLException::OUT_OF_RESOURCES, getErrorMessage(code));
        break;
    case CL_OUT_OF_HOST_MEMORY:
        es.ThrowDOMException(WebCLException::OUT_OF_HOST_MEMORY, getErrorMessage(code));
        break;
    case CL_PROFILING_INFO_NOT_AVAILABLE:
        es.ThrowDOMException(WebCLException::PROFILING_INFO_NOT_AVAILABLE, getErrorMessage(code));
        break;
    case CL_MEM_COPY_OVERLAP:
        es.ThrowDOMException(WebCLException::MEM_OBJECT_ALLOCATION_FAILURE, getErrorMessage(code));
        break;
    case CL_IMAGE_FORMAT_MISMATCH:
        es.ThrowDOMException(WebCLException::IMAGE_FORMAT_MISMATCH, getErrorMessage(code));
        break;
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        es.ThrowDOMException(WebCLException::IMAGE_FORMAT_NOT_SUPPORTED, getErrorMessage(code));
        break;
    case CL_BUILD_PROGRAM_FAILURE:
        es.ThrowDOMException(WebCLException::BUILD_PROGRAM_FAILURE, getErrorMessage(code));
        break;
    case CL_MAP_FAILURE:
        es.ThrowDOMException(WebCLException::MAP_FAILURE,getErrorMessage(code));
        break;
    case CL_MISALIGNED_SUB_BUFFER_OFFSET:
        es.ThrowDOMException(WebCLException::MISALIGNED_SUB_BUFFER_OFFSET, getErrorMessage(code));
        break;
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
        es.ThrowDOMException(WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST, getErrorMessage(code));
        break;
    case CL_INVALID_VALUE:
        es.ThrowDOMException(WebCLException::INVALID_VALUE, getErrorMessage(code));
        break;
    case CL_INVALID_DEVICE_TYPE:
        es.ThrowDOMException(WebCLException::INVALID_DEVICE_TYPE, getErrorMessage(code));
        break;
    case CL_INVALID_PLATFORM:
        es.ThrowDOMException(WebCLException::INVALID_PLATFORM, getErrorMessage(code));
        break;
    case CL_INVALID_DEVICE:
        es.ThrowDOMException(WebCLException::INVALID_DEVICE, getErrorMessage(code));
        break;
    case CL_INVALID_CONTEXT:
        es.ThrowDOMException(WebCLException::INVALID_CONTEXT, getErrorMessage(code));
        break;
    case CL_INVALID_QUEUE_PROPERTIES:
        es.ThrowDOMException(WebCLException::INVALID_QUEUE_PROPERTIES, getErrorMessage(code));
        break;
    case CL_INVALID_COMMAND_QUEUE:
        es.ThrowDOMException(WebCLException::INVALID_COMMAND_QUEUE, getErrorMessage(code));
        break;
    case CL_INVALID_HOST_PTR:
        es.ThrowDOMException(WebCLException::INVALID_HOST_PTR, getErrorMessage(code));
        break;
    case CL_INVALID_MEM_OBJECT:
        es.ThrowDOMException(WebCLException::INVALID_MEM_OBJECT, getErrorMessage(code));
        break;
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        es.ThrowDOMException(WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR, getErrorMessage(code));
        break;
    case CL_INVALID_IMAGE_SIZE:
        es.ThrowDOMException(WebCLException::INVALID_IMAGE_SIZE, getErrorMessage(code));
        break;
    case CL_INVALID_SAMPLER:
        es.ThrowDOMException(WebCLException::INVALID_SAMPLER, getErrorMessage(code));
        break;
    case CL_INVALID_BINARY:
        es.ThrowDOMException(WebCLException::INVALID_BINARY, getErrorMessage(code));
        break;
    case CL_INVALID_BUILD_OPTIONS:
        es.ThrowDOMException(WebCLException::INVALID_BUILD_OPTIONS, getErrorMessage(code));
        break;
    case CL_INVALID_PROGRAM:
        es.ThrowDOMException(WebCLException::INVALID_PROGRAM, getErrorMessage(code));
        break;
    case CL_INVALID_PROGRAM_EXECUTABLE:
        es.ThrowDOMException(WebCLException::INVALID_PROGRAM_EXECUTABLE, getErrorMessage(code));
        break;
    case CL_INVALID_KERNEL_NAME:
        es.ThrowDOMException(WebCLException::INVALID_KERNEL_NAME, getErrorMessage(code));
        break;
    case CL_INVALID_KERNEL_DEFINITION:
        es.ThrowDOMException(WebCLException::INVALID_KERNEL_DEFINITION, getErrorMessage(code));
        break;
    case CL_INVALID_KERNEL:
        es.ThrowDOMException(WebCLException::INVALID_KERNEL,getErrorMessage(code));
        break;
    case CL_INVALID_ARG_INDEX:
        es.ThrowDOMException(WebCLException::INVALID_ARG_INDEX, getErrorMessage(code));
        break;
    case CL_INVALID_ARG_VALUE:
        es.ThrowDOMException(WebCLException::INVALID_ARG_VALUE, getErrorMessage(code));
        break;
    case CL_INVALID_ARG_SIZE:
        es.ThrowDOMException(WebCLException::INVALID_ARG_SIZE, getErrorMessage(code));
        break;
    case CL_INVALID_KERNEL_ARGS:
        es.ThrowDOMException(WebCLException::INVALID_KERNEL_ARGS, getErrorMessage(code));
        break;
    case CL_INVALID_WORK_DIMENSION:
        es.ThrowDOMException(WebCLException::INVALID_WORK_DIMENSION, getErrorMessage(code));
        break;
    case CL_INVALID_WORK_GROUP_SIZE:
        es.ThrowDOMException(WebCLException::INVALID_WORK_GROUP_SIZE, getErrorMessage(code));
        break;
    case CL_INVALID_WORK_ITEM_SIZE:
        es.ThrowDOMException(WebCLException::INVALID_WORK_ITEM_SIZE, getErrorMessage(code));
        break;
    case CL_INVALID_GLOBAL_OFFSET:
        es.ThrowDOMException(WebCLException::INVALID_GLOBAL_OFFSET, getErrorMessage(code));
        break;
    case CL_INVALID_EVENT_WAIT_LIST:
        es.ThrowDOMException(WebCLException::INVALID_EVENT_WAIT_LIST, getErrorMessage(code));
        break;
    case CL_INVALID_EVENT:
        es.ThrowDOMException(WebCLException::INVALID_EVENT, getErrorMessage(code));
        break;
    case CL_INVALID_OPERATION:
        es.ThrowDOMException(WebCLException::INVALID_OPERATION, getErrorMessage(code));
        break;
    case CL_INVALID_GL_OBJECT:
        es.ThrowDOMException(WebCLException::INVALID_GL_OBJECT, getErrorMessage(code));
        break;
    case CL_INVALID_BUFFER_SIZE:
        es.ThrowDOMException(WebCLException::INVALID_BUFFER_SIZE, getErrorMessage(code));
        break;
    case CL_INVALID_MIP_LEVEL:
        es.ThrowDOMException(WebCLException::INVALID_MIP_LEVEL, getErrorMessage(code));
        break;
    case CL_INVALID_GLOBAL_WORK_SIZE:
        es.ThrowDOMException(WebCLException::INVALID_GLOBAL_WORK_SIZE, getErrorMessage(code));
        break;
    case CL_INVALID_PROPERTY:
        es.ThrowDOMException(WebCLException::INVALID_PROPERTY, getErrorMessage(code));
        break;
    case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
        es.ThrowDOMException(WebCLException::INVALID_GL_SHAREGROUP_REFERENCE_KHR, getErrorMessage(code));
        break;
    default:
        es.ThrowDOMException(WebCLException::FAILURE, getErrorMessage(code));
        break;
    }
}

WebCLException::WebCLException(unsigned short code, const String& name, const String& sanitizedMessage, const String& unsanitizedMessage)
{
	m_code = code;
	m_name = name;
	m_sanitizedMessage = sanitizedMessage;
	m_unsanitizedMessage = unsanitizedMessage;

	CLLOG(INFO) << "WebCLException, code/name/sanitizedMsg/unSanitizedMsg/" << code << "/" << name.Latin1().data() << "/" << sanitizedMessage.Latin1().data() << "/" << unsanitizedMessage.Latin1().data();
}

WebCLException* WebCLException::create(ExceptionCode ec, const String& sanitizedMessage, const String& unsanitizedMessage)
{
	return new WebCLException(ec, getErrorName(ec), getErrorMessage(ec), unsanitizedMessage);
}

String WebCLException::getErrorName(int code) {
	CLLOG(INFO) << "WebCLException::getErrorName, code=" << code;
	int index = code - WebCLExceptionOffset;
	if (index < 0 || index >= WebCLExceptionMax) {
		index = WebCLException::FAILURE - WebCLExceptionOffset;
	}
	return String(exceptionNames[index]);
}

String WebCLException::getErrorMessage(int code) {
	CLLOG(INFO) << "WebCLException::getErrorMessage, code=" << code;

	int index = code - WebCLExceptionOffset;
	if (index < 0 || index >= WebCLExceptionMax) {
		index = WebCLException::FAILURE - WebCLExceptionOffset;
	}
	return String(exceptionDescriptions[index]);
}

} // namespace blink
