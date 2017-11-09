// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "WebCLDevice.h"

#include "bindings/core/v8/ExceptionState.h"
#include "platform/bindings/V8Binding.h"
#include "bindings/modules/v8/V8WebCLPlatform.h"

#include "core/dom/custom/WebCL/WebCLException.h"

namespace blink {

WebCLDevice* WebCLDevice::create(WebCL* context, cl_device_id device_id)
{
	return new WebCLDevice(context, device_id);
}

WebCLDevice::WebCLDevice(WebCL* context, cl_device_id device_id)
	: m_context(context), m_cl_device_id(device_id), mDeviceType(0)
{
	initSupportedExtensionState = WebCLException::SUCCESS;
}

WebCLDevice::~WebCLDevice()
{
}

ScriptValue WebCLDevice::getInfo(ScriptState* scriptState, unsigned device_type, ExceptionState& ec)
{
	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();

 	cl_int err = 0;
  std::string device_string;
	size_t string_max_size = 1024;
 	cl_uint uint_units = 0;
 	size_t sizet_units = 0;
 	size_t sizet_array[1024] = {0};
 	cl_ulong  ulong_units = 0;
 	cl_bool bool_units = false;
 	cl_device_type type = 0;
 	cl_device_mem_cache_type global_type = 0;
 	cl_command_queue_properties queue_properties = 0;
 	cl_device_exec_capabilities exec = 0;
 	cl_device_local_mem_type local_type = 0;

 	if (m_cl_device_id == NULL) {
 		CLLOG(INFO) << "Error: Invalid Device ID";
 		ec.ThrowDOMException(WebCLException::INVALID_DEVICE, "WebCLException::INVALID_DEVICE");
 		return ScriptValue(scriptState, v8::Null(isolate));
 	}

 	switch(device_type)
 	{

 		case WebCL::DEVICE_EXTENSIONS:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_EXTENSIONS, string_max_size, &device_string, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, V8String(isolate, String(device_string.data())));
 			break;
 		case WebCL::DEVICE_NAME:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_NAME, string_max_size, &device_string, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, V8String(isolate, String(device_string.data())));
 			break;
 		case WebCL::DEVICE_OPENCL_C_VERSION:
 			return ScriptValue(scriptState, V8String(isolate, String("WebCL C 1.0")));
 			break;
 		case WebCL::DEVICE_VERSION:
 			return ScriptValue(scriptState, V8String(isolate, String("WebCL 1.0")));
 			break;
 		case WebCL::DEVICE_PROFILE:
 			return ScriptValue(scriptState, V8String(isolate, String("WEBCL_PROFILE")));
 			break;
 		case WebCL::DEVICE_VENDOR:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_VENDOR, string_max_size, &device_string, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, V8String(isolate, String(device_string.data())));
 			break;
 		case WebCL::DRIVER_VERSION:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DRIVER_VERSION, string_max_size, &device_string, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, V8String(isolate, String(device_string.data())));
 			break;
 		case WebCL::DEVICE_ADDRESS_BITS:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_ADDRESS_BITS , sizeof(cl_uint), &uint_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_MAX_CLOCK_FREQUENCY:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_MAX_CONSTANT_ARGS:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_MAX_READ_IMAGE_ARGS:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_MAX_SAMPLERS:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_SAMPLERS, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_MAX_WRITE_IMAGE_ARGS:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_MEM_BASE_ADDR_ALIGN:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_VENDOR_ID:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_INT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
 			if (enableExtensionList.Contains("cl_khr_fp64"))
 				err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_INT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_LONG:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
 			if (enableExtensionList.Contains("cl_khr_fp64"))
 				err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_HALF:
 			if (enableExtensionList.Contains("cl_khr_fp16"))
 				err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, sizeof(cl_uint), &uint_units,NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_IMAGE2D_MAX_HEIGHT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_IMAGE2D_MAX_WIDTH:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_IMAGE3D_MAX_DEPTH:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_IMAGE3D_MAX_HEIGHT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_IMAGE3D_MAX_WIDTH:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_MAX_PARAMETER_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_MAX_WORK_GROUP_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_MAX_WORK_ITEM_SIZES:
			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, 1024, &sizet_array, NULL);
			if (err == CL_SUCCESS) {
				Vector<unsigned> maxWorItemSizes;
				maxWorItemSizes.push_back(sizet_array[0]);
				maxWorItemSizes.push_back(sizet_array[1]);
				maxWorItemSizes.push_back(sizet_array[2]);
				return ScriptValue(scriptState, ToV8(maxWorItemSizes, creationContext, isolate));
			}
 			break;
 		case WebCL::DEVICE_PROFILING_TIMER_RESOLUTION:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_MAX_WORK_ITEM_DIMENSIONS:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &sizet_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
 			break;
 		case WebCL::DEVICE_LOCAL_MEM_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(ulong_units)));
 			break;
 		case WebCL::DEVICE_MAX_CONSTANT_BUFFER_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(ulong_units)));
 			break;
 		case WebCL::DEVICE_MAX_MEM_ALLOC_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(ulong_units)));
 			break;
 		case WebCL::DEVICE_GLOBAL_MEM_CACHE_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(ulong_units)));
 			break;
 		case WebCL::DEVICE_GLOBAL_MEM_SIZE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(ulong_units)));
 			break;
 		case WebCL::DEVICE_AVAILABLE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_AVAILABLE , sizeof(cl_bool), &bool_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Boolean::New(isolate, static_cast<bool>(bool_units)));
 			break;
 		case WebCL::DEVICE_COMPILER_AVAILABLE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_COMPILER_AVAILABLE , sizeof(cl_bool), &bool_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Boolean::New(isolate, static_cast<bool>(bool_units)));
 			break;
 		case WebCL::DEVICE_ENDIAN_LITTLE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_bool), &bool_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Boolean::New(isolate, static_cast<bool>(bool_units)));
 			break;
 		case WebCL::DEVICE_ERROR_CORRECTION_SUPPORT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(cl_bool), &bool_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Boolean::New(isolate, static_cast<bool>(bool_units)));
 			break;
 		case WebCL::DEVICE_IMAGE_SUPPORT:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &bool_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Boolean::New(isolate, static_cast<bool>(bool_units)));
 			break;
 		case WebCL::DEVICE_TYPE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(type)));
 			break;
 		case WebCL::DEVICE_QUEUE_PROPERTIES:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_QUEUE_PROPERTIES,
 					sizeof(cl_command_queue_properties), &queue_properties, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(queue_properties)));
 			break;
 		case WebCL::DEVICE_PLATFORM:
 		{
 			cl_platform_id platform_id;
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platform_id, NULL);
 			if (err == CL_SUCCESS) {
 				Persistent<WebCLPlatform> platforms = nullptr;
 				platforms = m_context->findCLPlatform((cl_obj_key)platform_id, ec);
 				if (platforms == NULL) {
 					platforms = WebCLPlatform::create(m_context, platform_id);
 				}
 				return ScriptValue(scriptState, ToV8(platforms, creationContext, isolate));
 			}
 			break;
 		}
 		case WebCL::DEVICE_EXECUTION_CAPABILITIES:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(cl_device_exec_capabilities), &exec, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(exec)));
 			break;
 		case WebCL::DEVICE_GLOBAL_MEM_CACHE_TYPE: {
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cl_device_mem_cache_type), &global_type, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(global_type)));
 			break;
 		}
 		case WebCL::DEVICE_LOCAL_MEM_TYPE:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(cl_device_local_mem_type), &local_type, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(local_type)));
 			break;
 		case WebCL::DEVICE_HOST_UNIFIED_MEMORY:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &bool_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Boolean::New(isolate, static_cast<bool>(bool_units)));
 			break;
 		case WebCL::DEVICE_SINGLE_FP_CONFIG:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_ulong), &ulong_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(ulong_units)));
 			break;
 		case WebCL::DEVICE_MAX_COMPUTE_UNITS:
 			err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &uint_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_HALF: {
 			if (enableExtensionList.Contains("cl_khr_fp16"))
 				err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, sizeof(cl_uint), &uint_units, NULL);
 			if (err == CL_SUCCESS)
 				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
 			break;
 		}
 		default:
 			ec.ThrowDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");
 			printf("Error:UNSUPPORTED DEVICE TYPE = %d ",device_type);
 			return ScriptValue(scriptState, v8::Null(isolate));
 	}
 	WebCLException::throwException(err, ec);
 	return ScriptValue(scriptState, v8::Null(isolate));
 }

Nullable<WTF::Vector<WTF::String>> WebCLDevice::getSupportedExtensions(ExceptionState& ec)
{
	CLLOG(INFO) << "getSupportedExtensions called";
	if (0 == supportedExtensionList.size()) {
		CLLOG(INFO) << "supportedExtensionList is null. call initSupportedExtensions";
		initSupportedExtensions();
	}
	if (WebCLException::SUCCESS != initSupportedExtensionState) {
		CLLOG(INFO) << "initSupportedExtensions error : " << initSupportedExtensionState;
		ec.ThrowDOMException(initSupportedExtensionState, WebCLException::getErrorName(initSupportedExtensionState));
		return nullptr;
	}
	return supportedExtensionList;
}

int WebCLDevice::initSupportedExtensions()
{
	std::string device_string;
	size_t string_max_size = 1024;
	supportedExtensionList.clear();

	if (m_cl_device_id == NULL) {
		initSupportedExtensionState = WebCLException::INVALID_DEVICE;
		return -1;
	}
	cl_int err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_EXTENSIONS, string_max_size, &device_string, NULL);
	if (err != CL_SUCCESS) {
		initSupportedExtensionState = WebCLException::INVALID_DEVICE;
		return -1;
	}

	WTF::String device_data = WTF::String(device_string.data());
	device_data.Split(" ", false, supportedExtensionList);

	CLLOG(INFO) << "errCode : " << initSupportedExtensionState;
	return 0;
}

CLboolean WebCLDevice::enableExtension(const String& extensionName, ExceptionState& ec)
{
	CLLOG(INFO) << "enableExtension is called";
	String interExtensionName;
	if (0 == supportedExtensionList.size()) {
		CLLOG(INFO) << "supportedExtensionList is null. call initSupportedExtensions";
		initSupportedExtensions();
	}

	if (WebCLException::SUCCESS != initSupportedExtensionState) {
		CLLOG(INFO) << "initSupportedExtensions error : " << initSupportedExtensionState;
		ec.ThrowDOMException(initSupportedExtensionState, WebCLException::getErrorName(initSupportedExtensionState));
		return false;
	}

	if (extensionName.UpperASCII() == "KHR_FP16") {
		interExtensionName = String("cl_khr_fp16");
	} else if(extensionName.UpperASCII() == "KHR_FP64") {
		interExtensionName = String("cl_khr_fp64");
	} else {
		interExtensionName = extensionName;
	}

	if (interExtensionName == "WEBCL_html_image") {
		if (!enableExtensionList.Contains(interExtensionName)) {
			enableExtensionList.push_back(interExtensionName);
		}
		return true;
	} else if (interExtensionName == "WEBCL_html_video") {
		if (!enableExtensionList.Contains(interExtensionName)) {
			enableExtensionList.push_back(interExtensionName);
		}
		return true;
	} else {
		CLLOG(INFO) << "CL::enableExtension count : " << supportedExtensionList.size();
		if (supportedExtensionList.Contains(interExtensionName)) {
			CLLOG(INFO) << "CL::enableExtension diff true " << &interExtensionName;
			if (!enableExtensionList.Contains(interExtensionName)) {
				enableExtensionList.push_back(interExtensionName);
				if (interExtensionName == "cl_khr_fp64") {
					enableExtensionList.push_back("KHR_fp64");
				} else if(interExtensionName == "cl_khr_fp16") {
					enableExtensionList.push_back("KHR_fp16");
				}
			}
			return true;
		}
	}
	return false;
}


cl_device_id WebCLDevice::getCLDevice()
{
	return m_cl_device_id;
}

void WebCLDevice::setDefaultValue(ExceptionState& ec)
{
	cl_int err = 0;
 	cl_device_type type = 0;
 	size_t sizet_units = 0;
 	cl_ulong  ulong_units = 0;
 	size_t sizet_array[1024] = {0};

	if (m_cl_device_id == NULL) {
		CLLOG(INFO) << "Error: Invalid Device ID";
	}
	else {
		err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
		if (err == CL_SUCCESS)
			mDeviceType = static_cast<unsigned>(type);
		else {
			WebCLException::throwException(err, ec);
			return;
		}

		err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
		if (err == CL_SUCCESS)
			mMaxMemAllocSize = ulong_units;
		else {
			WebCLException::throwException(err, ec);
			return;
		}

		err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &sizet_units, NULL);
 		if (err == CL_SUCCESS)
 			mImage2DMaxHeight = sizet_units;
 		else {
			WebCLException::throwException(err, ec);
			return;
		}

 		sizet_units = 0;
		err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &sizet_units, NULL);
 		if (err == CL_SUCCESS)
 			mImage2DMaxWidth = sizet_units;
 		else {
			WebCLException::throwException(err, ec);
			return;
		}

		err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, 1024, &sizet_array, NULL);
		if (err == CL_SUCCESS) {
			mMaxWorkItemSizes.push_back(sizet_array[0]);
 			mMaxWorkItemSizes.push_back(sizet_array[1]);
 			mMaxWorkItemSizes.push_back(sizet_array[2]);
 		}
 		else {
 			WebCLException::throwException(err, ec);
 			return;
 		}

 		sizet_units = 0;
		err = webcl_clGetDeviceInfo(webcl_channel_, m_cl_device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, NULL);
 		if (err == CL_SUCCESS)
 			mMaxWorkGroupSize = sizet_units;
 		else {
			WebCLException::throwException(err, ec);
			return;
		}

	}
}

unsigned WebCLDevice::getDeviceType()
{
	return mDeviceType;
}

cl_ulong WebCLDevice::getDeviceMaxMemAllocSize() {
	return mMaxMemAllocSize;
}

size_t WebCLDevice::getImage2DMaxWidth() {
	return mImage2DMaxWidth;
}

size_t WebCLDevice::getImage2DMaxHeight() {
	return mImage2DMaxHeight;
}

Vector<unsigned> WebCLDevice::getMaxWorkItemSizes() {
	return mMaxWorkItemSizes;
}

size_t WebCLDevice::getMaxWorkGroupSize() {
	return mMaxWorkGroupSize;
}

}
