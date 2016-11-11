/*
 * opencl_macro.h
 *
 *  Created on: 2015. 6. 5.
 *      Author: jphofasb
 */
#ifndef UI_OPENCL_OPENCL_INCLUDE_H_
#define UI_OPENCL_OPENCL_INCLUDE_H_

#include "third_party/opencl/cl.h"
// gl/cl sharing
#include "third_party/opencl/cl_gl.h"

#include "base/logging.h"
#include <vector>

// gl/cl sharing
//#include "third_party/khronos/GLES2/gl2.h"
#include <third_party/khronos/KHR/khrplatform.h>
typedef khronos_int8_t GLbyte;
typedef khronos_float_t GLclampf;
typedef khronos_int32_t GLfixed;
typedef short GLshort;
typedef unsigned short GLushort;
typedef void GLvoid;
typedef struct __GLsync *GLsync;
typedef khronos_int64_t GLint64;
typedef khronos_uint64_t GLuint64;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef char GLchar;
typedef khronos_float_t GLfloat;
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;
typedef unsigned int GLbitfield;
typedef int GLint;
typedef unsigned char GLboolean;
typedef int GLsizei;
typedef khronos_uint8_t GLubyte;

#define CL_CALLBACK

#define CL_SEND_IPC_MESSAGE_FAILURE 	-9999

// #define ENABLE_CLLOG

#ifdef ENABLE_CLLOG
#define CLLOG(severity) \
	DLOG(severity) << "CL::"
#else
#define CLLOG(severity) \
	LAZY_STREAM(LOG_STREAM(severity), false)
#endif

#define DELETE_ARRAY(name)	\
	if(name)				\
		delete[] name;

#define DELETE_VECTOR_OBJ(type, name)	\
	if(name.size())						\
		name.clear();					\
	std::vector<type>().swap(name);

#define DELETE_VECTOR_PTR(type, name)	\
	if(name->size())					\
		name->clear();					\
	std::vector<type>().swap(*name);

/*
 * Some data type definition for convenience
 */
//from AMD code, for  void pointer type on IPC message
#if defined(OS_ANDROID)
#define cl_point uint32_t
#elif defined(OS_LINUX)
#define cl_point uintptr_t
#endif

//from AMD code...
typedef intptr_t cl_device_partition_property;

enum OPENCL_OBJECT_TYPE {
	CL_WEBCL,
	CL_PLATFORM,
	CL_DEVICE,
	CL_CONTEXT,
	CL_MEMORY_OBJECT,
	CL_IMAGE,
	CL_BUFFER,
	CL_PROGRAM,
	CL_COMMAND_QUEUE,
	CL_KERNEL,
	CL_EVENT,
	CL_USER_EVENT
};

enum OPENCL_OPERATION_TYPE {
	SET_ARG,
	CREATE_BUFFER,
	CREATE_IMAGE,
	ENQUEUE_BASE,
	ENQUEUE_COPY_BUFFER,
	ENQUEUE_COPY_BUFFER_RECT,
	ENQUEUE_COPY_IMAGE,
	ENQUEUE_COPY_IMAGE_TO_BUFFER,
	ENQUEUE_COPY_BUFFER_TO_IMAGE,
	ENQUEUE_READ_BUFFER,
	ENQUEUE_READ_BUFFER_RECT,
	ENQUEUE_READ_IMAGE,
	ENQUEUE_WRITE_BUFFER,
	ENQUEUE_WRITE_BUFFER_RECT,
	ENQUEUE_WRITE_IMAGE,
	ENQUEUE_NDRANGE_KERNEL,
	// gl/cl sharing
	CREATE_BUFFER_FROM_GL_BUFFER,
	CREATE_IMAGE_FROM_GL_RENDER_BUFFER,
	CREATE_IMAGE_FROM_GL_TEXTURE,
	GET_GL_OBJECT_INFO,
	ENQUEUE_ACQUIRE_GLOBJECTS,
	ENQUEUE_RELEASE_GLOBJECTS,
	FINISH,
};

enum HOST_PTR_DATA_TYPE {
	EMPTY_DATA,
	SIGNED_CHAR,
	UNSIGNED_CHAR,
	SHORT,
	UNSIGNED_SHORT,
	INT,
	UNSIGNED,
	FLOAT,
	DOUBLE,
	CL_MEM
};

enum SETARG_DATA_TYPE {
	SETARG_CL_SAMPLER,
	SETARG_CL_MEM,
	SETARG_UNSIGNED,
	SETARG_ARRAY_BUFFER,
};

// gl/cl sharing
enum GLResourceType {
	BUFFER,
	RENDERBUFFER,
	TEXTURE,
};

struct BaseOperationData {
	cl_point ptr_1;
	cl_point ptr_2;
	cl_point ptr_3;
	unsigned uint_01;
	unsigned uint_02;
	unsigned uint_03;
	unsigned uint_04;
	unsigned uint_05;
	unsigned uint_06;
	unsigned long ulong_01;
	unsigned long ulong_02;
	int int_01;
	int int_02;
	int int_03;
	int int_04;
	int int_05;
	bool bool_01;
	bool bool_02;
	bool bool_03;
	size_t uint_arr_01[3];
	size_t uint_arr_02[3];
	size_t uint_arr_03[3];
};

struct WebCL_Operation_Base {
	int operation_type;
};

struct WebCL_Operation_createBuffer : public WebCL_Operation_Base {
	cl_point context;
	unsigned long flags;
	size_t size;
	bool use_host_ptr;
	WebCL_Operation_createBuffer() {
		operation_type = OPENCL_OPERATION_TYPE::CREATE_BUFFER;
		use_host_ptr = false;
	}
};

// gl/cl sharing
struct WebCL_Operation_createBufferFromGLBuffer : public WebCL_Operation_Base {
	cl_point context;
	unsigned long flags;
	GLuint bufobj;
	WebCL_Operation_createBufferFromGLBuffer() {
		operation_type = OPENCL_OPERATION_TYPE::CREATE_BUFFER_FROM_GL_BUFFER;
	}
};

struct WebCL_Operation_createImageFromGLRenderbuffer : public WebCL_Operation_Base {
	cl_point context;
	unsigned long flags;
	GLuint bufobj;
	WebCL_Operation_createImageFromGLRenderbuffer() {
		operation_type = OPENCL_OPERATION_TYPE::CREATE_IMAGE_FROM_GL_RENDER_BUFFER;
	}
};

struct WebCL_Operation_createImageFromGLTexture : public WebCL_Operation_Base {
	cl_point context;
	unsigned long flags;
	unsigned long textureTarget;
	GLint miplevel;
	GLuint textureobj;
	WebCL_Operation_createImageFromGLTexture() {
		operation_type = OPENCL_OPERATION_TYPE::CREATE_IMAGE_FROM_GL_TEXTURE;
	}
};

struct WebCL_Operation_getGLObjectInfo : public WebCL_Operation_Base {
	cl_point memobj;
	bool needObjectType;
	bool needObjectName;
	WebCL_Operation_getGLObjectInfo() {
		operation_type = OPENCL_OPERATION_TYPE::GET_GL_OBJECT_INFO;
	}
};

struct WebCL_Operation_enqueueCommonGLObjects : public WebCL_Operation_Base {
	cl_point commandQueue;
	cl_uint numObjects;
	cl_uint numEventInWaitList;
	bool need_event;
	WebCL_Operation_enqueueCommonGLObjects() {
		numObjects = 0;
		numEventInWaitList = 0;
		need_event = false;
	}
};

struct WebCL_Operation_createImage : public WebCL_Operation_Base {
	cl_point context;
	unsigned long flags;
	unsigned channel_order;
	unsigned channel_type;
	size_t image_width;
	size_t image_height;
	size_t image_row_pitch;
	bool use_host_ptr;
	WebCL_Operation_createImage() {
		operation_type = OPENCL_OPERATION_TYPE::CREATE_IMAGE;
		use_host_ptr = false;
	}
};

struct WebCL_Operation_setArg : public WebCL_Operation_Base {
	cl_point kernel;
	cl_point ptr_arg;
	size_t arg_index;
	size_t arg_size;
	int data_type;
	bool is_local;
	WebCL_Operation_setArg() {
		operation_type = OPENCL_OPERATION_TYPE::SET_ARG;
		is_local = false;
	}
};

struct WebCL_Operation_enqueueBase : public WebCL_Operation_Base {
	cl_point command_queue;
	cl_bool blocking;
	size_t event_length;
	bool need_event;
	int data_type;
	size_t data_size;
	WebCL_Operation_enqueueBase() {
		blocking = false;
		need_event = false;
		event_length = 0;
	}
};

struct WebCL_Operation_enqueueCopyBuffer : public WebCL_Operation_enqueueBase  {
	cl_point src_buffer;
	cl_point dst_buffer;
	size_t src_offset;
	size_t dst_offset;
	size_t num_bytes;
	WebCL_Operation_enqueueCopyBuffer() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER;
	}
};

struct WebCL_Operation_enqueueCopyBufferRect : public WebCL_Operation_enqueueBase  {
	cl_point src_buffer;
	cl_point dst_buffer;
	size_t src_origin[3];
	size_t dst_origin[3];
	size_t region[3];
	size_t src_row_pitch;
	size_t src_slice_pitch;
	size_t dst_row_pitch;
	size_t dst_slice_pitch;
	WebCL_Operation_enqueueCopyBufferRect() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER_RECT;
	}
};

struct WebCL_Operation_enqueueCopyImage : public WebCL_Operation_enqueueBase  {
	cl_point src_image;
	cl_point dst_image;
	size_t src_origin[3];
	size_t dst_origin[3];
	size_t region[3];
	WebCL_Operation_enqueueCopyImage() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_COPY_IMAGE;
	}
};

struct WebCL_Operation_enqueueCopyBufferToImage : public WebCL_Operation_enqueueBase  {
	cl_point src_buffer;
	cl_point dst_image;
	size_t src_offset;
	size_t dst_origin[3];
	size_t region[3];
	WebCL_Operation_enqueueCopyBufferToImage() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_COPY_BUFFER_TO_IMAGE;
	}
};

struct WebCL_Operation_enqueueCopyImageToBuffer : public WebCL_Operation_enqueueBase  {
	cl_point src_image;
	cl_point dst_buffer;
	size_t src_origin[3];
	size_t region[3];
	size_t dst_offset;
	WebCL_Operation_enqueueCopyImageToBuffer() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_COPY_IMAGE_TO_BUFFER;
	}
};


struct WebCL_Operation_enqueueReadBuffer : public WebCL_Operation_enqueueBase  {
	cl_point buffer;
	size_t buffer_offset;
	size_t num_bytes;
	WebCL_Operation_enqueueReadBuffer() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER;
	}
};

struct WebCL_Operation_enqueueReadBufferRect : public WebCL_Operation_enqueueBase  {
	cl_point buffer;
	size_t buffer_origin[3];
	size_t host_origin[3];
	size_t region[3];
	size_t buffer_row_pitch;
	size_t buffer_slice_pitch;
	size_t host_row_pitch;
	size_t host_slice_pitch;
	WebCL_Operation_enqueueReadBufferRect() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_READ_BUFFER_RECT;
	}
};

struct WebCL_Operation_enqueueReadImage : public WebCL_Operation_enqueueBase  {
	cl_point image;
	size_t origin[3];
	size_t region[3];
	size_t row_pitch;
	size_t slice_pitch;
	WebCL_Operation_enqueueReadImage() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_READ_IMAGE;
	}
};

struct WebCL_Operation_enqueueWriteBuffer : public WebCL_Operation_enqueueBase  {
	cl_point buffer;
	size_t buffer_offset;
	size_t num_bytes;
	WebCL_Operation_enqueueWriteBuffer() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER;
	}
};

struct WebCL_Operation_enqueueWriteBufferRect : public WebCL_Operation_enqueueBase  {
	cl_point buffer;
	size_t buffer_origin[3];
	size_t host_origin[3];
	size_t region[3];
	size_t buffer_row_pitch;
	size_t buffer_slice_pitch;
	size_t host_row_pitch;
	size_t host_slice_pitch;
	WebCL_Operation_enqueueWriteBufferRect() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_BUFFER_RECT;
	}
};

struct WebCL_Operation_enqueueWriteImage : public WebCL_Operation_enqueueBase  {
	cl_point image;
	size_t origin[3];
	size_t region[3];
	size_t input_row_pitch;
	size_t input_slice_pitch;
	WebCL_Operation_enqueueWriteImage() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_WRITE_IMAGE;
	}
};

struct WebCL_Operation_enqueueNDRangeKernel : public WebCL_Operation_enqueueBase  {
	cl_point kernel;
	size_t work_dim;
	size_t global_work_offset;
	size_t global_work_size;
	size_t local_work_size;
	WebCL_Operation_enqueueNDRangeKernel() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_NDRANGE_KERNEL;
	}
};

struct WebCL_Operation_finish : public WebCL_Operation_enqueueBase  {
	WebCL_Operation_finish() : WebCL_Operation_enqueueBase() {
		operation_type = OPENCL_OPERATION_TYPE::FINISH;
	}
};

struct BaseResultData {
	int int_01;
	cl_point ptr_1;
	cl_point ptr_2;
	unsigned uint_01;
	unsigned uint_02;
	unsigned uint_03;
	unsigned uint_04;
	unsigned uint_05;
	unsigned uint_06;
	bool bool_01;
	bool bool_02;
	bool bool_03;
};

struct WebCL_Result_Base {
	int operation_type;
	int result;
};

struct WebCL_Result_createBuffer : public WebCL_Result_Base {
	cl_point buffer;
	WebCL_Result_createBuffer() {
		operation_type = OPENCL_OPERATION_TYPE::CREATE_BUFFER;
	}
};

struct WebCL_Result_createImage : public WebCL_Result_Base {
	cl_point image;
	WebCL_Result_createImage() {
		operation_type = OPENCL_OPERATION_TYPE::CREATE_IMAGE;
	}
};

struct WebCL_Result_enqueueOperationBase : public WebCL_Result_Base {
	cl_point ret_event;
	WebCL_Result_enqueueOperationBase() {
		ret_event = 0;
		operation_type = OPENCL_OPERATION_TYPE::ENQUEUE_BASE;
	}
};

struct WebCL_Result_finish : public WebCL_Result_Base {
	cl_point ret_event;
	WebCL_Result_finish() {
		ret_event = 0;
		operation_type = OPENCL_OPERATION_TYPE::FINISH;
	}
};

struct WebCL_Result_setArg : public WebCL_Result_Base {
	WebCL_Result_setArg() {
		operation_type = OPENCL_OPERATION_TYPE::SET_ARG;
	}
};

//cl_gl_sharing
struct WebCL_Result_createFromGLBuffer : public WebCL_Result_Base {
	cl_point buffer;
	cl_uint serviceId;
	WebCL_Result_createFromGLBuffer() {
		operation_type = OPENCL_OPERATION_TYPE::CREATE_BUFFER_FROM_GL_BUFFER;
	}
};

struct WebCL_Result_getGLObjectInfo : public WebCL_Result_Base {
	cl_gl_object_type glObjectType;
	GLuint glObjectName;
	GLenum clGLTextureTarget;
	GLint clGLMipMapLevel;
	WebCL_Result_getGLObjectInfo() {
		operation_type = OPENCL_OPERATION_TYPE::GET_GL_OBJECT_INFO;
	}
};

#define ARG_PTR(type) \
	type *

#define ARG_CONST(type) \
	const type

//define callback pointer, has 1 arguments
#define CL_API_CALLBACK_ARGS1(name, arg1_type) \
	typedef void (*name)(arg1_type);

//define callback pointer, has 2 arguments
#define CL_API_CALLBACK_ARGS2(name, arg1_type, arg2_type) \
	typedef void (*name)(arg1_type, arg2_type);

//define callback pointer, has 3 arguments
#define CL_API_CALLBACK_ARGS3(name, arg1_type, arg2_type, arg3_type) \
	typedef void (*name)(arg1_type, arg2_type, arg3_type);

//define callback pointer, has 4 arguments
#define CL_API_CALLBACK_ARGS4(name, arg1_type, arg2_type, arg3_type, arg4_type) \
	typedef void (*name)(arg1_type, arg2_type, arg3_type, arg4_type);

//define callback pointer, has 5 arguments
#define CL_API_CALLBACK_ARGS5(name, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type) \
	typedef void (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type);

//define callback pointer for calling API
CL_API_CALLBACK_ARGS4(CL_CallbackclCreateContextFromType, ARG_CONST(ARG_PTR(char)), ARG_CONST(ARG_PTR(void)), size_t, ARG_PTR(void))
CL_API_CALLBACK_ARGS3(CL_CallbackclSetEvent, cl_event, cl_int, ARG_PTR(void));
CL_API_CALLBACK_ARGS2(CL_CallbackclBuildProgram, cl_program, ARG_PTR(void));

//define function pointer, no argument
#define CL_API_ARGS0(name, return_type) \
	typedef return_type (*name)();
//define function pointer, has 1 argument
#define CL_API_ARGS1(name, return_type, arg1_type) \
	typedef return_type (*name)(arg1_type);
//define function pointer, has 2 arguments
#define CL_API_ARGS2(name, return_type, arg1_type, arg2_type) \
	typedef return_type (*name)(arg1_type, arg2_type);
//define function pointer, has 3 arguments
#define CL_API_ARGS3(name, return_type, arg1_type, arg2_type, arg3_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type);
//define function pointer, has 4 arguments
#define CL_API_ARGS4(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type);
//define function pointer, has 5 arguments
#define CL_API_ARGS5(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type);
//define function pointer, has 6 arguments
#define CL_API_ARGS6(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type);
//define function pointer, has 7 arguments
#define CL_API_ARGS7(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type);
//define function pointer, has 8 arguments
#define CL_API_ARGS8(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type);
//define function pointer, has 9 arguments
#define CL_API_ARGS9(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type);
//define function pointer, has 10 arguments
#define CL_API_ARGS10(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type);
//define function pointer, has 11 arguments
#define CL_API_ARGS11(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type);
//define function pointer, has 12 arguments
#define CL_API_ARGS12(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type, arg12_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type, arg12_type);
//define function pointer, has 13 arguments
#define CL_API_ARGS13(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type, arg12_type, arg13_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type, arg12_type, arg13_type);
//define function pointer, has 14 arguments
#define CL_API_ARGS14(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type, arg12_type, arg13_type, arg14_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type, arg12_type, arg13_type, arg14_type);
//define function pointer, has 15 arguments
#define CL_API_ARGS15(name, return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type, arg12_type, arg13_type, arg14_type, arg15_type) \
	typedef return_type (*name)(arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type, arg11_type, arg12_type, arg13_type, arg14_type, arg15_type);

//reinterpret_cast<CLGetPlatformIDs>(base::GetFunctionPointerFromNativeLibrary(opencl_library, "clGetPlatformIDs"));
// #define CL_API_LOAD(library, api_name, name)
// 		reinterpret_cast<name>(base::GetFunctionPointerFromNativeLibrary(library, api_name));
#define CL_API_LOAD(library, api_name, name) \
		base::GetFunctionPointerFromNativeLibrary(library, api_name) ? reinterpret_cast<name>(base::GetFunctionPointerFromNativeLibrary(library, api_name)) : reinterpret_cast<name>(handleFuncLookupFail(api_name));

#endif  // UI_OPENCL_OPENCL_INCLUDE_H_
