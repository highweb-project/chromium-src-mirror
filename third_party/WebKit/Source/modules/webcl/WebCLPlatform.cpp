// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "modules/webcl/WebCLPlatform.h"

#include "WebCL.h"
#include "WebCLException.h"

#include "bindings/core/v8/V8Binding.h"

#include "wtf/HashSet.h"

namespace blink {

WebCLPlatform::WebCLPlatform(WebCL* context, cl_platform_id platform_id)
	: m_context(context),
	  m_cl_platform_id(platform_id)
{
	initSupportedExtensionState = WebCLException::SUCCESS;
}

WebCLPlatform::~WebCLPlatform()
{
}


HeapVector<Member<WebCLDevice>> WebCLPlatform::getDevices(ExceptionState& ec) {
	return getDevices(WebCL::DEVICE_TYPE_ALL, ec);
}

HeapVector<Member<WebCLDevice>> WebCLPlatform::getDevices(unsigned device_type, ExceptionState& ec) {
	CLLOG(INFO) << "WebCLPlatform::getDevices, device_type=" << device_type;

	WebCLDeviceMap* deviceMap = NULL;
	HeapVector<Member<WebCLDevice>> deviceList;

	if(device_type != WebCL::DEVICE_TYPE_DEFAULT &&
			 device_type != WebCL::DEVICE_TYPE_CPU &&
			 device_type != WebCL::DEVICE_TYPE_GPU &&
			 device_type != WebCL::DEVICE_TYPE_ACCELERATOR &&
			 device_type != WebCL::DEVICE_TYPE_ALL)
	{
		initSupportedExtensionState = WebCLException::INVALID_DEVICE_TYPE;
		ec.throwDOMException(WebCLException::INVALID_DEVICE_TYPE, "WebCLException::INVALID_DEVICE_TYPE");
		return deviceList;
	}
	else if (m_cl_platform_id == NULL) {
		printf("Error: Invalid Platform ID\n");
		initSupportedExtensionState = WebCLException::INVALID_PLATFORM;
		ec.throwDOMException(WebCLException::INVALID_PLATFORM, "WebCLException::INVALID_PLATFORM");
		return deviceList;
	}
	else {
		switch(device_type){
			case WebCL::DEVICE_TYPE_DEFAULT: {
				if (m_device_list_type_DEFAULT.size() <= 0) {
					CLLOG(INFO) << "device_list_default create";
					m_device_list_type_DEFAULT = getDeviceList(m_context, m_cl_platform_id, device_type, ec);
				}
				deviceMap = &m_device_list_type_DEFAULT;
				break;
			}
			case WebCL::DEVICE_TYPE_CPU: {
				if (m_device_list_type_CPU.size() <= 0) {
					CLLOG(INFO) << "device_list_default create";
					m_device_list_type_CPU = getDeviceList(m_context, m_cl_platform_id, device_type, ec);
				}
				deviceMap = &m_device_list_type_CPU;
				break;
			}
			case WebCL::DEVICE_TYPE_GPU: {
				if (m_device_list_type_GPU.size() <= 0) {
					CLLOG(INFO) << "device_list_default create";
					m_device_list_type_GPU = getDeviceList(m_context, m_cl_platform_id, device_type, ec);
				}
				deviceMap = &m_device_list_type_GPU;
				break;
			}
			case WebCL::DEVICE_TYPE_ACCELERATOR: {
				if (m_device_list_type_ACCELERATOR.size() <= 0) {
					CLLOG(INFO) << "device_list_default create";
					m_device_list_type_ACCELERATOR = getDeviceList(m_context, m_cl_platform_id, device_type, ec);
				}
				deviceMap = &m_device_list_type_ACCELERATOR;
				break;
			}
			case WebCL::DEVICE_TYPE_ALL: {
				if (m_device_list_type_ALL.size() <= 0) {
					CLLOG(INFO) << "device_list_type_ALL create";
					m_device_list_type_ALL = getDeviceList(m_context, m_cl_platform_id, WebCL::DEVICE_TYPE_ALL, ec);
				}
				deviceMap = &m_device_list_type_ALL;
				break;
			}
		}
	}

	if (deviceMap != NULL) {
		for(WebCLDeviceMap::iterator iter = deviceMap->begin(); iter != deviceMap->end(); ++iter) {
			deviceList.append(iter->value);
		}
	}

	if (deviceList.size() == 0) {
		initSupportedExtensionState = WebCLException::DEVICE_NOT_FOUND;
		ec.throwDOMException(WebCLException::DEVICE_NOT_FOUND, "WebCLException::DEVICE_NOT_FOUND");
	}
	return deviceList;
}

ScriptValue WebCLPlatform::getInfo (ScriptState* scriptState, int platform_info, ExceptionState& ec)
{
	v8::Isolate* isolate = scriptState->isolate();

	cl_int err = 0;
	if (m_cl_platform_id == NULL) {
			ec.throwDOMException(WebCLException::INVALID_PLATFORM, "WebCLException::INVALID_PLATFORM");
			return ScriptValue(scriptState, v8::Null(isolate));
	}

	char platform_string[1024];
	switch(platform_info)
	{
		case WebCL::PLATFORM_PROFILE:
			// err = webcl_clGetPlatformInfo(webcl_channel_, m_cl_platform_id, CL_PLATFORM_PROFILE, sizeof(platform_string), platform_string, NULL);
			// if (err == CL_SUCCESS)
				// return ScriptValue(scriptState, v8String(isolate, String(platform_string)));
			return ScriptValue(scriptState, v8String(isolate, String("WEBCL_PROFILE")));
			break;
		case WebCL::PLATFORM_VERSION:
			// err = webcl_clGetPlatformInfo(webcl_channel_, m_cl_platform_id, CL_PLATFORM_VERSION, sizeof(platform_string), platform_string, NULL);
			// if (err == CL_SUCCESS)
				// return ScriptValue(scriptState, v8String(isolate, String(platform_string)));
			return ScriptValue(scriptState, v8String(isolate, String("WebCL 1.0")));
			break;
		case WebCL::PLATFORM_NAME:
			err = webcl_clGetPlatformInfo(webcl_channel_, m_cl_platform_id, CL_PLATFORM_NAME, sizeof(platform_string), platform_string, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8String(isolate, String(platform_string)));
			break;
		case WebCL::PLATFORM_VENDOR:
			err = webcl_clGetPlatformInfo(webcl_channel_, m_cl_platform_id, CL_PLATFORM_VENDOR, sizeof(platform_string), platform_string, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8String(isolate, String(platform_string)));
			break;
		case WebCL::PLATFORM_EXTENSIONS:
			err = webcl_clGetPlatformInfo(webcl_channel_, m_cl_platform_id, CL_PLATFORM_EXTENSIONS, sizeof(platform_string), platform_string, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8String(isolate, String(platform_string)));
			break;
		default:
			printf("Error: Unsupported Platform Info type = %d ",platform_info);
			ec.throwDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			return ScriptValue(scriptState, v8::Null(isolate));
	}

	if(err != CL_SUCCESS)
	{
		WebCLException::throwException(err, ec);
	}
	return ScriptValue(scriptState, v8::Null(isolate));
}

WTF::Vector<WTF::String> WebCLPlatform::getSupportedExtensions(ExceptionState& ec)
{
	CLLOG(INFO) << "getSupportedExtensions is called";
	if (0 == supportedExtensionList.size() && WebCLException::SUCCESS == initSupportedExtensionState) {
		CLLOG(INFO) << "supportedExtensionList is null. call initSupportedExtension";
		initSupportedExtension(ec);
	}

	if (WebCLException::SUCCESS != initSupportedExtensionState) {
		CLLOG(INFO) << "initSupportedExtension error : " << initSupportedExtensionState;
		ec.throwDOMException(initSupportedExtensionState, WebCLException::getErrorName(initSupportedExtensionState));
	}

	return supportedExtensionList;
}

int WebCLPlatform::initSupportedExtension(ExceptionState& ec) {
	// CLLOG(INFO) << "CL::" << "WebCLPlatform::initSupportedExtension";

	// if (WebCLException::SUCCESS != initSupportedExtensionState) {
	// 	ec.throwDOMException(initSupportedExtensionState, WebCLException::getErrorName(initSupportedExtensionState));
	// }

	// WTF::Vector<WTF::String> result = WTF::Vector<WTF::String>();

	// char platform_string[1024] = "";
	// char extensions[16][64];
	// int count = 0;
	// int word_length = 0;
	// int i =0;
	// int j = 0;
	// HashSet<WTF::String> data;
	// HeapVector<Member<WebCLDevice>> deviceList;
	// HashSet<WTF::String>::iterator data_iterator;
	// supportedExtensionList.clear();

	// if (m_cl_platform_id == NULL) {
	// 	printf("Error: Invalid Platform ID\n");
	// 	initSupportedExtensionState = WebCLException::INVALID_PLATFORM;
	// 	supportedExtensionList = result;
	// 	return -1;
	// }
	// cl_int err = webcl_clGetPlatformInfo(webcl_channel_, m_cl_platform_id, WebCL::PLATFORM_EXTENSIONS, sizeof(platform_string), platform_string, NULL);
	// CLLOG(INFO) << "CL::" << ">>" << platform_string;
	// if (err != CL_SUCCESS) {
	// 	initSupportedExtensionState = WebCLException::INVALID_PLATFORM;
	// 	supportedExtensionList = result;
	// 	return -1;
	// }

	// while(platform_string[i] != '\0')
	// {
	// 	while(platform_string[i] == ' ')
	// 		++i;
	// 	while(platform_string[i] !=  ' ' && platform_string[i] != '\0')
	// 	extensions[count][word_length++] = platform_string[i++];
	// 	extensions[count++][word_length] = '\0';  /* Append terminator         */
	// 	word_length = 0;
	// }
	// for(i = 0 ; i<count ; i++) {
	// 	printf("CL_PLATFORM_EXTENSIONS: %s\n",extensions[i]);
	// 	data.add(String(extensions[i]));
	// }

	// deviceList = getDevices(ec);

	// CLLOG(INFO) << "deviceList Size : " << deviceList.size();

	// for(i = 0; i < (int)deviceList.size(); i++) {
	// 	WebCLDevice *device = deviceList[i].get();
	// 	WTF::Vector<WTF::String> device_extensions = device->getSupportedExtensions(ec);
	// 	CLLOG(INFO) << "device_extensions.size : " << device_extensions.size();
	// 	for(j = 0; j < (int)device_extensions.size(); j++) {
	// 		CLLOG(INFO) << "device_extensions[" << j  << "] : " << &device_extensions[j];
	// 		data.add(device_extensions[j]);
	// 	}
	// }
	// for(data_iterator = data.begin(); data_iterator != data.end(); ++data_iterator) {
	// 	supportedExtensionList.append(*data_iterator);
	// }
    return 0;
}

CLboolean WebCLPlatform::enableExtension(const String& extensionName, ExceptionState& ec)
{
	CLLOG(INFO) << "enableExtension is called";

	String interExtensionName;

	if (0 == supportedExtensionList.size()) {
		CLLOG(INFO) << "supportedExtensionList is null. call initSupportedExtensions";
		initSupportedExtension(ec);
	}

	if (WebCLException::SUCCESS != initSupportedExtensionState) {
		CLLOG(INFO) << "initSupportedExtensions error : " << initSupportedExtensionState;
		return false;
	}

	if (extensionName.upper() == "KHR_FP16") {
		interExtensionName = String("cl_khr_fp16");
	} else if(extensionName.upper() == "KHR_FP64") {
		interExtensionName = String("cl_khr_fp64");
	} else {
		interExtensionName = extensionName;
	}

	if (interExtensionName == "WEBCL_html_image") {
		if (!enableExtensionList.contains(interExtensionName)) {
			enableExtensionList.append(interExtensionName);
		}
		return true;
	} else if (interExtensionName == "WEBCL_html_video") {
		if (!enableExtensionList.contains(interExtensionName)) {
			enableExtensionList.append(interExtensionName);
		}
		return true;
	} else {
		CLLOG(INFO) << "CL::enableExtension count : " << supportedExtensionList.size();
		if (supportedExtensionList.contains(interExtensionName)) {
			CLLOG(INFO) << "CL::enableExtension diff true " << &interExtensionName;
			if (!enableExtensionList.contains(interExtensionName)) {
				enableExtensionList.append(interExtensionName);
				if (interExtensionName == "cl_khr_fp64") {
					enableExtensionList.append("KHR_fp64");
				} else if(interExtensionName == "cl_khr_fp16") {
					enableExtensionList.append("KHR_fp16");
				}
			}
			return true;
		}
	}
	return false;
}

WebCLDeviceMap WebCLPlatform::getDeviceList(WebCL* compute_context,
		cl_platform_id platform_id, unsigned device_type, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLPlatform::getDeviceList, device_type=" << device_type;

	if (m_device_list_type_ALL.size() <= 0 && device_type != WebCL::DEVICE_TYPE_ALL) {
		m_device_list_type_ALL = getDeviceList(compute_context, platform_id, WebCL::DEVICE_TYPE_ALL, ec);
	} else if (m_device_list_type_ALL.size() > 0 && device_type == WebCL::DEVICE_TYPE_ALL) {
		return m_device_list_type_ALL;
	}

	WebCLDeviceMap deviceList;
	cl_int err = 0;
	cl_uint num_devices = 0;
	cl_device_id* m_cl_devices = NULL;
	cl_uint m_num_devices = 0;

	if(device_type == WebCL::DEVICE_TYPE_GPU)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
	else if(device_type ==  WebCL::DEVICE_TYPE_CPU)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_CPU, 0, NULL, &num_devices);
	else if(device_type ==  WebCL::DEVICE_TYPE_ACCELERATOR)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &num_devices);
	else if(device_type == WebCL::DEVICE_TYPE_DEFAULT)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_DEFAULT, 0, NULL, &num_devices);
	else if(device_type == WebCL::DEVICE_TYPE_ALL)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
	else
		CLLOG(INFO) << "Error:Invalid Device Type";

	m_num_devices = num_devices;
	if(num_devices == 0)
	{
		CLLOG(INFO) << "Error: Device Type Not Supported \n";
		return deviceList;
	}
	if (err != CL_SUCCESS)
	{
		switch (err) {
			case CL_INVALID_PLATFORM :
				printf("Error: CL_INVALID_PLATFORM \n");
				break;
			case CL_INVALID_DEVICE_TYPE :
				printf("Error: CL_INVALID_DEVICE_TYPE \n");
				break;
			case CL_INVALID_VALUE :
				printf("Error: CL_INVALID_VALUE \n");
				break;
			case CL_DEVICE_NOT_FOUND :
				printf("Error: CL_DEVICE_NOT_FOUND \n");
				break;
			case CL_OUT_OF_RESOURCES :
				printf("Error: CL_OUT_OF_RESOURCES  \n");
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				break;
			default:
				printf("Error: Invaild Error Type\n");
				break;
		}
		return deviceList;
	}

	m_cl_devices = new cl_device_id[num_devices];

	if(device_type == WebCL::DEVICE_TYPE_GPU)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_GPU, num_devices, m_cl_devices, &num_devices);
	else if(device_type == WebCL::DEVICE_TYPE_CPU)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_CPU, num_devices, m_cl_devices, &num_devices);
	else if(device_type == WebCL::DEVICE_TYPE_ACCELERATOR)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_ACCELERATOR, num_devices, m_cl_devices, &num_devices);
	else if(device_type == WebCL::DEVICE_TYPE_DEFAULT)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_DEFAULT, num_devices, m_cl_devices, &num_devices);
	else if(device_type == WebCL::DEVICE_TYPE_ALL)
		err = gpu::webcl_clGetDeviceIDs(webcl_channel_, platform_id, CL_DEVICE_TYPE_ALL, num_devices, m_cl_devices, &num_devices);
	else
		printf("Error:Invalid Device Type \n");

	if (err != CL_SUCCESS)
	{
		printf("Error ACCELERATOR 2\n");
		switch (err) {
			case CL_INVALID_PLATFORM :
				printf("Error: CL_INVALID_PLATFORM \n");
				break;
			case CL_INVALID_DEVICE_TYPE :
				printf("Error: CL_INVALID_DEVICE_TYPE \n");
				break;
			case CL_INVALID_VALUE :
				printf("Error: CL_INVALID_VALUE \n");
				break;
			case CL_DEVICE_NOT_FOUND :
				printf("Error: CL_DEVICE_NOT_FOUND \n");
				break;
			case CL_OUT_OF_RESOURCES :
				printf("Error: CL_OUT_OF_RESOURCES  \n");
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("Error: CL_OUT_OF_HOST_MEMORY\n");
				break;
			default:
				printf("Error: Invaild Error Type\n");
				break;
		}
		return deviceList;
	}

	for (unsigned int i = 0; i < m_num_devices; i++) {
		CLLOG(INFO) << "DEVICE_ID[" << i << "]=" << m_cl_devices[i];

		Member<WebCLDevice> o = findCLDevice((cl_obj_key)m_cl_devices[i], ec);
		if (o != NULL) {
			deviceList.set((cl_obj_key)m_cl_devices[i], o);
		} else {
			Member<WebCLDevice> o = WebCLDevice::create(compute_context, m_cl_devices[i]);
			o->setDefaultValue(ec);
			if (ec.hadException()) {
				continue;
			}
			deviceList.set((cl_obj_key)m_cl_devices[i], o);
		}
	}
	return deviceList;
}

CLboolean WebCLPlatform::containDevice(cl_obj_key key, ExceptionState& ec)
{
	if (key == 0) {
		return false;
	}

	if (m_device_list_type_ALL.size() <= 0) {
		getDevices(ec);
	}
	if (m_device_list_type_ALL.contains(key)) {
		CLLOG(INFO) << "WebCLPlatform::containDevice : TYPE_ALL";
		return true;
	}
	return false;
}

Member<WebCLDevice> WebCLPlatform::findCLDevice(cl_obj_key key, ExceptionState& ec)
{
	Member<WebCLDevice> device = nullptr;

	if (key == 0) {
		return nullptr;
	}

	if (m_device_list_type_ALL.contains(key)) {
		device = m_device_list_type_ALL.get(key);
	}

	return device;
}

}
