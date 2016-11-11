/*
 * opencl_test.cc
 *
 *  Created on: 2015. 6. 5.
 *      Author: jphofasb
 */

#include "ui/opencl/opencl_implementation.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "gpu/ipc/service/gpu_channel.h"
#include "gpu/ipc/common/gpu_messages.h"
#include "dirent.h"
#include "gpu/ipc/service/gpu_channel_manager.h"

#include <array>

#if defined(OS_LINUX)
gpu::GpuChannel* gfx::CLApi::parent_channel_;
#endif

namespace gfx {

void InitializeStaticCLBindings(CLApi* apiImpl) {
	DLOG(INFO) << "InitializeStaticCLBindings";

	// determine so library
	std::string clSoFile = "libOpenCL.so";

#if defined(OS_ANDROID)
// check ICD Loader (*.icd) exist in "/system/vendor/Khronos/OpenCL/vendors/*.icd"
// http://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
	DIR *dir;
	struct dirent *ent;
	struct stat st;

	std::string directory = "/system/vendor/Khronos/OpenCL/vendors";

	dir = opendir("/system/vendor/Khronos/OpenCL/vendors");
	if (NULL != dir) {
		while ((ent = readdir(dir)) != NULL) {
			const std::string file_name = ent->d_name;
			const std::string full_file_name = directory + "/" + file_name;

			if (file_name[0] == '.')
			   continue;

			if (stat(full_file_name.c_str(), &st) == -1)
			   continue;

			const bool is_directory = (st.st_mode & S_IFDIR) != 0;

			if (is_directory)
			   continue;

			DLOG(INFO) << "full_file_name : " << full_file_name;

			base::FilePath icdFilePath = base::FilePath(full_file_name);
			std::string soFileName;

			bool result = base::ReadFileToString(icdFilePath, &soFileName);
			DLOG(INFO) << "base::ReadFileToString result : " << result;

			if(result) {
				// http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
				soFileName.erase(soFileName.find_last_not_of(" \n\r\t")+1);

				DLOG(INFO) << "soFileName : " << soFileName;
				DLOG(INFO) << "soFileName.length() : " << soFileName.length();
				clSoFile = soFileName;
			}

			break;
		}

		closedir(dir);
	}
#endif

	//load so library
	DLOG(INFO) << "clSoFile : " << clSoFile;
	base::FilePath fileName = base::FilePath(clSoFile);
	base::NativeLibraryLoadError error;
	base::NativeLibrary opencl_library = base::LoadNativeLibrary(fileName, &error);
	if (!opencl_library) {
		DLOG(INFO) << "load library failed!!";
		// [ERROR:native_library_posix.cc(41)] dlclose failed: NULL library handle
		// base::UnloadNativeLibrary(opencl_library);
	}
	else {
		DLOG(INFO) << "load library success!!";
		apiImpl->InitApi(opencl_library);
	}
}

CLApi::CLApi(){
	clLibraryLoaded_ = false;
}

CLApi::~CLApi(){
}

static void* handleFuncLookupFail(std::string api_name) {
	DLOG(INFO) << "handleFuncLookupFail, api_name : " << api_name;

	return nullptr;
}

#if defined(OS_ANDROID)
void CLApi::setChannel(gpu::GpuChannel* channel) {
	DLOG(INFO) << "CLApi::setChannel, CLApi : " << this << ", channel : " << channel;
	gpu_channel_ = channel;
};
#endif

void CLApi::InitApi(base::NativeLibrary nativeLib) {
	DLOG(INFO) << "CLApi::InitApi";

	opencl_library_ = nativeLib;

	cl_get_platform_ids_ = CL_API_LOAD(opencl_library_, "clGetPlatformIDs", CLGetPlatformIDs);
	cl_get_info_ = CL_API_LOAD(opencl_library_, "clGetPlatformInfo", CLGetInfo);
	cl_get_device_ids_ = CL_API_LOAD(opencl_library_, "clGetDeviceIDs", CLGetDeviceIDs);
	cl_get_device_info_ = CL_API_LOAD(opencl_library_, "clGetDeviceInfo", CLGetDeviceInfo);
	cl_create_context_ = CL_API_LOAD(opencl_library_, "clCreateContext", CLCreateContext);
	cl_create_context_from_type_ = CL_API_LOAD(opencl_library_, "clCreateContextFromType", CLCreateContextFromType);
	cl_wait_for_events_ = CL_API_LOAD(opencl_library_, "clWaitForEvents", CLWaitForEvents);
	cl_create_buffer_ = CL_API_LOAD(opencl_library_, "clCreateBuffer", CLCreateBuffer);
	cl_get_mem_object_info_ = CL_API_LOAD(opencl_library_, "clGetMemObjectInfo", CLGetMemObjectInfo);
	cl_create_sub_buffer_ = CL_API_LOAD(opencl_library_, "clCreateSubBuffer", CLCreateSubBuffer);
	cl_create_sampler_ = CL_API_LOAD(opencl_library_, "clCreateSampler", CLCreateSampler);
	cl_get_sampler_info_ = CL_API_LOAD(opencl_library_, "clGetSamplerInfo", CLGetSamplerInfo);
	cl_release_sampler_ = CL_API_LOAD(opencl_library_, "clReleaseSampler", CLReleaseSampler);
	cl_get_image_info_ = CL_API_LOAD(opencl_library_, "clGetImageInfo", CLGetImageInfo);
	cl_get_event_info_ = CL_API_LOAD(opencl_library_, "clGetEventInfo", CLGetEventInfo);
	cl_get_event_profiling_info_ = CL_API_LOAD(opencl_library_, "clGetEventProfilingInfo", CLGetEventProfilingInfo);
	cl_set_event_callback_ = CL_API_LOAD(opencl_library_, "clSetEventCallback", CLSetEventCallback);
	cl_release_event_ = CL_API_LOAD(opencl_library_, "clReleaseEvent", CLReleaseEvent);
	cl_get_context_info = CL_API_LOAD(opencl_library_, "clGetContextInfo", CLGetContextInfo);
	cl_create_image_2d_ = CL_API_LOAD(opencl_library_, "clCreateImage2D", CLCreateImage2D);
	cl_set_user_event_status_ = CL_API_LOAD(opencl_library_, "clSetUserEventStatus", CLSetUserEventStatus);
	cl_create_user_event_ = CL_API_LOAD(opencl_library_, "clCreateUserEvent", CLCreateUserEvent);
	cl_create_command_queue_ = CL_API_LOAD(opencl_library_, "clCreateCommandQueue", CLCreateCommandQueue);
	cl_get_supported_image_format_ = CL_API_LOAD(opencl_library_, "clGetSupportedImageFormats", CLGetSupportedImageFormat);
	cl_get_command_queue_info_ = CL_API_LOAD(opencl_library_, "clGetCommandQueueInfo", CLGetCommandQueueInfo);
	cl_release_context_ = CL_API_LOAD(opencl_library_, "clReleaseContext", CLReleaseContext);
	cl_release_program_ = CL_API_LOAD(opencl_library_, "clReleaseProgram", CLReleaseProgram);
	cl_release_kernel_ = CL_API_LOAD(opencl_library_, "clReleaseKernel", CLReleaseKernel);
	cl_release_mem_object_ = CL_API_LOAD(opencl_library_, "clReleaseMemObject", CLReleaseMemObject);
	cl_release_command_queue_ = CL_API_LOAD(opencl_library_, "clReleaseCommandQueue", CLReleaseCommandQueue);
	cl_enqueue_copy_buffer_ = CL_API_LOAD(opencl_library_, "clEnqueueCopyBuffer", CLEnqueueCopyBuffer);
	cl_enqueue_copy_buffer_rect_ = CL_API_LOAD(opencl_library_, "clEnqueueCopyBufferRect", CLEnqueueCopyBufferRect);
	cl_enqueue_copy_image_ = CL_API_LOAD(opencl_library_, "clEnqueueCopyImage", CLEnqueueCopyImage);
	cl_enqueue_copy_buffer_to_image_ = CL_API_LOAD(opencl_library_, "clEnqueueCopyBufferToImage", CLEnqueueCopyBufferToImage);
	cl_enqueue_copy_image_to_buffer_ = CL_API_LOAD(opencl_library_, "clEnqueueCopyImageToBuffer", CLEnqueueCopyImageToBuffer);
	cl_enqueue_read_buffer_ = CL_API_LOAD(opencl_library_, "clEnqueueReadBuffer", CLEnqueueReadBuffer);
	cl_enqueue_read_buffer_rect_ = CL_API_LOAD(opencl_library_, "clEnqueueReadBufferRect", CLEnqueueReadBufferRect);
	cl_enqueue_read_image_ = CL_API_LOAD(opencl_library_, "clEnqueueReadImage", CLEnqueueReadImage);
	cl_enqueue_write_buffer_ = CL_API_LOAD(opencl_library_, "clEnqueueWriteBuffer", CLEnqueueWriteBuffer);
	cl_enqueue_write_buffer_rect_ = CL_API_LOAD(opencl_library_, "clEnqueueWriteBufferRect", CLEnqueueWriteBufferRect);
	cl_enqueue_write_image_ = CL_API_LOAD(opencl_library_, "clEnqueueWriteImage", CLEnqueueWriteImage);
	cl_enqueue_n_d_range_kernel_ = CL_API_LOAD(opencl_library_, "clEnqueueNDRangeKernel", CLEnqueueNDRangeKernel);
	cl_enqueue_marker_ = CL_API_LOAD(opencl_library_, "clEnqueueMarker", CLEnqueueMarker);
	cl_enqueue_barrier_ = CL_API_LOAD(opencl_library_, "clEnqueueBarrier", CLEnqueueBarrier);
	cl_enqueue_wait_for_events_ = CL_API_LOAD(opencl_library_, "clEnqueueWaitForEvents", CLEnqueueWaitForEvents);
	cl_finish_ = CL_API_LOAD(opencl_library_, "clFinish", CLFinish);
	cl_flush_ = CL_API_LOAD(opencl_library_, "clFlush", CLFlush);
	cl_get_kernel_info_ = CL_API_LOAD(opencl_library_, "clGetKernelInfo", CLGetKernelInfo);
	cl_get_kernel_work_group_info_ = CL_API_LOAD(opencl_library_, "clGetKernelWorkGroupInfo", CLGetKernelWorkGroupInfo);
	cl_get_kernel_arg_info_ = CL_API_LOAD(opencl_library_, "clGetKernelArgInfo", CLGetKernelArgInfo);
	cl_set_kernel_arg_ = CL_API_LOAD(opencl_library_, "clSetKernelArg", CLSetKernelArg);
	cl_get_program_info_ = CL_API_LOAD(opencl_library_, "clGetProgramInfo", CLGetProgramInfo);
	cl_create_program_with_source_ = CL_API_LOAD(opencl_library_, "clCreateProgramWithSource", CLCreateProgramWithSource);
	cl_get_program_build_info_ = CL_API_LOAD(opencl_library_, "clGetProgramBuildInfo", CLGetProgramBuildInfo);
	cl_build_program_ = CL_API_LOAD(opencl_library_, "clBuildProgram", CLBuildProgram);
	cl_create_kernel_ = CL_API_LOAD(opencl_library_, "clCreateKernel", CLCreateKernel);
	cl_create_kernels_in_program_ = CL_API_LOAD(opencl_library_, "clCreateKernelsInProgram", CLCreateKernelsInProgram);
	// gl/cl sharing
	cl_create_from_gl_buffer_ = CL_API_LOAD(opencl_library_, "clCreateFromGLBuffer", CLCreateFromGLBuffer);
	cl_create_from_gl_render_buffer_ = CL_API_LOAD(opencl_library_, "clCreateFromGLRenderbuffer", CLCreateFromGLRenderbuffer);
	cl_create_from_gl_texture_2d_ = CL_API_LOAD(opencl_library_, "clCreateFromGLTexture2D", CLCreateFromGLTexture2D);
	cl_get_gl_object_info_ = CL_API_LOAD(opencl_library_, "clGetGLObjectInfo", CLGetGLObjectInfo);
	cl_get_gl_texture_info_ = CL_API_LOAD(opencl_library_, "clGetGLTextureInfo", CLGetGLTextureInfo);
	cl_enqueue_acquire_globjects_ = CL_API_LOAD(opencl_library_, "clEnqueueAcquireGLObjects", CLEnqueueAcquireGLObjects);
	cl_enqueue_release_globjects_ = CL_API_LOAD(opencl_library_, "clEnqueueReleaseGLObjects", CLEnqueueReleaseGLObjects);

	if(opencl_library_ && cl_get_platform_ids_ && cl_get_info_) {
		clLibraryLoaded_ = true;
	}

	DLOG(INFO) << ">>clLibraryLoaded_=" << clLibraryLoaded_;
}

bool CLApi::doClTest() {
	DLOG(INFO) << "CLApi::doClTest";

	if(!clLibraryLoaded_) {
		return false;
	}

	cl_int err = 0;
	cl_uint m_num_platforms = 0;
	cl_platform_id* m_cl_platforms = NULL;

	err = cl_get_platform_ids_(0, NULL, &m_num_platforms);
	if(err != 0) {
		return false;
	}

	m_cl_platforms = new cl_platform_id[m_num_platforms];
	err = cl_get_platform_ids_(m_num_platforms, m_cl_platforms, NULL);
	if(err != 0) {
		return false;
	}

	char platform_string[1024];
	err = cl_get_info_(m_cl_platforms[0], 0x0902, sizeof(platform_string), &platform_string, NULL);
	if(err != 0) {
		return false;
	}

	CLLOG(INFO) << "doClTest result = " << platform_string;
	return true;
}

cl_int CLApi::doclGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms)
{
	CLLOG(INFO) << "WEBCL::CLApi::doclGetPlatformIDs";
	cl_int err = cl_get_platform_ids_(num_entries, platforms, num_platforms);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>num_platforms=" << num_platforms << ", num_entries=" << num_entries << ", err=" << err;

	return err;
}

cl_int CLApi::doClGetPlatformInfo(
		cl_platform_id platform,
		cl_platform_info param_name,
		size_t param_value_size,
		char* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "WEBCL::CLApi::doClGetPlatformInfo, platform=" << platform << ", param_name=" << param_name;
	cl_int err = cl_get_info_(platform, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>result=" << param_value;
	return err;
}

cl_int CLApi::doClGetDeviceIDs(
		cl_platform_id platform,
		cl_device_type device_type,
		cl_uint num_entries,
		cl_device_id* devices,
		cl_uint* num_devices)
{
	CLLOG(INFO) << "WEBCL::CLApi::doClGetDeviceIDs, platform=" << platform << ", cl_device_type=" << device_type;
	cl_int err = cl_get_device_ids_(platform, device_type, num_entries, devices, num_devices);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>result=" << devices << "::" << *num_devices;
	return err;
}

cl_int CLApi::doClGetDeviceInfo(
		cl_device_id device,
		cl_device_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "doClGetDeviceInfo, device=" << device << ", param_name=" << param_name;
	cl_int err = cl_get_device_info_(device, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>result=" << err;
	return err;
}

cl_context CLApi::doClCreateContext(
		cl_context_properties* properties,
		cl_uint num_devices,
		cl_device_id* devices,
		CL_CallbackclCreateContextFromType callback,
		void* user_data,
		cl_int* errcode_ret)
{
	CLLOG(INFO) << "doClCreateContext";
	CLLOG(INFO) << ">>num_devices=" << num_devices << ", devices=" << devices;
	cl_context context = cl_create_context_(properties, num_devices, devices, callback, user_data, errcode_ret);
	CLLOG(INFO) << ">>err=" << *errcode_ret;
	return context;
}

cl_context CLApi::doClCreateContextFromType(
		cl_context_properties* properties,
		cl_device_type device_type,
		CL_CallbackclCreateContextFromType callback,
		void* user_data,
		cl_int* errcode_ret)
{
	CLLOG(INFO) << "doClCreateContextFromType";
	cl_context context = cl_create_context_from_type_(properties, device_type, callback, user_data, errcode_ret);
	CLLOG(INFO) << ">>err=" << *errcode_ret;
	return context;
}

cl_int CLApi::doClWaitForEvents(
		cl_uint num_events,
		const cl_event* event_list)
{
	CLLOG(INFO) << "doClWaitForEvents";
	cl_int errcode = cl_wait_for_events_(num_events, event_list);
	CLLOG(INFO) << ">>err=" << errcode;
	return errcode;
}

void CLApi::doClCreateBuffer()
{
	CLLOG(INFO) << "doClCreateBuffer";

	cl_int errcode_ret = CL_INVALID_VALUE;

	mSharedResultPtr->ptr_1 = (cl_point)cl_create_buffer_(
			(cl_context)mSharedOperationPtr->ptr_1,
			mSharedOperationPtr->ulong_01,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->bool_01?mSharedDataPtr:nullptr,
			&errcode_ret);

	mSharedResultPtr->int_01 = errcode_ret;

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;
}

cl_int CLApi::doClGetMemObjectInfo(
		cl_mem memobj,
		cl_mem_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "doClGetMemObjectInfo, param_name=" << param_name << ", memobj=" << memobj;
	cl_int err = cl_get_mem_object_info_(memobj, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_mem CLApi::doClCreateSubBuffer(
		cl_mem buffer,
		cl_mem_flags flags,
		cl_buffer_create_type buffer_create_type,
		const void* buffer_create_info,
		cl_int* errcode_ret)
{
	CLLOG(INFO) << "doClCreateSubBuffer, buffer=" << buffer << ", flags=" << flags << ", type=" << buffer_create_type << ", origin=" << ((cl_buffer_region*)buffer_create_info)->origin << ", size=" << ((cl_buffer_region*)buffer_create_info)->size;
	cl_mem sub_buffer = cl_create_sub_buffer_(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret);
	CLLOG(INFO) << ">>err=" << *errcode_ret;
	return sub_buffer;
}

cl_sampler CLApi::doClCreateSampler(
		cl_context context,
		cl_bool normalized_coords,
		cl_addressing_mode addressing_mode,
		cl_filter_mode filter_mode,
		cl_int* errcode_ret)
{
	CLLOG(INFO) << "doClCreateSampler";
	cl_sampler sampler = cl_create_sampler_(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);
	return sampler;
}

cl_int CLApi::doClGetSamplerInfo(
		cl_sampler sampler,
		cl_sampler_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "doClGetSamplerInfo, param_name=" << param_name;
	cl_int err = cl_get_sampler_info_(sampler, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClReleaseSampler(
		cl_sampler sampler)
{
	CLLOG(INFO) << "doClReleaseSampler";
	cl_int err = cl_release_sampler_(sampler);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClGetImageInfo(
		cl_mem image,
		cl_image_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "doClGetImageInfo, param_name=" << param_name << ", image=" << image;
	cl_int err = cl_get_image_info_(image, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClGetEventInfo (
		cl_event event,
		cl_event_info param_name,
		size_t param_value_size,
		void *param_value,
		size_t *param_value_size_ret)
{
	CLLOG(INFO) << "doClGetEventInfo";
	cl_int err = cl_get_event_info_(event, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClGetContextInfo(
		cl_context context,
		cl_context_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "doClGetContextInfo, param_name=" << param_name << ", context=" << context;
	cl_int err = cl_get_context_info(context, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClGetEventProfilingInfo (
		cl_event event,
		cl_profiling_info param_name,
		size_t param_value_size,
		void *param_value,
		size_t *param_value_size_ret)
{
	CLLOG(INFO) << "dlClGetEventProfilingInfo";
	cl_int err = cl_get_event_profiling_info_(event, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClSetEventCallback (
		cl_event event,
		cl_int  command_exec_callback_type ,
		CL_CallbackclSetEvent callback,
		void *user_data)
{
	CLLOG(INFO) << "doClSetEventCallback";
	CLLOG(INFO) << "CLApi::doClSetEventCallback, CLApi : " << this << ", gpu_channel_ : " << gpu_channel_;
#if defined(OS_ANDROID)
	((unsigned*)user_data)[4] = reinterpret_cast<unsigned>(gpu_channel_);
#endif
	cl_int err = cl_set_event_callback_(event, command_exec_callback_type, &this->WebCLCallbackPtr, user_data);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClReleaseEvent (
		cl_event event)
{
	CLLOG(INFO) << "doClReleaseEvent";
	cl_int err = cl_release_event_(event);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

void CLApi::doClCreateImage2D()
{
	CLLOG(INFO) << "doClCreateImage2D";

	cl_image_format image_format = {
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02
	};

	cl_int errcode_ret = CL_INVALID_VALUE;

	mSharedResultPtr->ptr_1 = (cl_point)cl_create_image_2d_(
			(cl_context)mSharedOperationPtr->ptr_1,
			mSharedOperationPtr->ulong_01,
			&image_format,
			mSharedOperationPtr->uint_03,
			mSharedOperationPtr->uint_04,
			mSharedOperationPtr->uint_05,
			mSharedOperationPtr->bool_01?mSharedDataPtr:nullptr,
			&errcode_ret);

	mSharedResultPtr->int_01 = errcode_ret;

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;
}

cl_int CLApi::doClSetUserEventStatus(
		cl_event clEvent,
		cl_int executionStatus)
{
	CLLOG(INFO) << "doClSetUserEventStatus";
	CLLOG(INFO) << ">>executionStatus : " << executionStatus;

	cl_int err = cl_set_user_event_status_(clEvent, executionStatus);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_event CLApi::doClCreateUserEvent(
		cl_context mContext,
		cl_int* errcode_ret)
{
	CLLOG(INFO) << "doClCreateUserEvent";

	cl_event mEvent = cl_create_user_event_(mContext, errcode_ret);
	CLLOG(INFO) << ">>err=" << *errcode_ret;
	return mEvent;
}

cl_command_queue CLApi::doClCreateCommandQueue(
		cl_context context,
		cl_device_id device,
		cl_command_queue_properties properties,
		cl_int* errcode_ret)
{
	CLLOG(INFO) << "doClCreateCommandQueue, context=" << context << ", device=" << device << ", properties=" << properties;

	cl_command_queue comand_queue = cl_create_command_queue_(context, device, properties, errcode_ret);
	CLLOG(INFO) << ">>err=" << *errcode_ret;
	return comand_queue;
}

cl_int CLApi::doClGetSupportedImageFormat(
		cl_context context,
		cl_mem_flags flags,
		cl_mem_object_type image_type,
		cl_uint num_entries,
		cl_image_format* image_formats,
		cl_uint* num_image_formats)
{
	CLLOG(INFO) << "doClGetSupportedImageFormat";
	CLLOG(INFO) << ">>context=" << context << " ,flags=" << flags << ", image_type=" << image_type << ", num_entries=" << num_entries;

	cl_int err = cl_get_supported_image_format_(context, flags, image_type, num_entries, image_formats, num_image_formats);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClGetCommandQueueInfo(
		cl_command_queue command_queue,
		cl_command_queue_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "doClGetCommandQueueInfo, param_name=" << param_name << ", command_queue=" << command_queue << ", param_value=" << param_value;
	cl_int err = cl_get_command_queue_info_(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClReleaseContext(
		cl_context context)
{
	CLLOG(INFO) << "doClReleaseContext";
	cl_int err = cl_release_context_(context);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClReleaseProgram(
		cl_program program)
{
	CLLOG(INFO) << "doClReleaseProgram";
	cl_int err = cl_release_program_(program);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClReleaseKernel(
		cl_kernel kernel)
{
	CLLOG(INFO) << "doClReleaseKernel " << (cl_kernel)kernel;
	cl_int err = cl_release_kernel_(kernel);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClReleaseMemObject(
		cl_mem memobj)
{
	CLLOG(INFO) << "doClReleaseMemObject";
	cl_int err = cl_release_mem_object_(memobj);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClReleaseCommandQueue(
		cl_command_queue command_queue)
{
	CLLOG(INFO) << "doClReleaseCommandQueue";
	cl_int err = cl_release_command_queue_(command_queue);
	CLLOG(INFO) << ">>err=" << err;

	//delete all garbage pointer
	size_t garbage_size = 0;

	garbage_size = garbage_ptr_list_unsigned_char.size();
	for(size_t i=0; i<garbage_size; i++)
		delete[] (unsigned char*)garbage_ptr_list_unsigned_char[i];

	garbage_size = garbage_ptr_list_unsigned.size();
	for(size_t i=0; i<garbage_size; i++)
		delete[] (unsigned*)garbage_ptr_list_unsigned[i];

	garbage_size = garbage_ptr_list_int.size();
	for(size_t i=0; i<garbage_size; i++)
		delete[] (int*)garbage_ptr_list_int[i];

	garbage_size = garbage_ptr_list_float.size();
	for(size_t i=0; i<garbage_size; i++)
		delete[] (float*)garbage_ptr_list_float[i];

	garbage_size = garbage_ptr_list_double.size();
	for(size_t i=0; i<garbage_size; i++)
		delete[] (double*)garbage_ptr_list_double[i];

	garbage_size = garbage_ptr_list_cl_mem.size();
	for(size_t i=0; i<garbage_size; i++)
		delete[] (cl_mem*)garbage_ptr_list_cl_mem[i];

	garbage_ptr_list_unsigned_char.clear();
	std::vector<cl_point>().swap(garbage_ptr_list_unsigned_char);
	garbage_ptr_list_unsigned.clear();
	std::vector<cl_point>().swap(garbage_ptr_list_unsigned);
	garbage_ptr_list_int.clear();
	std::vector<cl_point>().swap(garbage_ptr_list_int);
	garbage_ptr_list_float.clear();
	std::vector<cl_point>().swap(garbage_ptr_list_float);
	garbage_ptr_list_double.clear();
	std::vector<cl_point>().swap(garbage_ptr_list_double);
	garbage_ptr_list_cl_mem.clear();
	std::vector<cl_point>().swap(garbage_ptr_list_cl_mem);

	return err;
}

void CLApi::doClFinish()
{
	CLLOG(INFO) << "doClFinish";
	mSharedResultPtr->int_01 = cl_finish_((cl_command_queue)mSharedOperationPtr->ptr_1);

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;
}

cl_int CLApi::doClFlush(
		cl_command_queue command_queue)
{
	CLLOG(INFO) << "doClFlush, command_queue=" << command_queue;
	cl_int err = cl_flush_(command_queue);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

void CLApi::doClEnqueueCopyBuffer()
{
	CLLOG(INFO) << "doClEnqueueCopyBuffer";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_04) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_04];
		for(size_t i=0; i<mSharedOperationPtr->uint_04; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_copy_buffer_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			(cl_mem)mSharedOperationPtr->ptr_3,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			mSharedOperationPtr->uint_03,
			mSharedOperationPtr->uint_04,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueCopyBufferRect()
{
	CLLOG(INFO) << "doClEnqueueCopyBufferRect";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_05) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_05];
		for(size_t i=0; i<mSharedOperationPtr->uint_05; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}


	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_copy_buffer_rect_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			(cl_mem)mSharedOperationPtr->ptr_3,
			(size_t*)mSharedOperationPtr->uint_arr_01,
			(size_t*)mSharedOperationPtr->uint_arr_02,
			(size_t*)mSharedOperationPtr->uint_arr_03,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			mSharedOperationPtr->uint_03,
			mSharedOperationPtr->uint_04,
			mSharedOperationPtr->uint_05,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueCopyImage()
{
	CLLOG(INFO) << "doClEnqueueCopyImage";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_01) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_01];
		for(size_t i=0; i<mSharedOperationPtr->uint_01; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_copy_image_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			(cl_mem)mSharedOperationPtr->ptr_3,
			(size_t*)mSharedOperationPtr->uint_arr_01,
			(size_t*)mSharedOperationPtr->uint_arr_02,
			(size_t*)mSharedOperationPtr->uint_arr_03,
			mSharedOperationPtr->uint_01,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueCopyBufferToImage()
{
	CLLOG(INFO) << "doClEnqueueCopyBufferToImage";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_02) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_02];
		for(size_t i=0; i<mSharedOperationPtr->uint_02; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_copy_buffer_to_image_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			(cl_mem)mSharedOperationPtr->ptr_3,
			mSharedOperationPtr->uint_01,
			(size_t*)mSharedOperationPtr->uint_arr_01,
			(size_t*)mSharedOperationPtr->uint_arr_02,
			mSharedOperationPtr->uint_02,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueCopyImageToBuffer()
{
	CLLOG(INFO) << "doClEnqueueCopyImageToBuffer";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_02) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_02];
		for(size_t i=0; i<mSharedOperationPtr->uint_02; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_copy_image_to_buffer_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			(cl_mem)mSharedOperationPtr->ptr_3,
			(size_t*)mSharedOperationPtr->uint_arr_01,
			(size_t*)mSharedOperationPtr->uint_arr_02,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueReadBuffer()
{
	CLLOG(INFO) << "doClEnqueueReadBuffer";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_03) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_03];
		for(size_t i=0; i<mSharedOperationPtr->uint_03; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_read_buffer_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			mSharedOperationPtr->bool_01,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			mSharedOperationPtr->bool_01?mSharedDataPtr:handleBlockedCallData(mSharedOperationPtr->int_01, mSharedOperationPtr->uint_04),
			mSharedOperationPtr->uint_03,
			event_wait_list_inter,
			mSharedOperationPtr->bool_02?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_02) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>result= "<< mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueReadBufferRect()
{
	CLLOG(INFO) << "doClEnqueueReadBufferRect";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_05) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_05];
		for(size_t i=0; i<mSharedOperationPtr->uint_05; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_read_buffer_rect_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			mSharedOperationPtr->bool_02,
			(size_t*)mSharedOperationPtr->uint_arr_01,
			(size_t*)mSharedOperationPtr->uint_arr_02,
			(size_t*)mSharedOperationPtr->uint_arr_03,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			mSharedOperationPtr->uint_03,
			mSharedOperationPtr->uint_04,
			mSharedOperationPtr->bool_02?mSharedDataPtr:handleBlockedCallData(mSharedOperationPtr->int_01, mSharedOperationPtr->uint_06),
			mSharedOperationPtr->uint_05,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)


	CLLOG(INFO) << ">>result= "<< mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueReadImage()
{
	CLLOG(INFO) << "doClEnqueueReadImage, blocking=" << mSharedOperationPtr->bool_02 << ", need event=" << mSharedOperationPtr->bool_01;

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_03) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_03];
		for(size_t i=0; i<mSharedOperationPtr->uint_03; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_read_image_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			mSharedOperationPtr->bool_02,
			(size_t*)mSharedOperationPtr->uint_arr_01,
			(size_t*)mSharedOperationPtr->uint_arr_02,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			mSharedOperationPtr->bool_02?mSharedDataPtr:handleBlockedCallData(mSharedOperationPtr->int_01, mSharedOperationPtr->uint_04),
			mSharedOperationPtr->uint_03,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>result= "<< mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueWriteBuffer()
{
	CLLOG(INFO) << "doClEnqueueReadImage";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_03) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_03];
		for(size_t i=0; i<mSharedOperationPtr->uint_03; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_write_buffer_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			mSharedOperationPtr->bool_02,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			mSharedOperationPtr->bool_02?mSharedDataPtr:handleBlockedCallData(mSharedOperationPtr->int_01, mSharedOperationPtr->uint_04),
			mSharedOperationPtr->uint_03,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>result= "<< mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueWriteBufferRect()
{
	CLLOG(INFO) << "doClEnqueueWriteBufferRect";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_05) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_05];
		for(size_t i=0; i<mSharedOperationPtr->uint_05; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_write_buffer_rect_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			mSharedOperationPtr->bool_02,
			(size_t*)mSharedOperationPtr->uint_arr_01,
			(size_t*)mSharedOperationPtr->uint_arr_02,
			(size_t*)mSharedOperationPtr->uint_arr_03,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			mSharedOperationPtr->uint_03,
			mSharedOperationPtr->uint_04,
			mSharedOperationPtr->bool_02?mSharedDataPtr:handleBlockedCallData(mSharedOperationPtr->int_01, mSharedOperationPtr->uint_06),
			mSharedOperationPtr->uint_05,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>result= "<< mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueWriteImage()
{
	CLLOG(INFO) << "doClEnqueueWriteImage";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_03) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_03];
		for(size_t i=0; i<mSharedOperationPtr->uint_03; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_write_image_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_mem)mSharedOperationPtr->ptr_2,
			mSharedOperationPtr->bool_02,
			(size_t*)mSharedOperationPtr->uint_arr_01,
			(size_t*)mSharedOperationPtr->uint_arr_02,
			mSharedOperationPtr->uint_01,
			mSharedOperationPtr->uint_02,
			mSharedOperationPtr->bool_02?mSharedDataPtr:handleBlockedCallData(mSharedOperationPtr->int_01, mSharedOperationPtr->uint_04),
			mSharedOperationPtr->uint_03,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)

	CLLOG(INFO) << ">>result= "<< mSharedResultPtr->int_01;
}

void CLApi::doClEnqueueNDRangeKernel()
{
	CLLOG(INFO) << "doClEnqueueNDRangeKernel";

	cl_event* event_wait_list_inter = nullptr;
	if(mSharedOperationPtr->uint_05) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_05];
		for(size_t i=0; i<mSharedOperationPtr->uint_05; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	size_t* global_work_offset_inter = nullptr;
	size_t* global_work_size_inter = nullptr;
	size_t* local_work_size_inter = nullptr;

	size_t* data_ptr_inter = static_cast<size_t*>(mSharedDataPtr);

	if(mSharedOperationPtr->uint_02) {
		global_work_offset_inter = new size_t[mSharedOperationPtr->uint_02];
		memcpy(global_work_offset_inter, data_ptr_inter, sizeof(size_t)*mSharedOperationPtr->uint_02);
	}
	if(mSharedOperationPtr->uint_03) {
		global_work_size_inter = new size_t[mSharedOperationPtr->uint_03];
		memcpy(global_work_size_inter, data_ptr_inter+mSharedOperationPtr->uint_02, sizeof(size_t)*mSharedOperationPtr->uint_03);

		for(size_t i=0; i<mSharedOperationPtr->uint_03; i++)
			CLLOG(INFO) << global_work_size_inter[i];
	}
	if(mSharedOperationPtr->uint_04) {
		local_work_size_inter = new size_t[mSharedOperationPtr->uint_04];
		memcpy(local_work_size_inter, data_ptr_inter+mSharedOperationPtr->uint_02+mSharedOperationPtr->uint_03, sizeof(size_t)*mSharedOperationPtr->uint_04);
	}

	cl_event event_inter;
	mSharedResultPtr->int_01 = cl_enqueue_n_d_range_kernel_(
			(cl_command_queue)mSharedOperationPtr->ptr_1,
			(cl_kernel)mSharedOperationPtr->ptr_2,
			mSharedOperationPtr->uint_01,
			global_work_offset_inter,
			global_work_size_inter,
			local_work_size_inter,
			mSharedOperationPtr->uint_05,
			event_wait_list_inter,
			mSharedOperationPtr->bool_01?&event_inter:nullptr);

	CLLOG(INFO) << ">>result= "<< mSharedResultPtr->int_01;

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)
	DELETE_ARRAY(global_work_offset_inter)
	DELETE_ARRAY(global_work_size_inter)
	DELETE_ARRAY(local_work_size_inter)
}

cl_int CLApi::doClGetKernelInfo(
		cl_kernel kernel,
		cl_kernel_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "WEBCL::CLApi::doClGetKernelInfo, kernel=" << kernel << ", param_name=" << param_name;
	cl_int err = cl_get_kernel_info_(kernel, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>result=" << param_value;
	return err;
}

cl_int CLApi::doClGetKernelWorkGroupInfo(
		cl_kernel kernel,
		cl_device_id device,
		cl_kernel_work_group_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "WEBCL::CLApi::doClGetKernelWorkGroupInfo, kernel=" << kernel << ", param_name=" << param_name << ", device_id=" << device;
	cl_int err = cl_get_kernel_work_group_info_(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>result=" << param_value;
	return err;
}

cl_int CLApi::doClGetKernelArgInfo(
		cl_kernel kernel,
		cl_uint arg_indx,
		cl_kernel_arg_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "WEBCL::CLApi::doClGetKernelArgInfo, kernel=" << kernel << ", param_name=" << param_name << ",arg_indx=" << arg_indx;
	CLLOG(INFO) << "kernel : " << kernel << " , arg_indx : " << arg_indx << ", param_name = " << param_name << ", param_value_size = " << param_value_size << ", param_value = " << param_value << ", param_value_size_ret : " << param_value_size_ret;
	if (cl_get_kernel_arg_info_ == nullptr) {
		CLLOG(INFO) << "cl_get_kernel_arg_info is Not Supported";
		return CL_NOT_SUPPORTED_FUNCTION;
	}
	cl_int err = cl_get_kernel_arg_info_(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>result=" << param_value;
	return err;
}

void CLApi::doClSetKernelArg()
{
	CLLOG(INFO) << "doClSetKernelArg_SHM" << mSharedOperationPtr->int_01;

	if (mSharedOperationPtr->bool_01) {
		mSharedResultPtr->int_01 = cl_set_kernel_arg_(
						(cl_kernel)mSharedOperationPtr->ptr_1,
						mSharedOperationPtr->uint_01,
						(size_t)mSharedOperationPtr->uint_02,
						NULL);
	}
	else {
		switch(mSharedOperationPtr->int_01) {
			case SETARG_DATA_TYPE::SETARG_CL_SAMPLER:
			case SETARG_DATA_TYPE::SETARG_CL_MEM:
			{
				void* data = (void*)mSharedOperationPtr->ptr_2;
				mSharedResultPtr->int_01 = cl_set_kernel_arg_(
						(cl_kernel)mSharedOperationPtr->ptr_1,
						mSharedOperationPtr->uint_01,
						(size_t)mSharedOperationPtr->uint_02,
						&data);
				break;
			}
			case SETARG_DATA_TYPE::SETARG_UNSIGNED:
			case SETARG_ARRAY_BUFFER:
			{
				mSharedResultPtr->int_01 = cl_set_kernel_arg_(
						(cl_kernel)mSharedOperationPtr->ptr_1,
						mSharedOperationPtr->uint_01,
						(size_t)mSharedOperationPtr->uint_02,
						mSharedDataPtr);
				break;
			}
		}
	}
	CLLOG(INFO) << ">>result_SHM= "<< mSharedResultPtr->int_01;
}

cl_int CLApi::doClGetProgramInfo(
		cl_program program,
		cl_program_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "WEBCL::CLApi::doClGetProgramInfo, program=" << program << ", param_name=" << param_name;
	cl_int err = cl_get_program_info_(program, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>result=" << param_value;
	return err;
}

void CLApi::WebCLCallbackPtr(cl_event event, cl_int cmd_sts, void* userData)
{
#if defined(OS_ANDROID)
	gpu::GpuChannel* parent_channel_ = reinterpret_cast<gpu::GpuChannel*>(((cl_point*)userData)[4]);
	CLLOG(INFO) << "CLApi::WebCLCallbackPtr, parentGpuChannel : " << parent_channel_;
#endif

	if(parent_channel_ != nullptr) {
#if defined(OS_ANDROID)
		unsigned* keyArr = (unsigned*) userData;
#elif defined(OS_LINUX)
		cl_point* keyArr = (cl_point*) userData;
#endif
		CLLOG(INFO) << "routeId=" << keyArr[2] << ", event key=" << keyArr[0] << ", callback key=" << keyArr[1] << ", object_type=" << keyArr[3];

		parent_channel_->Send(new OpenCLChannelMsg_Callback((int)keyArr[2], keyArr[0], keyArr[1], keyArr[3]));
	}
	else
		CLLOG(INFO) << "can't find GpuChannel ptr.";
}

void CLApi::WebCLCallbackPtrProgram(cl_program program, void* userData)
{
	if(!program)
		return;

#if defined(OS_ANDROID)
	gpu::GpuChannel* parent_channel_ = (gpu::GpuChannel*)((cl_point*)userData)[4];
	CLLOG(INFO) << "CLApi::WebCLCallbackPtrProgram, parent_channel_ : " << parent_channel_;
#endif

	if(parent_channel_ != nullptr) {
#if defined(OS_ANDROID)
		unsigned* keyArr = (unsigned*) userData;
#elif defined(OS_LINUX)
		cl_point* keyArr = (cl_point*) userData;
#endif

		CLLOG(INFO) << "WebCLCallbackPtrProgram routeId=" << keyArr[2] << ", event key=" << keyArr[0] << ", callback key=" << keyArr[1] << ", object_type=" << keyArr[3];
		parent_channel_->Send(new OpenCLChannelMsg_Callback(keyArr[2], keyArr[0], keyArr[1], keyArr[3]));
	}
	else
		CLLOG(INFO) << "can't find GpuChannel ptr.";
}

cl_program CLApi::doClCreateProgramWithSource(
		cl_context context,
		cl_uint count,
		const char** strings,
		const size_t* lengths,
		cl_int* errcode_ret)
{
	CLLOG(INFO) << "WEBCL::CLApi::doClCreateProgramWithSource, context=" << context << ", count=" << count;
	cl_program program_id = cl_create_program_with_source_(context, count, strings, lengths, errcode_ret);
	CLLOG(INFO) << ">>cl_program=" << program_id;
	return program_id;
}

cl_int CLApi::doClGetProgramBuildInfo(
		cl_program program,
		cl_device_id device,
		cl_program_build_info param_name,
		size_t param_value_size,
		void* param_value,
		size_t* param_value_size_ret)
{
	CLLOG(INFO) << "WEBCL::CLApi::doClGetProgramBuildInfo, program=" << program << ", param_name=" << param_name;
	cl_int err = cl_get_program_build_info_(program, device, param_name, param_value_size, param_value, param_value_size_ret);
	CLLOG(INFO) << ">>err=" << err;
	CLLOG(INFO) << ">>result=" << param_value;
	return err;
}

cl_int CLApi::doClBuildProgram(
				cl_program program,
				cl_uint num_devices,
				const cl_device_id* device_list,
				const char* options,
				CL_CallbackclBuildProgram pfn_notify,
				void* user_data)
{
	CLLOG(INFO) << "WebCL::CLApi::doClBuildProgram, program=" << program << ", num_devices=" << num_devices << ", options=" << options;
	cl_int err = 0;
	if (((cl_point*)user_data)[2] == 0) {
		CLLOG(INFO) << "handler id null";
		err = cl_build_program_(program, num_devices, device_list, options, nullptr, nullptr);
	}
	else {
		CLLOG(INFO) << "handler id not null";
		CLLOG(INFO) << "nDebug, CLApi::doClBuildProgram, CLApi : " << this << ", gpu_channel_ : " << gpu_channel_;
#if defined(OS_ANDROID)
		((unsigned*)user_data)[4] = (cl_point)gpu_channel_;
#endif
		err = cl_build_program_(program, num_devices, device_list, options, &this->WebCLCallbackPtrProgram, user_data);
	}
	CLLOG(INFO) << ">>err=" << err;
	return err;
}

cl_int CLApi::doClEnqueueMarker(
		cl_command_queue command_queue,
		cl_event* event)
{
	CLLOG(INFO) << "doClEnqueueMarker, command_queue=" << command_queue;
	cl_int err = cl_enqueue_marker_(command_queue, event);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}
cl_int CLApi::doClEnqueueBarrier(
		cl_command_queue command_queue)
{
	CLLOG(INFO) << "doClEnqueueBarrier, command_queue=" << command_queue;
	cl_int err = cl_enqueue_barrier_(command_queue);
	CLLOG(INFO) << ">>err=" << err;
	return err;
}
cl_int CLApi::doClEnqueueWaitForEvents(
		cl_command_queue command_queue,
		cl_uint num_events,
		const cl_event* event_list)
{
	CLLOG(INFO) << "doClEnqueueWaitForEvents, command_queue=" << command_queue;
	cl_int err = cl_enqueue_wait_for_events_(command_queue, num_events, event_list);
	CLLOG(INFO) << ">>err=" << err;
	return err;

}

cl_kernel CLApi::doClCreateKernel(
				cl_program program,
				const char* kernel_name,
				cl_int* errcode_ret)
{
	CLLOG(INFO) << "WebCL::CLApi::doClCreateKernel, program=" << program << ", kernel_name=" << kernel_name;
	cl_kernel kernel = cl_create_kernel_(program, kernel_name, errcode_ret);
	CLLOG(INFO) << ">>kernel_id=" << kernel;
	CLLOG(INFO) << ">>errcode_ret=" << *errcode_ret;
	return kernel;
}

cl_int CLApi::doClCreateKernelsInProgram(
				cl_program program,
				cl_uint num_kernels,
				cl_kernel* kernels,
				cl_uint* num_kernels_ret)
{
	CLLOG(INFO) << "WebCL::CLApi::doClCreateKernelsInProgram, program=" << program << ", num_kernels=" << num_kernels;
	cl_int err = cl_create_kernels_in_program_(program, num_kernels, kernels, num_kernels_ret);
	if(num_kernels > 0) {
		for(size_t i=0; i<num_kernels; i++)
			CLLOG(INFO) << ">>" << kernels[i];
	}

	CLLOG(INFO) << ">>err=" << err;

	return err;
}

// gl/cl sharing
void CLApi::doClCreateBufferFromGLBuffer()
{
	CLLOG(INFO) << "doClCreateBufferFromGLBuffer, cl_context : " << (cl_context)mSharedOperationPtr->ptr_1;
	CLLOG(INFO) << "doClCreateBufferFromGLBuffer, cl_mem_flags : " << mSharedOperationPtr->ulong_01;
	CLLOG(INFO) << "doClCreateBufferFromGLBuffer, GLuint bufobj : " << mSharedOperationPtr->uint_01;

	GLuint bufobj = mSharedOperationPtr->uint_01;
	GLuint serviceId = 0;

	gpu::GpuChannelManager* gpuChannelManager = gpu_channel_->gpu_channel_manager();
	if(gpuChannelManager) {
		serviceId = gpuChannelManager->LookupGLServiceId(bufobj, GLResourceType::BUFFER);
	}

	CLLOG(INFO) << "doClCreateBufferFromGLBuffer, findServiceId, serviceId : " << serviceId;

	cl_int errcode_ret = CL_INVALID_VALUE;

	mSharedResultPtr->ptr_1 = (cl_point)cl_create_from_gl_buffer_(
			(cl_context)mSharedOperationPtr->ptr_1,
			mSharedOperationPtr->ulong_01,
			serviceId,
			&errcode_ret);

	mSharedResultPtr->int_01 = errcode_ret;
	mSharedResultPtr->uint_01 = serviceId;

	CLLOG(INFO) << "doClCreateBufferFromGLBuffer, errcode_ret : " << errcode_ret;
}

void CLApi::doClCreateImageFromGLRenderbuffer()
{
	CLLOG(INFO) << "doClCreateImageFromGLRenderbuffer, cl_context context : " << (cl_context)mSharedOperationPtr->ptr_1;
	CLLOG(INFO) << "doClCreateImageFromGLRenderbuffer, cl_mem_flags flags : " << mSharedOperationPtr->ulong_01;
	CLLOG(INFO) << "doClCreateImageFromGLRenderbuffer, GLuint renderbuffer : " << mSharedOperationPtr->uint_01;

	GLuint renderbuffer = mSharedOperationPtr->uint_01;
	GLuint serviceId = 0;

	gpu::GpuChannelManager* gpuChannelManager = gpu_channel_->gpu_channel_manager();
	if(gpuChannelManager) {
		for(; renderbuffer > 0; renderbuffer--) {
			serviceId = gpuChannelManager->LookupGLServiceId(renderbuffer, GLResourceType::RENDERBUFFER);

			if(serviceId != 0) {
				break;
			}
		}
	}

	CLLOG(INFO) << "doClCreateImageFromGLRenderbuffer, findServiceId, serviceId : " << serviceId;

	cl_int errcode_ret = CL_INVALID_VALUE;

	mSharedResultPtr->ptr_1 = (cl_point)cl_create_from_gl_render_buffer_(
			(cl_context)mSharedOperationPtr->ptr_1,
			mSharedOperationPtr->ulong_01,
			serviceId,
			&errcode_ret);

	mSharedResultPtr->int_01 = errcode_ret;

	CLLOG(INFO) << "doClCreateImageFromGLRenderbuffer, errcode_ret : " << errcode_ret;
}

void CLApi::doClCreateImageFromGLTexture()
{
	CLLOG(INFO) << "doClCreateImageFromGLTexture, cl_context context : " << (cl_context)mSharedOperationPtr->ptr_1;
	CLLOG(INFO) << "doClCreateImageFromGLTexture, cl_mem_flags flags : " << mSharedOperationPtr->ulong_01;
	CLLOG(INFO) << "doClCreateImageFromGLTexture, GLenum texture_target : " << mSharedOperationPtr->ulong_02;
	CLLOG(INFO) << "doClCreateImageFromGLTexture, GLint miplevel : " << mSharedOperationPtr->int_01;
	CLLOG(INFO) << "doClCreateImageFromGLTexture, GLuint texture : " << mSharedOperationPtr->uint_01;

	GLuint texture = mSharedOperationPtr->uint_01;
	GLuint serviceId = 0;

	gpu::GpuChannelManager* gpuChannelManager = gpu_channel_->gpu_channel_manager();
	if(gpuChannelManager) {
		serviceId = gpuChannelManager->LookupGLServiceId(texture, GLResourceType::TEXTURE);
	}

	CLLOG(INFO) << "doClCreateImageFromGLTexture, findServiceId, serviceId : " << serviceId;

	cl_int errcode_ret = CL_INVALID_VALUE;

	mSharedResultPtr->ptr_1 = (cl_point)cl_create_from_gl_texture_2d_(
			(cl_context)mSharedOperationPtr->ptr_1,
			mSharedOperationPtr->ulong_01,
			mSharedOperationPtr->ulong_02,
			mSharedOperationPtr->int_01,
			serviceId,
			&errcode_ret);

	mSharedResultPtr->int_01 = errcode_ret;

	CLLOG(INFO) << "doClCreateImageFromGLTexture, errcode_ret : " << errcode_ret;
}

void CLApi::doGetGLObjectInfo()
{
	if (cl_get_gl_object_info_ == nullptr) {
		CLLOG(INFO) << "cl_get_gl_object_info_ is nullptr";
		mSharedResultPtr->int_01 = CL_NOT_SUPPORTED_FUNCTION;
		return;
	}
	CLLOG(INFO) << "doGetGLObjectInfo, clmem : " << (cl_mem)mSharedOperationPtr->ptr_1;

	unsigned* gl_object_type = nullptr;
	unsigned* gl_object_name = nullptr;
	if(mSharedOperationPtr->bool_01) {
		gl_object_type = &mSharedResultPtr->uint_01;
	}
	if (mSharedOperationPtr->bool_02) {
		gl_object_name = &mSharedResultPtr->uint_02;
	}

	mSharedResultPtr->int_01 = CL_INVALID_VALUE;

	mSharedResultPtr->int_01 = cl_get_gl_object_info_((cl_mem)mSharedOperationPtr->ptr_1,
									gl_object_type,
									gl_object_name);
	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;

	mSharedResultPtr->uint_03 = 0;
	mSharedResultPtr->uint_04 = 0;
	if (*gl_object_type == CL_GL_OBJECT_TEXTURE2D || *gl_object_type == CL_GL_OBJECT_TEXTURE3D) {
		if (cl_get_gl_texture_info_ == nullptr) {
			CLLOG(INFO) << "cl_get_gl_texture_info_ is nullptr";
			mSharedResultPtr->int_01 = CL_NOT_SUPPORTED_FUNCTION;
		}
		GLenum targetValue = 0;
		GLint mipmapLevel = 0;
		cl_int err = cl_get_gl_texture_info_((cl_mem)mSharedOperationPtr->ptr_1, CL_GL_TEXTURE_TARGET,
									sizeof(GLenum), &targetValue, NULL);
		if (err != CL_SUCCESS) {
			mSharedResultPtr->int_01 = err;
			return;
		}
		err = cl_get_gl_texture_info_((cl_mem)mSharedOperationPtr->ptr_1, CL_GL_MIPMAP_LEVEL,
								sizeof(GLint), &mipmapLevel, NULL);
		if (err != CL_SUCCESS) {
			mSharedResultPtr->int_01 = err;
			return;
		}
		mSharedResultPtr->uint_03 = targetValue;
		mSharedResultPtr->uint_04 = mipmapLevel;
	}
}

void CLApi::doEnqueueAcquireGLObjects()
{
	if (cl_enqueue_acquire_globjects_ == nullptr) {
		CLLOG(INFO) << "cl_enqueue_acquire_globjects_ is nullptr";
		mSharedResultPtr->int_01 = CL_NOT_SUPPORTED_FUNCTION;
		return;
	}
	CLLOG(INFO) << "doEnqueueAcquireGLObjects, commandqueue : " << mSharedOperationPtr->ptr_1;

	mSharedResultPtr->int_01 = CL_INVALID_VALUE;

	cl_event* event_wait_list_inter = nullptr;
	cl_event event_inter;
	if(mSharedOperationPtr->uint_02) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_02];
		for(size_t i=0; i<mSharedOperationPtr->uint_02; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	mSharedResultPtr->int_01 = cl_enqueue_acquire_globjects_((cl_command_queue)mSharedOperationPtr->ptr_1,
									mSharedOperationPtr->uint_01,
									(cl_mem*)handleBlockedCallData(HOST_PTR_DATA_TYPE::CL_MEM, mSharedOperationPtr->uint_01),
									mSharedOperationPtr->uint_02,
									event_wait_list_inter,
									mSharedOperationPtr->bool_01?&event_inter:nullptr);

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)
}

void CLApi::doEnqueueReleaseGLObjects()
{
	if (cl_enqueue_release_globjects_ == nullptr) {
		CLLOG(INFO) << "cl_enqueue_release_globjects_ is nullptr";
		mSharedResultPtr->int_01 = CL_NOT_SUPPORTED_FUNCTION;
		return;
	}
	CLLOG(INFO) << "doEnqueueReleaseGLObjects, commandqueue : " << mSharedOperationPtr->ptr_1;

	mSharedResultPtr->int_01 = CL_INVALID_VALUE;

	cl_event* event_wait_list_inter = nullptr;
	cl_event event_inter;
	if(mSharedOperationPtr->uint_02) {
		event_wait_list_inter = new cl_event[mSharedOperationPtr->uint_02];
		for(size_t i=0; i<mSharedOperationPtr->uint_02; i++)
			event_wait_list_inter[i] = (cl_event)mSharedEventsPtr[i];
	}

	mSharedResultPtr->int_01 = cl_enqueue_release_globjects_((cl_command_queue)mSharedOperationPtr->ptr_1,
									mSharedOperationPtr->uint_01,
									(cl_mem*)mSharedDataPtr,
									mSharedOperationPtr->uint_02,
									event_wait_list_inter,
									mSharedOperationPtr->bool_01?&event_inter:nullptr);

	CLLOG(INFO) << ">>err=" << mSharedResultPtr->int_01;

	if(mSharedOperationPtr->bool_01) {
		CLLOG(INFO) << ">>event=" << event_inter;
		mSharedResultPtr->ptr_1 = (cl_point)event_inter;
	}
	else {
		CLLOG(INFO) << ">>event=none";
		mSharedResultPtr->ptr_1 = 0;
	}

	DELETE_ARRAY(event_wait_list_inter)
}

void CLApi::addCommandQueueGarbage(int data_type, cl_point ptr)
{
	switch(data_type) {
	case HOST_PTR_DATA_TYPE::UNSIGNED_CHAR:
		garbage_ptr_list_unsigned_char.push_back(ptr);
		break;
	case HOST_PTR_DATA_TYPE::UNSIGNED:
		garbage_ptr_list_unsigned.push_back(ptr);
		break;
	case HOST_PTR_DATA_TYPE::INT:
		garbage_ptr_list_int.push_back(ptr);
		break;
	case HOST_PTR_DATA_TYPE::FLOAT:
		garbage_ptr_list_float.push_back(ptr);
		break;
	case HOST_PTR_DATA_TYPE::DOUBLE:
		garbage_ptr_list_double.push_back(ptr);
		break;
	case HOST_PTR_DATA_TYPE::CL_MEM:
		garbage_ptr_list_cl_mem.push_back(ptr);
		break;
	}
}

void* CLApi::handleBlockedCallData(int data_type, size_t data_size)
{
	switch(data_type) {
	case HOST_PTR_DATA_TYPE::UNSIGNED_CHAR: {
		unsigned char* data = new unsigned char[data_size];
		memcpy(data, mSharedDataPtr, data_size*sizeof(unsigned char));
		addCommandQueueGarbage(data_type, (cl_point)data);
		return data;
	}
	case HOST_PTR_DATA_TYPE::UNSIGNED: {
		unsigned* data = new unsigned[data_size];
		memcpy(data, mSharedDataPtr, data_size*sizeof(unsigned));
		addCommandQueueGarbage(data_type, (cl_point)data);
		return data;
	}
		break;
	case HOST_PTR_DATA_TYPE::INT: {
		int* data = new int[data_size];
		memcpy(data, mSharedDataPtr, data_size*sizeof(int));
		addCommandQueueGarbage(data_type, (cl_point)data);
		return data;
	}
	case HOST_PTR_DATA_TYPE::FLOAT: {
		float* data = new float[data_size];
		memcpy(data, mSharedDataPtr, data_size*sizeof(float));
		addCommandQueueGarbage(data_type, (cl_point)data);
		return data;
	}
	case HOST_PTR_DATA_TYPE::DOUBLE: {
		double* data = new double[data_size];
		memcpy(data, mSharedDataPtr, data_size*sizeof(double));
		addCommandQueueGarbage(data_type, (cl_point)data);
		return data;
	}
	case HOST_PTR_DATA_TYPE::CL_MEM: {
		cl_mem* data = new cl_mem[data_size];
		memcpy(data, mSharedDataPtr, data_size*sizeof(cl_mem));
		addCommandQueueGarbage(data_type, (cl_point)data);
		return data;
	}
	default:
		return nullptr;
	}
}

bool CLApi::setSharedMemory(base::SharedMemoryHandle dataHandle, base::SharedMemoryHandle operationHandle, base::SharedMemoryHandle resultHandle, base::SharedMemoryHandle eventsHandle)
{
	CLLOG(INFO) << "setSharedMemory";

	bool result = true;

	mSharedData.reset(new base::SharedMemory(dataHandle, false));
	result &= mSharedData->Map(1024*1024*20);
	mSharedDataPtr = mSharedData->memory();
	result &= mSharedDataPtr?true:false;

	mSharedOperation.reset(new base::SharedMemory(operationHandle, false));
	result &= mSharedOperation->Map(sizeof(BaseOperationData));
	mSharedOperationPtr = static_cast<BaseOperationData*>(mSharedOperation->memory());
	result &= mSharedOperationPtr?true:false;

	mSharedResult.reset(new base::SharedMemory(resultHandle, false));
	result &= mSharedResult->Map(sizeof(BaseResultData));
	mSharedResultPtr = static_cast<BaseResultData*>(mSharedResult->memory());
	result &= mSharedResultPtr?true:false;

	mSharedEvents.reset(new base::SharedMemory(eventsHandle, false));
	result &= mSharedEvents->Map(sizeof(cl_point)*32);
	mSharedEventsPtr = static_cast<cl_point*>(mSharedEvents->memory());
	result &= mSharedResultPtr?true:false;

	CLLOG(INFO) << ">> result=" << result;

	return result;
}

bool CLApi::clearSharedMemory()
{
	CLLOG(INFO) << "cleartSharedMemory";

	bool result = true;

	result &= mSharedData->Unmap();
	result &= mSharedOperation->Unmap();
	result &= mSharedResult->Unmap();
	result &= mSharedEvents->Unmap();

	mSharedData->Close();
	mSharedOperation->Close();
	mSharedOperation->Close();
	mSharedEvents->Close();

	mSharedData.reset(nullptr);
	mSharedOperation.reset(nullptr);
	mSharedResult.reset(nullptr);
	mSharedResult.reset(nullptr);

	CLLOG(INFO) << ">> result=" << result;

	return result;
}

} // namesapce gfx
