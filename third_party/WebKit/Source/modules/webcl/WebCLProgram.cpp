// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"
#include "platform/bindings/V8Binding.h"
#include "bindings/modules/v8/V8WebCLProgram.h"

#include "WebCLProgram.h"
#include "WebCLKernel.h"
#include "WebCL.h"
#include "WebCLCallback.h"
#include "WebCLPlatform.h"
#include "WebCLContext.h"
#include "core/dom/custom/WebCL/WebCLException.h"

namespace blink {

static bool isASCIILineBreakOrWhiteSpaceCharacter(UChar c)
{
	return c == '\r' || c == '\n' || c == ' ';
}

WebCLProgram::WebCLProgram(WebCL* context,
		cl_program program) : mContext(context), mClProgram(program)
{
	mBuildCallback = nullptr;
}

WebCLProgram::~WebCLProgram()
{
}

ScriptValue WebCLProgram::getInfo(ScriptState* scriptState, CLenum paramName, ExceptionState& ec)
{
	v8::Handle<v8::Object> creationContext = scriptState->GetContext()->Global();
	v8::Isolate* isolate = scriptState->GetIsolate();

	cl_int err = 0;
	cl_uint uint_units = 0;
	size_t sizet_units = 0;
	char program_string[4096];
	// HeapVector<Member<WebCLDevice>> deviceList;
	Member<WebCLDevice> deviceObj = nullptr;
	size_t szParmDataBytes = 0;
	if (mClProgram == NULL) {
			ec.ThrowDOMException(WebCLException::INVALID_PROGRAM, "WebCLException::INVALID_PROGRAM");
			printf("Error: Invalid program object\n");
			return ScriptValue(scriptState, v8::Null(isolate));
	}

	switch(paramName)
	{
		case WebCL::PROGRAM_REFERENCE_COUNT:
			err = webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
			break;
		case WebCL::PROGRAM_NUM_DEVICES:
			err = webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_NUM_DEVICES , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
			break;
		case WebCL::PROGRAM_BINARY_SIZES:
			err = webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &sizet_units, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizet_units)));
			break;
		case WebCL::PROGRAM_SOURCE:
			err = webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_SOURCE, sizeof(program_string), &program_string, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, V8String(isolate, String(program_string)));
			break;
		case WebCL::PROGRAM_BINARIES:
			err = webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_BINARIES, sizeof(program_string), &program_string, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, V8String(isolate, String(program_string)));
			break;
		case WebCL::PROGRAM_CONTEXT: {
				Persistent<WebCLContext> passContextObj = mClContext.Get();
				return ScriptValue(scriptState, ToV8(passContextObj, creationContext, isolate));
			}
			break;
		case WebCL::PROGRAM_DEVICES:
			if(mDeviceList.size() == 0) {
				cl_device_id* cdDevices;
				err = webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_DEVICES, 0, NULL, &szParmDataBytes);
				if (err == CL_SUCCESS) {
					cdDevices = (cl_device_id*) malloc(szParmDataBytes);
					webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_DEVICES, szParmDataBytes, cdDevices, NULL);
					for(unsigned int i = 0; i < szParmDataBytes; i++) {
						deviceObj = mContext->findCLDevice((cl_obj_key)(cdDevices[i]), ec);
						if (deviceObj == nullptr) {
							CLLOG(INFO) << "deviceObj is NULL";
							continue;
						}
						mDeviceList.push_back(deviceObj.Get());
					}
					free(cdDevices);

				}
			}
			return ScriptValue(scriptState, ToV8(mDeviceList, creationContext, isolate));
			break;
		default:
			ec.ThrowDOMException(WebCLException::INVALID_PROGRAM, "WebCLException::INVALID_PROGRAM");
			return ScriptValue(scriptState, v8::Null(isolate));
	}
	WebCLException::throwException(err, ec);
	return ScriptValue(scriptState, v8::Null(isolate));
}

ScriptValue WebCLProgram::getBuildInfo(ScriptState* scriptState, WebCLDevice* device, CLenum param_name, ExceptionState& ec)
{
	v8::Isolate* isolate = scriptState->GetIsolate();

	cl_device_id device_id = NULL;
	cl_int err = 0;
	char buffer[8192];
	size_t len = 0;

	if (mClProgram == NULL) {
			printf("Error: Invalid program object\n");
			ec.ThrowDOMException(WebCLException::INVALID_PROGRAM, "WebCLException::INVALID_PROGRAM");
			return ScriptValue(scriptState, v8::Null(isolate));
	}
	if (device != NULL) {
		device_id = device->getCLDevice();
		if (device_id == NULL) {
			ec.ThrowDOMException(WebCLException::INVALID_DEVICE, "WebCLException::INVALID_DEVICE");
			printf("Error: device_id null\n");
			return ScriptValue(scriptState, v8::Null(isolate));
		}
	}

	switch (param_name) {
		case WebCL::PROGRAM_BUILD_LOG:
			err = webcl_clGetProgramBuildInfo(webcl_channel_, mClProgram, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, V8String(isolate, String(buffer)));
			break;
		case WebCL::PROGRAM_BUILD_OPTIONS:
			err = webcl_clGetProgramBuildInfo(webcl_channel_, mClProgram, device_id, CL_PROGRAM_BUILD_OPTIONS, sizeof(buffer), &buffer, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, V8String(isolate, String(buffer)));
			break;
		case WebCL::PROGRAM_BUILD_STATUS:
			cl_build_status build_status;
			err = webcl_clGetProgramBuildInfo(webcl_channel_, mClProgram, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(build_status)));
			break;
		default:
			ec.ThrowDOMException(WebCLException::INVALID_PROGRAM, "WebCLException::INVALID_PROGRAM");
			return ScriptValue(scriptState, v8::Null(isolate));
	}
	WebCLException::throwException(err, ec);
	return ScriptValue(scriptState, v8::Null(isolate));
}

void WebCLProgram::build(const HeapVector<Member<WebCLDevice>>& devices, const String& buildOptions, WebCLCallback* callback, ExceptionState& ec)
{
	if (mClProgram == NULL) {
		ec.ThrowDOMException(WebCLException::INVALID_PROGRAM, "WebCLException::INVALID_PROGRAM");
		return;
	}

	//[jphong] Check defined extension usage in kernel source.
	CLLOG(INFO) << programSource.Latin1().data();
	size_t extension_label = programSource.Find("OPENCL EXTENSION", 0);
	if(extension_label != WTF::kNotFound) {
		if(mDeviceList.size() == 0) {
			Member<WebCLDevice> deviceObj = nullptr;
			cl_int err = -1;
			cl_device_id* cdDevices;
			size_t szParmDataBytes = 0;
			err = webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_DEVICES, 0, NULL, &szParmDataBytes);
			if (err == CL_SUCCESS) {
				cdDevices = (cl_device_id*) malloc(szParmDataBytes);
				err = webcl_clGetProgramInfo(webcl_channel_, mClProgram, CL_PROGRAM_DEVICES, szParmDataBytes, cdDevices, NULL);
				for(unsigned int i = 0; i < szParmDataBytes; i++) {
					deviceObj = mContext->findCLDevice((cl_obj_key)(cdDevices[i]), ec);
					if (deviceObj == nullptr) {
						CLLOG(INFO) << "deviceObj is NULL";
						continue;
					}
					mDeviceList.push_back(deviceObj.Get());
				}
				free(cdDevices);
			}
		}

		CLLOG(INFO) << "kernel source defined extension!!, device size=" << mDeviceList.size();

		size_t extension_kh16 = programSource.Find("cl_khr_fp16", 0);
		size_t extension_kh64 = programSource.Find("cl_khr_fp64", 0);

		CLLOG(INFO) << extension_kh16 << "," << extension_kh64;

		bool extensionEnabled = false;
		size_t num_device = mDeviceList.size();
		for(size_t i=0; i<num_device; i++) {
			if((extension_kh16 != WTF::kNotFound && mDeviceList[i]->getEnableExtensionList().Contains("KHR_fp16"))
				|| (extension_kh64 != WTF::kNotFound && mDeviceList[i]->getEnableExtensionList().Contains("KHR_fp64"))) {
				extensionEnabled = true;
				break;
			}
		}

		if(!extensionEnabled) {
			ec.ThrowDOMException(WebCLException::WEBCL_EXTENSION_NOT_ENABLED, "WebCLException::WEBCL_EXTENSION_NOT_ENABLED");
			return;
		}
	}

	size_t kernel_label = programSource.Find("__kernel ", 0);
	while (kernel_label != WTF::kNotFound) {
		size_t openBrace = programSource.Find("{", kernel_label);
		size_t openBraket = programSource.ReverseFind("(", openBrace);
		size_t space = programSource.ReverseFind(" ", openBraket);
		String kernelName = programSource.Substring(space + 1, openBraket - space - 1);

		if (kernelName.length() > 254) {
			// Kernel Name length isn't allowed larger than 255.
			ec.ThrowDOMException(WebCLException::BUILD_PROGRAM_FAILURE, "WebCLException::BUILD_PROGRAM_FAILURE");
			return;
		}

		size_t closeBraket = programSource.Find(")", openBraket);
		String arguments = programSource.Substring(openBraket + 1, closeBraket - openBraket - 1);
		if (arguments.Contains("struct ") || arguments.Contains("image1d_array_t ") || arguments.Contains("image1d_buffer_t ") || arguments.Contains("image1d_t ") || arguments.Contains("image2d_array_t ")) {
			// 1. Kernel structure parameters aren't allowed;
			// 2. Kernel argument "image1d_t", "image1d_array_t", "image2d_array_t" and "image1d_buffer_t" aren't allowed;
			ec.ThrowDOMException(WebCLException::BUILD_PROGRAM_FAILURE, "WebCLException::BUILD_PROGRAM_FAILURE");
			return;
		}

		size_t closeBrace = programSource.Find("}", openBrace);
		String codeString =  programSource.Substring(openBrace + 1, closeBrace - openBrace - 1).RemoveCharacters(isASCIILineBreakOrWhiteSpaceCharacter);
		if (codeString.IsEmpty()) {
			// Kernel code isn't empty;
			ec.ThrowDOMException(WebCLException::BUILD_PROGRAM_FAILURE, "WebCLException::BUILD_PROGRAM_FAILURE");
			return;
		}

		kernel_label = programSource.Find("__kernel ", closeBrace);
	}

	if (buildOptions.length() > 0) {
		static AtomicString& buildOptionDashD = *new AtomicString("-D"/*, AtomicString::ConstructFromLiteral*/);
		static HashSet<AtomicString>& webCLSupportedBuildOptions = *new HashSet<AtomicString>();
		if (webCLSupportedBuildOptions.IsEmpty()) {
			webCLSupportedBuildOptions.insert(AtomicString("-cl-opt-disable"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-cl-single-precision-constant"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-cl-denorms-are-zero"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-cl-mad-enable"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-cl-no-signed-zeros"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-cl-unsafe-math-optimizations"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-cl-finite-math-only"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-cl-fast-relaxed-math"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-w"/*, AtomicString::ConstructFromLiteral*/));
			webCLSupportedBuildOptions.insert(AtomicString("-Werror"/*, AtomicString::ConstructFromLiteral*/));
		}

		Vector<String> webCLBuildOptionsVector;
		buildOptions.Split(" ", false, webCLBuildOptionsVector);

		for (size_t i = 0; i < webCLBuildOptionsVector.size(); i++) {
			// Every build option must start with a hyphen.
			if (!webCLBuildOptionsVector[i].StartsWith("-")) {
				ec.ThrowDOMException(WebCLException::INVALID_BUILD_OPTIONS, "WebCLException::INVALID_BUILD_OPTIONS");
				return;
			}

			if (webCLSupportedBuildOptions.Contains(AtomicString(webCLBuildOptionsVector[i])))
				continue;

			if (webCLBuildOptionsVector[i].StartsWith(buildOptionDashD)) {
				size_t j;
				for (j = i + 1; j < webCLBuildOptionsVector.size() && !webCLBuildOptionsVector[j].StartsWith("-"); ++j) {}
				if (webCLBuildOptionsVector[i].StripWhiteSpace() == buildOptionDashD && j == i + 1) {
					ec.ThrowDOMException(WebCLException::INVALID_BUILD_OPTIONS, "WebCLException::INVALID_BUILD_OPTIONS");
					return;
				}

				i = --j;
				continue;
			}

			ec.ThrowDOMException(WebCLException::INVALID_BUILD_OPTIONS, "WebCLException::INVALID_BUILD_OPTIONS");
			return;
		}
	}

	if (callback) {
		if (mBuildCallback) {
			ec.ThrowDOMException(WebCLException::INVALID_OPERATION, "WebCLException::INVALID_OPERATION");
			return;
		}
	}

	cl_int err = CL_SUCCESS;
	Vector<cl_point> clDevices;
	Vector<cl_point> contextDevices;

	contextDevices = mClContext->getClDevices(ec);

	if (devices.size()) {
		Vector<cl_point> inputDevices;
		for (auto device : devices) {
			inputDevices.push_back((cl_point)device->getCLDevice());
		}

		size_t contextDevicesLength = contextDevices.size();
		for (size_t z, i = 0; i < inputDevices.size(); i++) {
			// Check if the inputDevices[i] is part of programs WebCLContext.
			for (z = 0; z < contextDevicesLength; z++) {
				if (contextDevices[z] == inputDevices[i]) {
					break;
				}
			}

			if (z == contextDevicesLength) {
				ec.ThrowDOMException(WebCLException::INVALID_DEVICE, "WebCLException::INVALID_DEVICE");
				return;
			}
			clDevices.push_back(inputDevices[i]);
		}
	} else {
		for (auto contextDevice : contextDevices)
			clDevices.push_back(contextDevice);
	}

	if (!clDevices.size()) {
		ec.ThrowDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
		return;
	}

	mIsProgramBuilt = true;
	if (callback != nullptr) {
		mBuildCallback = callback;
		err = webcl_clBuildProgram(webcl_channel_, mClProgram, clDevices.size(), (cl_device_id*)clDevices.data(), buildOptions.Utf8().data(), (cl_point)mClProgram, mHandlerKey, OPENCL_OBJECT_TYPE::CL_PROGRAM);
	}
	else {
		err = webcl_clBuildProgram(webcl_channel_, mClProgram, clDevices.size(), (cl_device_id*)clDevices.data(), buildOptions.Utf8().data(), (cl_point)mClProgram, 0, OPENCL_OBJECT_TYPE::CL_PROGRAM);
	}

	if (err != CL_SUCCESS)
		ec.ThrowDOMException(WebCLException::BUILD_PROGRAM_FAILURE, "WebCLException::BUILD_PROGRAM_FAILURE");

	contextDevices.clear();
}

void WebCLProgram::build(const String& options, WebCLCallback* callback, ExceptionState& es)
{
	HeapVector<Member<WebCLDevice>> devices;
	build(devices, options, callback, es);
}

WebCLKernel* WebCLProgram::createKernel(	const String& kernel_name, ExceptionState& ec)
{
	cl_int err = 0;
	cl_kernel cl_kernel_id = NULL;
	if (mClProgram == NULL) {
		printf("Error: Invalid program object\n");
		ec.ThrowDOMException(WebCLException::INVALID_PROGRAM, "WebCLException::INVALID_PROGRAM");
		return NULL;
	}
	if (!mIsProgramBuilt) {
		ec.ThrowDOMException(WebCLException::INVALID_PROGRAM_EXECUTABLE, "WebCLException::INVALID_PROGRAM_EXECUTABLE");
		return NULL;
	}
	const char* kernel_name_str = strdup(kernel_name.Utf8().data());
	cl_kernel_id = webcl_clCreateKernel(webcl_channel_, mClProgram, kernel_name_str, &err);
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	} else {
		WebCLKernel* o = WebCLKernel::create(mContext, cl_kernel_id, this, kernel_name);
		if (o != NULL) {
			o->setDevice(mDeviceId);
			o->setCLContext(mClContext);
			mClContext->addCLKernel((cl_obj_key)cl_kernel_id, o);
		}
		return o;
	}
	return NULL;
}

HeapVector<Member<WebCLKernel>> WebCLProgram::createKernelsInProgram(ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLProgram::createKernelsInProgram";

	cl_int err = 0;
	cl_kernel* kernelBuf = NULL;
	cl_uint num = 0;
	HeapVector<Member<WebCLKernel>> o;
	if (mClProgram == NULL) {
		printf("Error: Invalid program object\n");
		ec.ThrowDOMException(WebCLException::INVALID_PROGRAM, "WebCLException::INVALID_PROGRAM");
		return o;
	}
	if (!mIsProgramBuilt) {
		ec.ThrowDOMException(WebCLException::INVALID_PROGRAM_EXECUTABLE, "WebCLException::INVALID_PROGRAM_EXECUTABLE");
		return o;
	}
	err = webcl_clCreateKernelsInProgram (webcl_channel_, mClProgram, 0, nullptr, &num);
	if (err != CL_SUCCESS) {
		printf("Error: clCreateKernelsInProgram \n");
		ec.ThrowDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");
		return o;
	}
	if(num == 0) {
		printf("Warning: createKernelsInProgram - Number of Kernels is 0 \n");
		ec.ThrowDOMException(WebCLException::FAILURE, "WebCLException::FAILURE");
		return o;
	}
	kernelBuf = new cl_kernel[num];
	if(!kernelBuf)
		return o;

	err = webcl_clCreateKernelsInProgram (webcl_channel_, mClProgram, num, kernelBuf, NULL);
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	} else {
		CLLOG(INFO) << "WebCLKernelList Size=" << num;

		for(unsigned int i = 0; i < num; i++) {
			CLLOG(INFO) << "kerner[" << i << "] " << kernelBuf[i];

			WebCLKernel* kernelObject = WebCLKernel::create(mContext, kernelBuf[i], this, "");
			if (kernelObject != NULL) {
				kernelObject->setDevice(mDeviceId);
				kernelObject->setCLContext(mClContext);
				mClContext->addCLKernel((cl_obj_key)kernelBuf[i], kernelObject);
				String kernelName = kernelObject->getKernelName(ec);
				kernelObject->setKernelName(kernelName);
				o.push_back(kernelObject);
			}
		}
	}

	delete[] kernelBuf;

	return o;
}

void WebCLProgram::setDevice(WebCLDevice* m_device_id_)
{
	mDeviceId = m_device_id_;
}

cl_program WebCLProgram::getCLProgram()
{
	return mClProgram;
}

void WebCLProgram::release(ExceptionState& ec)
{
	cl_int err = 0;

	if (mClProgram == NULL) {
		printf("Error: Invalid program object\n");
		ec.ThrowDOMException(WebCLException::INVALID_DEVICE, "WebCLException::INVALID_DEVICE");
		return;
	}
	err = webcl_clReleaseProgram(webcl_channel_, mClProgram);
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	} else {
		if (mClContext != NULL) {
			mClContext->removeCLProgram((cl_obj_key)mClProgram);
		}
		mClProgram = NULL;
		mBuildCallback = nullptr;
	}
	return;
}

void WebCLProgram::triggerCallback() {
	if (mClProgram != NULL && mBuildCallback != nullptr) {
		mBuildCallback->handleEvent();
		mBuildCallback.Clear();
	}
}

DEFINE_TRACE(WebCLProgram) {
	visitor->Trace(mContext);
	visitor->Trace(mDeviceId);
	visitor->Trace(mBuildCallback);
	visitor->Trace(mClContext);
	visitor->Trace(mDeviceList);
}


} // namespace blink
