// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "WebCL.h"

#include "base/sys_info.h"

#include "WebCLContext.h"
#include "WebCLPlatform.h"
#include "WebCLException.h"
#include "WebCLValueChecker.h"
#include "WebCLDevice.h"
#include "WebCLCommandQueue.h"
#include "WebCLCallback.h"
#include "WebCLAPIBlocker.h"
#include "WebCLOperationHandler.h"
#include "modules/webgl/WebGLRenderingContext.h"

#include "core/dom/ExecutionContext.h"
#include "core/events/Event.h"
#include "wtf/HashMap.h"

#include "modules/EventTargetModules.h"
#include "third_party/WebKit/public/platform/Platform.h"

gpu::GpuChannelHost* webcl_channel_ = NULL;
// void setWebCLChannelHost(gpu::GpuChannelHost* channel_webcl) {
//      webcl_channel_ = channel_webcl;
// }

namespace blink {

WebCL::WebCL(ExecutionContext* context)
	: m_num_platforms(0),
	  m_cl_platforms(NULL)
{
	webcl_channel_ = Platform::current()->createWebCLGPUChannelContext();

	initSupportedExtensionState = WebCLException::SUCCESS;

	deviceName = base::SysInfo::HardwareModelName().data();
	mApiChecker = new WebCLAPIBlocker(deviceName.upper());

	mOperationHandler = adoptPtr(new WebCLOperationHandler());
	mOperationHandler->startHandling();

	mGL = nullptr;
}

WebCL::~WebCL() {
}

HeapVector<Member<WebCLPlatform>> WebCL::getPlatforms(ExceptionState& ec) {
	if(!createWebCLPlatformList()) {
		// TODO jphong mode erroe handling required?
		CLLOG(INFO) <<"Error: Invalid ERROR Type";
		ec.throwDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");
	}
	HeapVector<Member<WebCLPlatform>> mWebCLPlatformList;

	for(WebCLPlatformMap::iterator iter = mClPlatformMap.begin(); iter != mClPlatformMap.end(); ++iter) {
		CLLOG(INFO) << "mClPlatformMap :: key : "  << iter->key << " value : " << iter->value;
		mWebCLPlatformList.append(iter->value);
	}

	return mWebCLPlatformList;
}

void WebCL::releaseAll(ExceptionState& ec)
{
	CLLOG(INFO) << "=========== releaseAll ===========";

	isReleasing = true;
	CLLOG(INFO) << "WebCLContextMap size = " << mClContextMap.size();
	if (mClContextMap.size() > 0) {
		for(WebCLContextMap::iterator iter = mClContextMap.begin(); iter != mClContextMap.end(); ++iter) {
			WebCLContext* context = (WebCLContext*)iter->value;
			if (context != NULL)
				context->releaseAll(ec);
		}
		mClContextMap.clear();
		enableExtensionList.clear();
	}

	mOperationHandler->finishHandling();

	isReleasing = false;
}

WTF::Vector<WTF::String> WebCL::getSupportedExtensions(ExceptionState& ec)
{
	CLLOG(INFO) << "getSupportedExtensions is called";
	if (0 == supportedExtensionList.size() && WebCLException::SUCCESS == initSupportedExtensionState) {
		CLLOG(INFO) << "supportedExtensionList is null. call initSupportedExtension";
		initSupportedExtensionState = initSupportedExtension(ec);
	}

	if (WebCLException::SUCCESS != initSupportedExtensionState) {
		CLLOG(INFO) << "initSupportedExtension error : " << initSupportedExtensionState;
		ec.throwDOMException(initSupportedExtensionState, WebCLException::getErrorName(initSupportedExtensionState));
	}

	return supportedExtensionList;
}

int WebCL::initSupportedExtension(ExceptionState& ec) {
	CLLOG(INFO) << "WebCL::initSupportedExtension";

	if (WebCLException::SUCCESS != initSupportedExtensionState) {
		ec.throwDOMException(initSupportedExtensionState, WebCLException::getErrorName(initSupportedExtensionState));
	}

	int j = 0;
	HashSet<WTF::String> data;
	HashSet<WTF::String>::iterator data_iterator;
	supportedExtensionList.clear();

	if(createWebCLPlatformList()) {
		CLLOG(INFO) << "platformList Size : " << mClPlatformMap.size();

		for(WebCLPlatformMap::iterator iter = mClPlatformMap.begin(); iter != mClPlatformMap.end(); ++iter) {
			WebCLPlatform *platform = iter->value;
			WTF::Vector<WTF::String> platform_extensions = platform->getSupportedExtensions(ec);
			CLLOG(INFO) << "platform_extensions.size : " << platform_extensions.size();

			for(j = 0; j < (int)platform_extensions.size(); j++) {
				CLLOG(INFO) << "platform_extensions[" << j  << "] : " << &platform_extensions[j];

				data.add(platform_extensions[j]);
			}
		}

		for(data_iterator = data.begin(); data_iterator != data.end(); ++data_iterator) {
			supportedExtensionList.append(*data_iterator);
			CLLOG(INFO) << "supportedExtensionList.append(" << &data_iterator;
		}

		CLLOG(INFO) << "supportedExtensionList.size : " << supportedExtensionList.size();

		return WebCLException::SUCCESS;
	}
	else
		return WebCLException::FAILURE;
}

CLboolean WebCL::enableExtension(const String& extensionName, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::enableExtension, enableExtension is called, extensionName : " << extensionName.utf8().data();
	CLboolean result = false;

	CLLOG(INFO) << "WebCL::enableExtension, deviceName : " << deviceName.utf8().data();
	String extensionNameLocal = extensionName;
	if(deviceName == "Nexus 7" && extensionName == "KHR_gl_sharing") {
		extensionNameLocal = "cl_khr_gl_sharing";
		CLLOG(INFO) << "WebCL::enableExtension, adjust extensionName : " << extensionNameLocal.utf8().data();
	}

	if (0 == supportedExtensionList.size()) {
		CLLOG(INFO) << "WebCL::enableExtension, supportedExtensionList is null. call initSupportedExtensions";
		initSupportedExtension(ec);
	}

	if (WebCLException::SUCCESS != initSupportedExtensionState) {
		CLLOG(INFO) << "WebCL::enableExtension, initSupportedExtensions error : " << initSupportedExtensionState;
		return result;
	}

	for(WebCLPlatformMap::iterator iter = mClPlatformMap.begin(); iter != mClPlatformMap.end(); ++iter)
	{
		WebCLPlatform* platform = iter->value;
		if (platform->enableExtension(extensionNameLocal, ec)) {
			if (!enableExtensionList.contains(extensionNameLocal)) {
				enableExtensionList.append(extensionNameLocal);
			}
			result = true;
		}
		HeapVector<Member<WebCLDevice>> deviceList = platform->getDevices(ec);
		for(unsigned int i = 0; i < deviceList.size(); i++)
		{
			WebCLDevice* device = deviceList[i];
			if (device->enableExtension(extensionNameLocal, ec)) {
				if (!enableExtensionList.contains(extensionNameLocal)) {
					enableExtensionList.append(extensionNameLocal);
					if (extensionNameLocal == "KHR_fp16") {
						enableExtensionList.append("cl_khr_fp16");
					} else if(extensionNameLocal == "KHR_fp64") {
						enableExtensionList.append("cl_khr_fp64");
					}
				}
				result = true;
			}
		}
	}

	return result;
}

// gl/cl sharing
WebCLContext* WebCL::createContext(WebGLRenderingContext* gl, ExceptionState& ec) {
	CLLOG(INFO) << "WebCL::createContext, gl : " << gl;

	String extensionName = deviceName == "Nexus 7" ? "cl_khr_gl_sharing" : "KHR_gl_sharing";
	// String extensionName = "cl_khr_gl_sharing";
	if(!enableExtensionList.contains(extensionName)) {
		ec.throwDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return NULL;
	}

	return createContext(gl, WebCL::DEVICE_TYPE_DEFAULT, ec);
}

WebCLContext* WebCL::createContext(WebGLRenderingContext* gl, unsigned int deviceType, ExceptionState& ec) {
	CLLOG(INFO) << "WebCL::createContext, gl : " << gl << ", deviceType : " << deviceType;

	String extensionName = deviceName == "Nexus 7" ? "cl_khr_gl_sharing" : "KHR_gl_sharing";
	// String extensionName = "cl_khr_gl_sharing";
	if(!enableExtensionList.contains(extensionName)) {
		ec.throwDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return NULL;
	}

	mGL = gl;
	return createContext(deviceType, ec);
}

WebCLContext* WebCL::createContext(WebGLRenderingContext* gl, WebCLPlatform* platform, ExceptionState& ec) {
	CLLOG(INFO) << "WebCL::createContext, gl : " << gl << ", platform : " << platform;

	String extensionName = deviceName == "Nexus 7" ? "cl_khr_gl_sharing" : "KHR_gl_sharing";
	// String extensionName = "cl_khr_gl_sharing";
	if(!enableExtensionList.contains(extensionName)) {
		ec.throwDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return NULL;
	}

	return createContext(gl, platform, WebCL::DEVICE_TYPE_DEFAULT, ec);
}

WebCLContext* WebCL::createContext(WebGLRenderingContext* gl, WebCLPlatform* platform, unsigned int deviceType, ExceptionState& ec) {
	CLLOG(INFO) << "WebCL::createContext, gl : " << gl << ", platform : " << platform << ", deviceType : " << deviceType;

	String extensionName = deviceName == "Nexus 7" ? "cl_khr_gl_sharing" : "KHR_gl_sharing";
	// String extensionName = "cl_khr_gl_sharing";
	if(!enableExtensionList.contains(extensionName)) {
		ec.throwDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return NULL;
	}

	mGL = gl;
	return createContext(platform, deviceType, ec);
}

WebCLContext* WebCL::createContext(WebGLRenderingContext* gl, WebCLDevice* device, ExceptionState& ec) {
	CLLOG(INFO) << "WebCL::createContext, gl : " << gl << ", device : " << device;

	String extensionName = deviceName == "Nexus 7" ? "cl_khr_gl_sharing" : "KHR_gl_sharing";
	// String extensionName = "cl_khr_gl_sharing";
	if(!enableExtensionList.contains(extensionName)) {
		ec.throwDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return NULL;
	}

	mGL = gl;
	return createContext(device, ec);
}

WebCLContext* WebCL::createContext(WebGLRenderingContext* gl, HeapVector<Member<WebCLDevice>> devices, ExceptionState& ec) {
	CLLOG(INFO) << "WebCL::createContext, gl : " << gl << ", devices.size() : " << devices.size();

	String extensionName = deviceName == "Nexus 7" ? "cl_khr_gl_sharing" : "KHR_gl_sharing";
	// String extensionName = "cl_khr_gl_sharing";
	if(!enableExtensionList.contains(extensionName)) {
		ec.throwDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
		return NULL;
	}

	mGL = gl;
	return createContext(devices, ec);
}

WebCLContext* WebCL::createContext(ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::createContext";

 	return createContext(WebCL::DEVICE_TYPE_DEFAULT, ec);
}

WebCLContext* WebCL::createContext(unsigned int deviceType, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::createContext, deviceType : " << deviceType;

	if(deviceType == 0) {
		deviceType = WebCL::DEVICE_TYPE_DEFAULT;
	}

	if(!WebCLValueChecker::isDeviceTypeValid(deviceType)) {
		CLLOG(INFO) <<"Error: INVALID_DEVICE_TYPE, invalid device type=" << deviceType;
		ec.throwDOMException(WebCLException::INVALID_DEVICE_TYPE, "WebCLException::INVALID_DEVICE_TYPE");

		return NULL;
	}

	getPlatforms(ec);
	if(mClPlatformMap.size() <= 0) {
		CLLOG(INFO) <<"Error: CL_INVALID_PLATFORM";
		ec.throwDOMException(WebCLException::INVALID_PLATFORM, "WebCLException::INVALID_PLATFORM");

		return NULL;
	}

	WebCLPlatformMap::iterator iter = mClPlatformMap.begin();

	WebCLPlatform* platform = iter->value;

	if(platform->getDevices(deviceType, ec).size() <= 0) {
		CLLOG(INFO) <<"Error: DEVICE_NOT_FOUND, device type=" << deviceType << "can't find any devices on selected platform.";
		ec.throwDOMException(WebCLException::DEVICE_NOT_FOUND, "WebCLException::INVALID_DEVICE_TYPE");

		return NULL;
	}

	cl_int err = 0;
	cl_context cl_context_id = 0;

	// gl/cl sharing
	Vector<cl_context_properties> contextProperties;
	contextProperties.append(CL_CONTEXT_PLATFORM);
	contextProperties.append((cl_context_properties)platform->getPlatformId());

	CLLOG(INFO) << "CL_GL_CONTEXT_KHR, mGL : " << mGL;

	if(mGL) {
		bool enableGlSharing = true;
		for(iter = mClPlatformMap.begin(); iter != mClPlatformMap.end(); ++iter) {
			WebCLPlatform* platform = iter->value;
			HeapVector<Member<WebCLDevice>> platformDevices = platform->getDevices(deviceType, ec);
			for(unsigned i = 0; i < platformDevices.size(); i++) {
				if (!platformDevices[i].get()->getEnableExtensionList().contains("cl_khr_gl_sharing")) {
					enableGlSharing = false;
					break;
				}
			}
		}

		if (enableGlSharing) {
			cl_point glContext = webcl_getGLContext(webcl_channel_);
			cl_point glDisplay = webcl_getGLDisplay(webcl_channel_);
			CLLOG(INFO) << "glContext : " << glContext << ", glDisplay : " << glDisplay;

			contextProperties.append(CL_GL_CONTEXT_KHR);
			contextProperties.append((cl_context_properties)glContext);
			contextProperties.append(CL_EGL_DISPLAY_KHR);
			contextProperties.append((cl_context_properties)glDisplay);
		}
	}

	contextProperties.append(0);

	if(deviceType == DEVICE_TYPE_GPU)
		cl_context_id = webcl_clCreateContextFromType(webcl_channel_, contextProperties.data(), CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
	else if(deviceType == DEVICE_TYPE_CPU)
		cl_context_id = webcl_clCreateContextFromType(webcl_channel_, contextProperties.data(), CL_DEVICE_TYPE_CPU, NULL, NULL, &err);
	else if(deviceType == DEVICE_TYPE_ACCELERATOR)
		cl_context_id = webcl_clCreateContextFromType(webcl_channel_, contextProperties.data(), CL_DEVICE_TYPE_ACCELERATOR, NULL, NULL, &err);
	else
		cl_context_id = webcl_clCreateContextFromType(webcl_channel_, contextProperties.data(), CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &err);

	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	}
	else {
		CLLOG(INFO) << "created context = " << cl_context_id;

		WebCLContext* context = WebCLContext::create(this, cl_context_id);
		context->initializeContextDevice(ec);

		context->setEnableExtensionList(platform->getEnableExtensionList());
		context->setGLContext(mGL);
		mGL = nullptr;

		// WebCLContextMap::AddResult result = mClContextMap.set((cl_obj_key)cl_context_id, context);
		mClContextMap.set((cl_obj_key)cl_context_id, context);

		return context;
	}
	mGL = nullptr;
	return NULL;
}

WebCLContext* WebCL::createContext(WebCLPlatform* platform, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::createContext, platform : " << platform;

	return createContext(platform, WebCL::DEVICE_TYPE_DEFAULT, ec);
}

WebCLContext* WebCL::createContext(WebCLPlatform* platform, unsigned int deviceType, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::createContext, platform : " << platform << ", deviceType : " << deviceType;

	if (platform == nullptr) {
		CLLOG(INFO) << "Error:CL_INVALID_PLATFORM";
		ec.throwDOMException(WebCLException::INVALID_PLATFORM, "WebCLException::INVALID_PLATFORM");
	}

	if(deviceType == 0) {
		deviceType = WebCL::DEVICE_TYPE_DEFAULT;
	}

	if(platform->getPlatformId() == NULL) {
		CLLOG(INFO) <<"Error: CL_INVALID_PLATFORM, platformId is null.";
		ec.throwDOMException(WebCLException::INVALID_PLATFORM, "WebCLException::INVALID_PLATFORM");
		return NULL;
	}

	if(!WebCLValueChecker::isDeviceTypeValid(deviceType)) {
		CLLOG(INFO) <<"Error: INVALID_DEVICE_TYPE, invalid device type=" << deviceType;
		ec.throwDOMException(WebCLException::INVALID_DEVICE_TYPE, "WebCLException::INVALID_DEVICE_TYPE");
		return NULL;
	}

	HeapVector<Member<WebCLDevice>> deviceList = platform->getDevices(deviceType, ec);
	if(deviceList.size() <= 0) {
		CLLOG(INFO) <<"Error: DEVICE_NOT_FOUND, device type=" << deviceType << " can't find any devices on selected platform.";
		ec.throwDOMException(WebCLException::DEVICE_NOT_FOUND, "WebCLException::DEVICE_NOT_FOUND");
		return NULL;
	}

	cl_int err = 0;
	cl_context cl_context_id = 0;

	// gl/cl sharing
	Vector<cl_context_properties> contextProperties;
	contextProperties.append(CL_CONTEXT_PLATFORM);
	contextProperties.append((cl_context_properties)platform->getPlatformId());

	CLLOG(INFO) << "CL_GL_CONTEXT_KHR, mGL : " << mGL;

	if(mGL) {
		bool enableGlSharing = true;
		HeapVector<Member<WebCLDevice>> platformDevices = platform->getDevices(deviceType, ec);
		for(unsigned i = 0; i < platformDevices.size(); i++) {
			if (!platformDevices[i].get()->getEnableExtensionList().contains("cl_khr_gl_sharing")) {
				enableGlSharing = false;
				break;
			}
		}
		if (enableGlSharing) {
			cl_point glContext = webcl_getGLContext(webcl_channel_);
			cl_point glDisplay = webcl_getGLDisplay(webcl_channel_);
			CLLOG(INFO) << "glContext : " << glContext << ", glDisplay : " << glDisplay;

			contextProperties.append(CL_GL_CONTEXT_KHR);
			contextProperties.append((cl_context_properties)glContext);
			contextProperties.append(CL_EGL_DISPLAY_KHR);
			contextProperties.append((cl_context_properties)glDisplay);
		}
	}

	contextProperties.append(0);

	if(deviceType == DEVICE_TYPE_GPU)
		cl_context_id = webcl_clCreateContextFromType(webcl_channel_, contextProperties.data(), CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
	else if(deviceType == DEVICE_TYPE_CPU)
		cl_context_id = webcl_clCreateContextFromType(webcl_channel_, contextProperties.data(), CL_DEVICE_TYPE_CPU, NULL, NULL, &err);
	else if(deviceType == DEVICE_TYPE_ACCELERATOR)
		cl_context_id = webcl_clCreateContextFromType(webcl_channel_, contextProperties.data(), CL_DEVICE_TYPE_ACCELERATOR, NULL, NULL, &err);
	else
		cl_context_id = webcl_clCreateContextFromType(webcl_channel_, contextProperties.data(), CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &err);

	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	}
	else {
		CLLOG(INFO) << "created context = " << cl_context_id;

		WebCLContext* context = WebCLContext::create(this, cl_context_id);
		context->initializeContextDevice(ec);

		//set enabled extension list
		context->setEnableExtensionList(platform->getEnableExtensionList());
		context->setGLContext(mGL);
		mGL = nullptr;

		// WebCLContextMap::AddResult result = mClContextMap.set((cl_obj_key)cl_context_id, context);
		mClContextMap.set((cl_obj_key)cl_context_id, context);

		return context;
	}
	mGL = nullptr;
	return NULL;
}

WebCLContext* WebCL::createContext(WebCLDevice* device, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::createContext, device : " << device;

	if(device->getCLDevice() == NULL) {
		CLLOG(INFO) <<"Error: INVALID_DEVICE, cl_deivce_id is null";
		ec.throwDOMException(WebCLException::INVALID_DEVICE, "WebCLException::INVALID_DEVICE");

		return NULL;
	}

	CLLOG(INFO) <<"WebCL::createContext, device=" << device->getCLDevice();

	cl_int err = 0;
	cl_context cl_context_id = 0;

	cl_uint num_devices = 1;
	cl_device_id* devices = new cl_device_id[num_devices];
	devices[0] = device->getCLDevice();

	// gl/cl sharing
	WebCLPlatformMap::iterator iter = mClPlatformMap.begin();
	WebCLPlatform* platform = iter->value;

	Vector<cl_context_properties> contextProperties;
	contextProperties.append(CL_CONTEXT_PLATFORM);
	contextProperties.append((cl_context_properties)platform->getPlatformId());
	CLLOG(INFO) << "CL_GL_CONTEXT_KHR, mGL : " << mGL;

	if(mGL && device->getEnableExtensionList().contains("cl_khr_gl_sharing")) {
		cl_uint glContext = webcl_getGLContext(webcl_channel_);
		cl_uint glDisplay = webcl_getGLDisplay(webcl_channel_);
		CLLOG(INFO) << "glContext : " << glContext << ", glDisplay : " << glDisplay;

		contextProperties.append(CL_GL_CONTEXT_KHR);
		contextProperties.append((cl_context_properties)glContext);
		contextProperties.append(CL_EGL_DISPLAY_KHR);
		contextProperties.append((cl_context_properties)glDisplay);
	}
	contextProperties.append(0);

	cl_context_id = webcl_clCreateContext(webcl_channel_, contextProperties.data(), num_devices, devices, NULL, NULL, &err);

	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	}
	else {
		CLLOG(INFO) << "created context = " << cl_context_id;

		WebCLContext* context = WebCLContext::create(this, cl_context_id);
		context->initializeContextDevice(ec);

		context->setEnableExtensionList(device->getEnableExtensionList());
		context->setGLContext(mGL);
		mGL = nullptr;

		// WebCLContextMap::AddResult result = mClContextMap.set((cl_obj_key)cl_context_id, context);
		mClContextMap.set((cl_obj_key)cl_context_id, context);

		return context;
	}
	mGL = nullptr;
	return NULL;
}

WebCLContext* WebCL::createContext(HeapVector<Member<WebCLDevice>> devices, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::createContext, devices.size() : " << devices.size();

	cl_int err = 0;
	cl_context cl_context_id = 0;

	cl_uint num_devices = devices.size();
	cl_device_id* deviceIds = new cl_device_id[num_devices];
	for(unsigned i = 0 ; i<num_devices; i++) {
		if(devices[i].get()->getCLDevice() == NULL) {
			CLLOG(INFO) <<"Error: INVALID_DEVICE, cl_deivce_id is null";
			ec.throwDOMException(WebCLException::INVALID_DEVICE, "WebCLException::INVALID_DEVICE");
			return NULL;
		}

		CLLOG(INFO) << "DEVICE[" << i << "]:" << devices[i].get()->getCLDevice();
		deviceIds[i] = devices[i].get()->getCLDevice();
	}

	// gl/cl sharing
	WebCLPlatformMap::iterator iter = mClPlatformMap.begin();
	WebCLPlatform* platform = iter->value;

	Vector<cl_context_properties> contextProperties;
	contextProperties.append(CL_CONTEXT_PLATFORM);
	contextProperties.append((cl_context_properties)platform->getPlatformId());
	CLLOG(INFO) << "CL_GL_CONTEXT_KHR, mGL : " << mGL;

	if(mGL) {
		bool enableGlSharing = true;
		for(unsigned i = 0; i < devices.size(); i++) {
			if (!devices[i].get()->getEnableExtensionList().contains("cl_khr_gl_sharing")) {
				enableGlSharing = false;
				break;
			}
		}
		if (enableGlSharing) {
			// gl/cl sharing
			cl_uint glContext = webcl_getGLContext(webcl_channel_);
			cl_uint glDisplay = webcl_getGLDisplay(webcl_channel_);
			CLLOG(INFO) << "glContext : " << glContext << ", glDisplay : " << glDisplay;
			contextProperties.append(CL_GL_CONTEXT_KHR);
			contextProperties.append((cl_context_properties)glContext);
			contextProperties.append(CL_EGL_DISPLAY_KHR);
			contextProperties.append((cl_context_properties)glDisplay);
		}
	}

	contextProperties.append(0);

	cl_context_id = webcl_clCreateContext(webcl_channel_, contextProperties.data(), num_devices, deviceIds, NULL, NULL, &err);

	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	}
	else {
		CLLOG(INFO) << "created context = " << cl_context_id;

		WebCLContext* context = WebCLContext::create(this, cl_context_id);
		context->initializeContextDevice(ec);

		Vector<WTF::String> tmpList;
		Vector<WTF::String> enabledExtensionList;
		for(size_t i=0; i<devices.size(); i++) {
			tmpList = devices[i]->getEnableExtensionList();
			for(size_t j=0; j<tmpList.size(); j++) {
				if(!enabledExtensionList.contains(tmpList[j]))
					enabledExtensionList.append(tmpList[j]);
			}
		}

		context->setEnableExtensionList(enabledExtensionList);
		context->setGLContext(mGL);
		mGL = nullptr;

		// WebCLContextMap::AddResult result = mClContextMap.set((cl_obj_key)cl_context_id, context);
		mClContextMap.set((cl_obj_key)cl_context_id, context);

		return context;
	}
	mGL = nullptr;
	return NULL;
}

void WebCL::waitForEvents(HeapVector<Member<WebCLEvent>> eventWaitList, WebCLCallback* whenFinished, ExceptionState& ec)
{
	if(eventWaitList.size() <= 0) {
		CLLOG(INFO) <<"eventWaitList is not valid.";
		ec.throwDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	int eventListSize = eventWaitList.size();
	for(int i=0; i<eventListSize; i++) {
		WebCLEvent* event = eventWaitList[i].get();
		if(event == nullptr
				|| (whenFinished == nullptr && (event->isUserEvent() || event->getCLEvent() == nullptr))) {
			CLLOG(INFO) <<"eventWaitList is not valid.";
			ec.throwDOMException(WebCLException::INVALID_EVENT_WAIT_LIST, "WebCLException::INVALID_EVENT_WAIT_LIST");
			return;
		}
		int executionState = event->getEventCommandExecutionStatus(ec);
		if (executionState < CL_SUCCESS || ec.hadException()) {
			CLLOG(INFO) <<"eventWaitList is not valid. EXECUTION_STATUS";
			ec.throwDOMException(WebCLException::INVALID_EVENT_WAIT_LIST, "WebCLException::INVALID_EVENT_WAIT_LIST");
			return;
		}
	}

	if(!whenFinished) {
		cl_event* evnetListInter = new cl_event[eventWaitList.size()];
		for(size_t i=0; i<eventWaitList.size(); i++) {
			evnetListInter[i] = eventWaitList[i].get()->getCLEvent();
		}

		cl_int errcode = webcl_clWaitForEvents(webcl_channel_, eventWaitList.size(), evnetListInter);

		if (errcode != CL_SUCCESS) {
			WebCLException::throwException(errcode, ec);
		}
	}
	else {
		for(auto event : eventWaitList) {
			event.get()->setCallback(WebCL::COMPLETE, whenFinished, ec);
		}
	}
}

bool WebCL::createWebCLPlatformList()
{
	CLLOG(INFO) << "createWebCLPlatformList";

	if(mClPlatformMap.size() > 0)
		return true;

	cl_int err = 0;

	err = webcl_getPlatformIDs(webcl_channel_, 0, NULL, &m_num_platforms);
	if (err != CL_SUCCESS) {
		return false;
	}

	m_cl_platforms = new cl_platform_id[m_num_platforms];
	err = webcl_getPlatformIDs(webcl_channel_, m_num_platforms, m_cl_platforms, NULL);
	if (err != CL_SUCCESS) {
		return false;
	}

	for (unsigned int i = 0 ; i < m_num_platforms; i++) {
		Member<WebCLPlatform> o = WebCLPlatform::create(this, m_cl_platforms[i]);
		if (o != NULL) {
			mClPlatformMap.set((cl_obj_key)m_cl_platforms[i], o);
		} else {
			return false;
		}
	}

	return true;
}

WebCLContext* WebCL::findCLContext(cl_obj_key key)
{
	CLLOG(INFO) << "WebCL::findCLContext, context key=" << key;

	if (key == 0) {
		return nullptr;
	}

	if(!mClContextMap.contains(key))
		return nullptr;

	Member<WebCLContext> context = mClContextMap.get(key);
	return context.get();
}

void WebCL::deleteCLContext(cl_obj_key key)
{
	CLLOG(INFO) << "WebCL::deleteCLContext, context key=" << key;
	if (!isReleasing) {
		if (mClContextMap.contains(key)) {
			mClContextMap.remove(key);
		}
	}
}

Member<WebCLPlatform> WebCL::findCLPlatform(cl_obj_key key, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::findCLPlatform, platform_id = " << key;

	if (key == 0) {
		CLLOG(INFO) << "WebCL::findCLPlatform, input key is NULL";
		return nullptr;
	}

	if (mClPlatformMap.size() <= 0) {
		getPlatforms(ec);
	}

	Member<WebCLPlatform> platform = nullptr;

	if (mClPlatformMap.contains(key)) {
		platform = mClPlatformMap.get(key);
	}

	if (platform == nullptr) {
		CLLOG(INFO) << "WebCL::findCLPlatform, platform_id not avaliable. return null";
	}

	return platform;
}

Member<WebCLDevice> WebCL::findCLDevice(cl_obj_key key, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCL::findCLDevice, device_id = " << key;

	if (key == 0) {
		CLLOG(INFO) << "WebCL::findCLDevice input key is NULL";
		return nullptr;
	}

	Member<WebCLDevice> device = nullptr;
	WebCLPlatform* platform;

	if (mClPlatformMap.size() <= 0) {
		getPlatforms(ec);
	}

	for(WebCLPlatformMap::iterator iter = mClPlatformMap.begin(); iter != mClPlatformMap.end(); ++iter)
	{
		platform = iter->value;
		if (platform->containDevice(key, ec)) {
			CLLOG(INFO) << "WebCL::findCLDevice, platform has key device";
			device = platform->findCLDevice(key, ec);
			break;
		}
	}

	return device;
}

Member<WebCLCommandQueue> WebCL::findCLcommandQueue(cl_obj_key key)
{
	if (key == 0) {
		return nullptr;
	}

	Member<WebCLCommandQueue> matchedCommandQueue = nullptr;

	WebCLContext* context = nullptr;
	for(WebCLContextMap::iterator iter = mClContextMap.begin(); iter != mClContextMap.end(); ++iter)
	{
		context = iter->value;
		if ((matchedCommandQueue = context->findCLCommandQueue(key)) != nullptr) {
			break;
		}
	}

	return matchedCommandQueue;
}

bool WebCL::isAPIBlocked(WTF::String api)
{
	if(mApiChecker) {
		return mApiChecker->isAPIBlocked(api);
	}

	return false;
}

void WebCL::startHandling()
{
	if(!mOperationHandler->canShareOperation()) {
		mOperationHandler->startHandling();
	}
}

void WebCL::setOperationParameter(WebCL_Operation_Base* paramPtr)
{
	mOperationHandler->setOperationParameter(paramPtr);
}

void WebCL::setOperationData(void* dataPtr, size_t sizeInBytes)
{
	mOperationHandler->setOperationData(dataPtr, sizeInBytes);
}

void WebCL::setOperationEvents(cl_point* events, size_t numEvents)
{
	mOperationHandler->setOperationEvents(events, numEvents);
}

void WebCL::getOperationResult(WebCL_Result_Base* resultPtr)
{
	mOperationHandler->getOperationResult(resultPtr);
}

void WebCL::sendOperationSignal(int operation)
{
	mOperationHandler->sendOperationSignal(operation);
}

void WebCL::getOperationResultData(void* resultDataPtr, size_t sizeInBytes)
{
	mOperationHandler->getOperationResultData(resultDataPtr, sizeInBytes);
}

bool WebCL::isEnableExtension(String extensionName) {
	return enableExtensionList.contains(extensionName);
}

DEFINE_TRACE(WebCL) {
	visitor->trace(mClPlatformMap);
	visitor->trace(mClContextMap);
	visitor->trace(mGL);
}

} // namespace blink
