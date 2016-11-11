// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_NATIVE_VULKAN_INCLUDE_H_
#define UI_NATIVE_VULKAN_INCLUDE_H_

#include "base/logging.h"

 //vulkan
#include <third_party/vulkan/include/vulkan/vulkan.h>

typedef VkResult VKCResult;
typedef long VKCint;
typedef unsigned VKCuint;
typedef unsigned VKCenum;

enum VULKAN_OPERATION_TYPE {
	VKC_WRITE_BUFFER,
	VKC_READ_BUFFER
};

#define VKC_SEND_IPC_MESSAGE_FAILURE 	-9999
#define VKC_FAILURE 	16000001
#define VKC_ARGUMENT_NOT_VALID 	16000002
#define VKC_GET_SHADER_CODE_FAIL	16000003
#define VKC_ALREADY_BEGIN_COMMAND_QUEUE	16000004
#define VKC_COMMAND_QUEUE_NOT_BEGINING	16000005
#define VKC_ERROR_NOT_SETARG_BUFFER_INDEX	16000006

// #define ENABLE_VKCLOG

#ifdef ENABLE_VKCLOG
#define VKCLOG(severity) \
	DLOG(severity) << "VKC::"
#else
#define VKCLOG(severity) \
	LAZY_STREAM(LOG_STREAM(severity), false)
#endif

#if defined(OS_ANDROID)
#define VKCPoint uint32_t
#elif defined(OS_LINUX)
#define VKCPoint uintptr_t
#endif

enum VULKAN_DEVICE_GETINFO_NAME_TABLE {
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

#define ARG_PTR(type) \
	type *

#define ARG_CONST(type) \
	const type

//define function pointer, no argument
#define VK_API_ARGS0(name, return_type) \
	typedef return_type (*name)();
//define function pointer, has 1 argument
#define VK_API_ARGS1(name, return_type, arg1_type) \
	typedef return_type (*name)(arg1_type);
//define function pointer, has 2 arguments
#define VK_API_ARGS2(name, return_type, arg1_type, arg2_type) \
	typedef return_type (*name)(arg1_type, arg2_type);
//define function pointer, has 3 arguments
#define VK_API_ARGS3(name, return_type, arg1_type, arg2_type, arg3_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type);
//define function pointer, has 4 arguments
#define VK_API_ARGS4(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type);
//define function pointer, has 5 arguments
#define VK_API_ARGS5(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type);
//define function pointer, has 6 arguments
#define VK_API_ARGS6(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type);
//define function pointer, has 7 arguments
#define VK_API_ARGS7(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type);
//define function pointer, has 8 arguments
#define VK_API_ARGS8(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type);
//define function pointer, has 9 arguments
#define VK_API_ARGS9(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type);
//define function pointer, has 10 arguments
#define VK_API_ARGS10(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type);

#define VK_API_LOAD(library, api_name, name) \
		base::GetFunctionPointerFromNativeLibrary(library, api_name) ? reinterpret_cast<name>(base::GetFunctionPointerFromNativeLibrary(library, api_name)) : reinterpret_cast<name>(handleFuncLookupFail(api_name));

struct BaseVKCOperationData {
	unsigned uint_01;
	unsigned uint_02;
	VKCPoint point_01;
	VKCPoint point_02;
};

struct BaseVKCResultData {
	VKCResult result_01;
};

struct WebVKC_Operation_Base {
	int operation_type;
};

struct WebVKC_RW_Buffer_Operation : public WebVKC_Operation_Base {
	VKCuint index;
	VKCuint bufferByteSize;
	VKCPoint vkcDevice;
	VKCPoint vkcMemory;
	WebVKC_RW_Buffer_Operation(int op_type) {
		operation_type = op_type;
	};
};

struct WebVKC_Result_Base {
	int operation_type;
	VKCResult result;
};

#endif  // UI_NATIVE_VULKAN_INCLUDE_H_
