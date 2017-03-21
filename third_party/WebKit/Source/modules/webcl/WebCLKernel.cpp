// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wtf/build_config.h"
#include "bindings/core/v8/V8Binding.h"
#include "bindings/modules/v8/V8WebCLKernel.h"

#include "WebCLKernel.h"
#include "WebCL.h"
#include "WebCLDevice.h"
#include "WebCLSampler.h"
#include "WebCLMemoryObject.h"
#include "modules/webcl/WebCLKernelArgInfo.h"
#include "WebCLContext.h"
#include "WebCLImage.h"
#include "WebCLBuffer.h"
#include "WebCLProgram.h"
#include "WebCLKernelArgInfoProvider.h"
#include "WebCLInputChecker.h"
#include "core/dom/custom/WebCL/WebCLException.h"

#include "bindings/modules/v8/V8WebCLBuffer.h"
#include "bindings/modules/v8/V8WebCLImage.h"
#include "bindings/modules/v8/V8WebCLSampler.h"
#include "bindings/core/v8/V8ArrayBufferView.h"

#include "core/dom/DOMTypedArray.h"
#include "core/dom/DOMArrayBufferView.h"

#include "wtf/text/WTFString.h"

namespace blink {

WebCLKernel::WebCLKernel(WebCL* compute_context, cl_kernel kernel, WebCLProgram* program, String kernelName)
							: mContext(compute_context), mClProgram(program), mClKernel(kernel)
{
	if (!kernelName.isEmpty()) {
		setKernelName(kernelName);
	}
}

WebCLKernel::~WebCLKernel()
{
}

ScriptValue WebCLKernel::getInfo (ScriptState* scriptState, CLenum kernelInfo, ExceptionState& ec)
{
	v8::Handle<v8::Object> creationContext = scriptState->context()->Global();
	v8::Isolate* isolate = scriptState->isolate();

	CLLOG(INFO) << "WebCLKernel::getInfo, kernel=" << mClKernel;

	cl_int err = 0;
	char function_name[1024];
	cl_uint uint_units = 0;
	 cl_program clProgramId = NULL;
	cl_context clContextId = NULL;
	WebCLContext* contextObj = nullptr;

	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	switch(kernelInfo)
	{
		case WebCL::KERNEL_FUNCTION_NAME:
			err = webcl_clGetKernelInfo(webcl_channel_, mClKernel, CL_KERNEL_FUNCTION_NAME, sizeof(function_name), &function_name, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8String(isolate, String(function_name)));
			break;
		case WebCL::KERNEL_NUM_ARGS:
			err = webcl_clGetKernelInfo(webcl_channel_, mClKernel, CL_KERNEL_NUM_ARGS , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
			break;
		case WebCL::KERNEL_REFERENCE_COUNT:
			err = webcl_clGetKernelInfo(webcl_channel_, mClKernel, CL_KERNEL_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(uint_units)));
			break;
		 case WebCL::KERNEL_PROGRAM: {
		 	err = webcl_clGetKernelInfo(webcl_channel_, mClKernel, CL_KERNEL_PROGRAM, sizeof(clProgramId), &clProgramId, NULL);
		 	Persistent<WebCLProgram> savedProgram = mClContext->findCLProgram((cl_obj_key)clProgramId);
		 	if(savedProgram.get()) {
		 		return ScriptValue(scriptState, toV8(savedProgram, creationContext, isolate));
		 	}
		 	else {
		 		return ScriptValue(scriptState, v8::Null(isolate));
		 	}
		 	 }
		 	break;
		case WebCL::KERNEL_CONTEXT:
			err = webcl_clGetKernelInfo(webcl_channel_, mClKernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &clContextId, NULL);
			if (err == CL_SUCCESS)
			{
				Persistent<WebCLContext> savedContext = mContext->findCLContext((cl_obj_key)clContextId);
				if(savedContext.get() == NULL) {
					CLLOG(INFO) << ">>can't find saved context!!";

					contextObj = WebCLContext::create(mContext.get(), clContextId);
					contextObj->initializeContextDevice(ec);
					return ScriptValue(scriptState, toV8(contextObj, creationContext, isolate));
				}
				else {
					CLLOG(INFO) << ">>find saved context!!";

					return ScriptValue(scriptState, toV8(savedContext, creationContext, isolate));
				}
			}
			break;
		default:
			ec.throwDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			return ScriptValue(scriptState, v8::Null(isolate));
	}
	WebCLException::throwException(err, ec);
	return ScriptValue(scriptState, v8::Null(isolate));
}

ScriptValue WebCLKernel::getWorkGroupInfo(ScriptState* scriptState, WebCLDevice* device, CLenum param_name, ExceptionState& ec)
{
	v8::Handle<v8::Object> creationContext = scriptState->context()->Global();
	v8::Isolate* isolate = scriptState->isolate();

	cl_int err = 0;
	cl_device_id clDevice = NULL;
	size_t sizetUnits = 0;
	size_t sizetArrayUnit[3] = {0};
	cl_ulong  ulongUnits = 0;

	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		CLLOG(INFO) << "mClKernel is NULL";
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return ScriptValue(scriptState, v8::Null(isolate));
	}

	if (device != NULL) {
		clDevice = device->getCLDevice();
		if (clDevice == NULL) {
			printf("Error: clDevice null\n");
			CLLOG(INFO) << "clDevice is NULL";
			ec.throwDOMException(WebCLException::INVALID_DEVICE, "WebCLException::INVALID_DEVICE");
			return ScriptValue(scriptState, v8::Null(isolate));
		}
	}
	switch (param_name) {

		case WebCL::KERNEL_WORK_GROUP_SIZE:
			err = webcl_clGetKernelWorkGroupInfo(webcl_channel_, mClKernel, clDevice, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &sizetUnits, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizetUnits)));
			break;
		case WebCL::KERNEL_COMPILE_WORK_GROUP_SIZE:
		{
			err = webcl_clGetKernelWorkGroupInfo(webcl_channel_, mClKernel, clDevice, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, 3*sizeof(size_t), &sizetArrayUnit, NULL);
			int values[3] = {0,0,0};

			Vector<unsigned> sizeUnsignedVector;

 			for(int i=0; i<3; i++)
 			{
 				printf("%d\n", values[i]);
 				sizeUnsignedVector.append(sizetArrayUnit[i]);
 			}
 			return ScriptValue(scriptState, toV8(sizeUnsignedVector, creationContext, isolate));
		}
		case WebCL::KERNEL_LOCAL_MEM_SIZE:
			err = webcl_clGetKernelWorkGroupInfo(webcl_channel_, mClKernel, clDevice, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ulongUnits, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(ulongUnits)));
			break;
		case WebCL::KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
		{
			err = webcl_clGetKernelWorkGroupInfo(webcl_channel_, mClKernel, clDevice, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &sizetUnits, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Integer::NewFromUnsigned(isolate, static_cast<unsigned>(sizetUnits)));
			break;
		}
		case WebCL::KERNEL_PRIVATE_MEM_SIZE:
		{
			err = webcl_clGetKernelWorkGroupInfo(webcl_channel_, mClKernel, clDevice, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(cl_ulong), &ulongUnits, NULL);
			if (err == CL_SUCCESS)
				return ScriptValue(scriptState, v8::Uint32::NewFromUnsigned(isolate, static_cast<unsigned long>(ulongUnits)));
			break;
		}
		default:
			ec.throwDOMException(WebCLException::INVALID_VALUE, "WebCLException::INVALID_VALUE");
			printf("Error: Unsupported Kernrl Info type\n");
			return ScriptValue(scriptState, v8::Null(isolate));
	}

	printf("Error: clGetKerelWorkGroupInfo\n");
	WebCLException::throwException(err, ec);
	return ScriptValue(scriptState, v8::Null(isolate));
}

WebCLKernelArgInfo WebCLKernel::getArgInfo(CLuint index, ExceptionState& ec)
{
	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return WebCLKernelArgInfo();
	}

	CLLOG(INFO) << "getArgInfo, numberOfArguments = " << mArgInfoProvider->numberOfArguments() << ", index = " << index;
	if (index >= mArgInfoProvider->numberOfArguments()) {
		ec.throwDOMException(WebCLException::INVALID_ARG_INDEX, "WebCLException::INVALID_ARG_INDEX");
		return WebCLKernelArgInfo();
	}

	return mArgInfoProvider->argumentsInfo()->at(index);
}

void WebCLKernel::setArg(CLuint index, const ScriptValue& value, ExceptionState& ec)
{
	v8::Isolate* isolate = value.isolate();
	v8::Handle<v8::Value> object(value.v8Value());

	if (V8WebCLBuffer::hasInstance(object, isolate)) {
		WebCLBuffer* buffer;
		buffer = V8WebCLBuffer::toImplWithTypeCheck(isolate, object);
		setArg(index, buffer, ec);
		return;
	} else if (V8WebCLImage::hasInstance(object, isolate)) {
		WebCLImage* image;
		image = V8WebCLImage::toImplWithTypeCheck(isolate, object);
		setArg(index, image, ec);
		return;
	} else if (V8WebCLSampler::hasInstance(object, isolate)) {
		WebCLSampler* sampler;
		sampler = V8WebCLSampler::toImplWithTypeCheck(isolate, object);
		setArg(index, sampler, ec);
		return;
	} else if (/*V8ArrayBufferView::hasInstance(object, isolate)*/object->IsArrayBufferView()) {
		DOMArrayBufferView* arrayBufferView;
		arrayBufferView = object->IsArrayBufferView() ? V8ArrayBufferView::toImpl(v8::Handle<v8::ArrayBufferView>::Cast(object)) : 0;
		setArg(index, arrayBufferView, ec);
		return;
	}
	unsigned unsignedValue = toUInt32(isolate, object, NormalConversion, ec);
	if(!ec.hadException()) {
		setArg(index, &unsignedValue, ec);
		return;
	}

	ec.throwDOMException(WebCLException::INVALID_ARG_VALUE, "WebCLException::INVALID_ARG_VALUE");
}

void WebCLKernel::setArg(CLuint index, WebCLSampler* value, ExceptionState& ec)
{
	cl_int err = 0;
	CLLOG(INFO) << "setArg(WebCLSampler)";

	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return;
	}

	if (index > mArgInfoProvider->numberOfArguments()) {
		ec.throwDOMException(WebCLException::INVALID_ARG_INDEX, "WebCLException::INVALID_ARG_INDEX");
		return;
	}

	cl_sampler clSamplerId = nullptr;
	if (value) {
		clSamplerId = value->getSampler();
		if (!clSamplerId) {
			ec.throwDOMException(WebCLException::INVALID_SAMPLER, "WebCLException::INVALID_SAMPLER");
			return;
		}
	}

	WebCLKernelArgInfo argInfo = getArgInfo(index, ec);
	if (argInfo.typeName() != "sampler_t") {
		ec.throwDOMException(WebCLException::INVALID_ARG_VALUE, "WebCLException::INVALID_ARG_VALUE");
		return;
	}

	mContext->startHandling();

	WebCL_Operation_setArg operation = WebCL_Operation_setArg();
	operation.kernel = (cl_point)mClKernel;
	operation.ptr_arg = (cl_point)clSamplerId;
	operation.arg_index = index;
	operation.arg_size = sizeof(cl_sampler);
	operation.data_type = SETARG_DATA_TYPE::SETARG_CL_SAMPLER;

	mContext->setOperationParameter(&operation);
	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::SET_ARG);

	WebCL_Result_setArg result = WebCL_Result_setArg();
	mContext->getOperationResult(&result);
	err = result.result;
	if (err == CL_SUCCESS){
		mAssociatedArgsMarking[index] = 1;
		return;
	}
	CLLOG(INFO) << "WebCLKernel::setArg error : " << err;
	WebCLException::throwException(err, ec);
}

void WebCLKernel::setArg(unsigned index, WebCLBuffer* buffer, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLKernel::setArg with WebCLBuffer";

	cl_mem clMemoryId = nullptr;
	if (buffer) {
		clMemoryId = buffer->getCLMem();
		if (!clMemoryId) {
			ec.throwDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
			return;
		}
	}

	WebCLKernelArgInfo argInfo = getArgInfo(index, ec);
	((WebCLMemoryObject*)buffer)->getMemObjType(ec);

	if (ec.hadException()) {
		ec.throwDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
		return;
	}

	CLLOG(INFO) << "kernel arg info, type name=" << argInfo.typeName().latin1().data() << ", name=" << argInfo.name().latin1().data() << ", isBuffer=" << argInfo.isBuffer();

	if(!argInfo.hasName() || !argInfo.isBuffer() || argInfo.addressQualifier().isEmpty()) {
		ec.throwDOMException(WebCLException::INVALID_ARG_VALUE, "WebCLException::INVALID_ARG_VALUE");
		return;
	}

	setArg(index, buffer->getCLMem(), ec);
}

void WebCLKernel::setArg(unsigned index, WebCLImage* image, ExceptionState& ec)
{
	CLLOG(INFO) << "WebCLKernel::setArg with WebCLImage";

	cl_mem clMemoryId = nullptr;
	if (image) {
		clMemoryId = image->getCLMem();
		if (!clMemoryId) {
			ec.throwDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
			return;
		}
	}

	WebCLKernelArgInfo argInfo = getArgInfo(index, ec);
	((WebCLMemoryObject*)image)->getMemObjType(ec);

	if (ec.hadException()) {
		ec.throwDOMException(WebCLException::INVALID_MEM_OBJECT, "WebCLException::INVALID_MEM_OBJECT");
		return;
	}

	if(!argInfo.hasName() || (argInfo.typeName() != "image2d_t" && argInfo.typeName() != "image3d_t")) {
		ec.throwDOMException(WebCLException::INVALID_ARG_VALUE, "WebCLException::INVALID_ARG_VALUE");
		return;
	}

	setArg(index, image->getCLMem(), ec);
}

void WebCLKernel::setArg(CLuint index, cl_mem value, ExceptionState& ec)
{
	CLLOG(INFO) << "setArg cl_mem";
	cl_int err = 0;

	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return;
	}

	if (index > mArgInfoProvider->numberOfArguments()) {
		ec.throwDOMException(WebCLException::INVALID_ARG_INDEX, "WebCLException::INVALID_ARG_INDEX");
		return;
	}

	CLLOG(INFO) << "webcl_clSetKernelArg";

	mContext->startHandling();

	WebCL_Operation_setArg operation = WebCL_Operation_setArg();
	operation.kernel = (cl_point)mClKernel;
	operation.ptr_arg = (cl_point)value;
	operation.arg_index = index;
	operation.arg_size = sizeof(cl_mem);
	operation.data_type = SETARG_DATA_TYPE::SETARG_CL_MEM;

	mContext->setOperationParameter(&operation);
	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::SET_ARG);

	WebCL_Result_setArg result = WebCL_Result_setArg();
	mContext->getOperationResult(&result);
	err = result.result;

	if (err == CL_SUCCESS){
		mAssociatedArgsMarking[index] = 1;
		return;
	}
	CLLOG(INFO) << "WebCLKernel::setArg error : " << err;
	WebCLException::throwException(err, ec);
}

void WebCLKernel::setArg(CLuint index, DOMArrayBufferView* value, ExceptionState& ec)
{
	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return;
	}
	CLLOG(INFO) << "setArg(DomArrayBufferView)";
	cl_int err = 0;

	if (index > mArgInfoProvider->numberOfArguments()) {
		ec.throwDOMException(WebCLException::INVALID_ARG_INDEX, "WebCLException::INVALID_ARG_INDEX");
		return;
	}

	err = CL_SUCCESS;
	WebCLKernelArgInfo argInfo = getArgInfo(index, ec);
	const String& accessQualifier = argInfo.addressQualifier();
	bool hasLocalQualifier = accessQualifier == "local";
	if (hasLocalQualifier) {
		if (value->type() != DOMArrayBufferView::TypeUint32) {
			ec.throwDOMException(WebCLException::INVALID_ARG_VALUE, "WebCLException::INVALID_ARG_VALUE");
			return;
		}

		Uint32Array* typedArray = static_cast<Uint32Array*>(value->view());
		if (typedArray->length() != 1) {
			ec.throwDOMException(WebCLException::INVALID_ARG_VALUE, "WebCLException::INVALID_ARG_VALUE");
			return;
		}

		unsigned* values = static_cast<Uint32Array*>(value->view())->data();

		mContext->startHandling();

		WebCL_Operation_setArg operation = WebCL_Operation_setArg();
		operation.kernel = (cl_point)mClKernel;
		operation.arg_index = index;
		operation.arg_size = static_cast<size_t>(values[0]);
		operation.data_type = SETARG_DATA_TYPE::SETARG_ARRAY_BUFFER;
		operation.is_local = true;

		mContext->setOperationParameter(&operation);
		mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::SET_ARG);

		WebCL_Result_setArg result = WebCL_Result_setArg();
		mContext->getOperationResult(&result);
		err = result.result;

		if (err == CL_SUCCESS) {
			mAssociatedArgsMarking[index] = 1;
			return;
		}

		CLLOG(INFO) << "WebCLKernel::setArg error : " << err;
		WebCLException::throwException(err, ec);
		return;
	}

	Vector<unsigned> uLongBuffer;
	WTF::String type = argInfo.typeName();
	bool err_status = false;
	switch(value->type()) {
	case (DOMArrayBufferView::TypeFloat64): // DOUBLE
		if (!type.startsWith("double")) {
			err_status = true;
		}
		break;
	case (DOMArrayBufferView::TypeFloat32): // FLOAT
		if (!type.startsWith("float")) {
			err_status = true;
		}
		break;
	case (DOMArrayBufferView::TypeUint32): {// UINT
		if (!(type.startsWith("uint") || type.startsWith("ulong"))) {
			err_status = true;
		}
		if (type.startsWith("long")) {
		#if CPU(BIG_ENDIAN)
			uLongBuffer.resize(hostDataSize/2);
			swapElementsForBigEndian(hostDataSize / 2, bufferView, uLongBuffer);
			bufferData = uLongBuffer.data();
		#endif
		}
		break;
	}
	case (DOMArrayBufferView::TypeInt32):  // INT
		if (!(type.startsWith("int") || type.startsWith("long"))) {
			err_status = true;
		}
		break;
	case (DOMArrayBufferView::TypeUint16): // USHORT
		if (!type.startsWith("ushort")) {
			err_status = true;
		}
		break;
	case (DOMArrayBufferView::TypeInt16): // SHORT
		if (!type.startsWith("short")) {
			err_status = true;
		}
		break;
	case (DOMArrayBufferView::TypeUint8): // UCHAR
		if (!type.startsWith("uchar")) {
			err_status = true;
		}
		break;
	case (DOMArrayBufferView::TypeInt8): // CHAR
		if (!type.startsWith("char")) {
			err_status = true;
		}
		break;
	default:
		err_status = true;
	}
	if (err_status) {
		ec.throwDOMException(WebCLException::INVALID_ARG_VALUE, "WebCLException::INVALID_ARG_VALUE");
		return;
	}

	mContext->startHandling();

	WebCL_Operation_setArg operation = WebCL_Operation_setArg();
	operation.kernel = (cl_point)mClKernel;
	operation.arg_index = index;
	operation.arg_size = value->byteLength();
	CLLOG(INFO) << "arg_size : " << value->byteLength();
	operation.data_type = SETARG_DATA_TYPE::SETARG_ARRAY_BUFFER;

	mContext->setOperationParameter(&operation);
	mContext->setOperationData(value->baseAddress(), value->byteLength());

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::SET_ARG);

	WebCL_Result_setArg result = WebCL_Result_setArg();
	mContext->getOperationResult(&result);
	err = result.result;

	if (err == CL_SUCCESS){
		mAssociatedArgsMarking[index] = 1;
		return;
	}

	CLLOG(INFO) << "WebCLKernel::setArg error : " << err;
	WebCLException::throwException(err, ec);
}

void WebCLKernel::setArg(unsigned index, unsigned* argValue, ExceptionState& ec)
{
	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return;
	}

	cl_int err = 0;

	if (index > mArgInfoProvider->numberOfArguments()) {
		ec.throwDOMException(WebCLException::INVALID_ARG_INDEX, "WebCLException::INVALID_ARG_INDEX");
		return;
	}

	mContext->startHandling();

	WebCL_Operation_setArg operation = WebCL_Operation_setArg();
	operation.kernel = (cl_point)mClKernel;
	operation.arg_index = index;
	operation.arg_size = sizeof(unsigned);
	operation.data_type = SETARG_DATA_TYPE::SETARG_UNSIGNED;

	mContext->setOperationParameter(&operation);
	mContext->setOperationData(argValue, sizeof(unsigned));

	mContext->sendOperationSignal(OPENCL_OPERATION_TYPE::SET_ARG);

	WebCL_Result_setArg result = WebCL_Result_setArg();
	mContext->getOperationResult(&result);
	err = result.result;

	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
		return;
	}

	mAssociatedArgsMarking[index] = 1;
}

void WebCLKernel::release(ExceptionState& ec)
{
	cl_int err = 0;
	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return;
	}
	CLLOG(INFO) << "WebCLKernel::release : " << mClKernel;
	err = webcl_clReleaseKernel(webcl_channel_, mClKernel);
	if (err != CL_SUCCESS) {
		WebCLException::throwException(err, ec);
	} else {
		if (mClContext != NULL) {
			mClContext->removeCLKernel((cl_obj_key)mClKernel);
		}
		mClKernel = NULL;
		mArgInfoProvider->clear();
		mArgInfoProvider = nullptr;
	}
	return;
}

void WebCLKernel::setDevice(WebCLDevice* m_device_id_)
{
	mDeviceId = m_device_id_;
}

void WebCLKernel::setCLContext(WebCLContext* context)
{
	mClContext = context;
}

cl_kernel WebCLKernel::getCLKernel()
{
	return mClKernel;
}

WebCLProgram* WebCLKernel::getProgram()
{
	return mClProgram;
}

String& WebCLKernel::getKernelName() {
	return mKernelName;
}

String WebCLKernel::getKernelName(ExceptionState& ec) {
	cl_int err = 0;
	char function_name[1024];

	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return String("");
	}

	err = webcl_clGetKernelInfo(webcl_channel_, mClKernel, CL_KERNEL_FUNCTION_NAME, sizeof(function_name), &function_name, NULL);
	if (err == CL_SUCCESS) {
		return String(function_name);
	}
	return String("");
}

size_t WebCLKernel::associatedArgsNum() {
	size_t associatedArgNum = 0;

	size_t argNum = mArgInfoProvider->numberOfArguments();
	for(size_t i=0; i<argNum; i++) {
		if(mAssociatedArgsMarking[i])
			++associatedArgNum;
	}

	return associatedArgNum;
}

void WebCLKernel::setKernelName(String kernelName) {
	mKernelName = kernelName;
	mArgInfoProvider = new WebCLKernelArgInfoProvider(this);
	mAssociatedArgsMarking.resize(mArgInfoProvider->numberOfArguments());
	for(size_t i=0; i<mArgInfoProvider->numberOfArguments(); i++)
		mAssociatedArgsMarking[i] = 0;
};

Vector<unsigned> WebCLKernel::getCompileWorkGroupSizeVector(WebCLDevice* device, ExceptionState& ec) {
	cl_int err = 0;
	size_t sizetArrayUnit[3] = {0};
	cl_device_id clDevice = NULL;

	Vector<unsigned> sizeUnsignedVector;

	if (mClKernel == NULL) {
		printf("Error: Invalid kernel\n");
		CLLOG(INFO) << "mClKernel is NULL";
		ec.throwDOMException(WebCLException::INVALID_KERNEL, "WebCLException::INVALID_KERNEL");
		return sizeUnsignedVector;
	}

	if (device != NULL) {
		clDevice = device->getCLDevice();
		if (clDevice == NULL) {
			printf("Error: clDevice null\n");
			CLLOG(INFO) << "clDevice is NULL";
			ec.throwDOMException(WebCLException::INVALID_DEVICE, "WebCLException::INVALID_DEVICE");
			return sizeUnsignedVector;
		}
	}
	err = webcl_clGetKernelWorkGroupInfo(webcl_channel_, mClKernel, clDevice, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, 3*sizeof(size_t), &sizetArrayUnit, NULL);

	if (err == CL_SUCCESS) {
		for(int i=0; i<3; i++)
		{
			sizeUnsignedVector.append(sizetArrayUnit[i]);
		}
	}
	else {
		WebCLException::throwException(err, ec);
	}

	return sizeUnsignedVector;
}

size_t WebCLKernel::getKernelNumArgs(ExceptionState& ec) {
	return (size_t)mArgInfoProvider->numberOfArguments();
}

DEFINE_TRACE(WebCLKernel) {
	visitor->trace(mContext);
	visitor->trace(mDeviceId);
	visitor->trace(mClContext);
	visitor->trace(mClProgram);
	visitor->trace(mArgInfoProvider);
}

} // namespace blink
