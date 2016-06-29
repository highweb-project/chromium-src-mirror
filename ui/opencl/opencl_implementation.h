/*
 * opencl_test.h
 *
 *  Created on: 2015. 6. 5.
 *      Author: jphofasb
 */

#ifndef UI_OPENCL_OPENCL_IMPLEMENTATION_H_
#define UI_OPENCL_OPENCL_IMPLEMENTATION_H_

#include "base/native_library.h"
#include "base/memory/ref_counted.h"
#include "base/memory/shared_memory.h"

#include "ui/opencl/opencl_include.h"

namespace gpu {
class GpuChannel;
}

namespace gfx {

	//define OpenCL function pointer type
	CL_API_ARGS3(CLGetPlatformIDs, cl_int, cl_uint, ARG_PTR(cl_platform_id), ARG_PTR(cl_uint))
	CL_API_ARGS5(CLGetInfo, cl_int, cl_platform_id, cl_platform_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS5(CLGetDeviceIDs, cl_int, cl_platform_id, cl_device_type, cl_uint, ARG_PTR(cl_device_id), ARG_PTR(cl_uint))
	CL_API_ARGS5(CLGetDeviceInfo, cl_int, cl_device_id, cl_device_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS6(CLCreateContext, cl_context, ARG_PTR(cl_context_properties), cl_uint, ARG_PTR(cl_device_id), CL_CallbackclCreateContextFromType, ARG_PTR(void), ARG_PTR(cl_int))
	CL_API_ARGS5(CLCreateContextFromType, cl_context, ARG_PTR(cl_context_properties), cl_device_type, CL_CallbackclCreateContextFromType, ARG_PTR(void), ARG_PTR(cl_int))
	CL_API_ARGS2(CLWaitForEvents, cl_int, cl_uint, ARG_CONST(ARG_PTR(cl_event)))
	CL_API_ARGS5(CLCreateBuffer, cl_mem, cl_context, cl_mem_flags, size_t, ARG_PTR(void), ARG_PTR(cl_int))
	CL_API_ARGS5(CLGetMemObjectInfo, cl_int, cl_mem, cl_mem_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS5(CLCreateSubBuffer, cl_mem, cl_mem, cl_mem_flags, cl_buffer_create_type, ARG_CONST(ARG_PTR(void)), ARG_PTR(cl_int))
	CL_API_ARGS5(CLCreateSampler, cl_sampler, cl_context, cl_bool, cl_addressing_mode, cl_filter_mode, ARG_PTR(cl_int))
	CL_API_ARGS5(CLGetSamplerInfo, cl_int, cl_sampler, cl_sampler_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS1(CLReleaseSampler, cl_int, cl_sampler)
	CL_API_ARGS5(CLGetImageInfo, cl_int, cl_mem, cl_image_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS5(CLGetEventInfo, cl_int, cl_event, cl_event_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS5(CLGetEventProfilingInfo, cl_int, cl_event, cl_profiling_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS4(CLSetEventCallback, cl_int, cl_event, cl_int, CL_CallbackclSetEvent, ARG_PTR(void))
	CL_API_ARGS1(CLReleaseEvent, cl_int, cl_event)
	CL_API_ARGS5(CLGetContextInfo, cl_int, cl_context, cl_context_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS8(CLCreateImage2D, cl_mem, cl_context, cl_mem_flags, ARG_CONST(ARG_PTR(cl_image_format)), size_t, size_t, size_t, ARG_PTR(void), ARG_PTR(cl_int))
	CL_API_ARGS2(CLSetUserEventStatus, cl_int, cl_event, cl_int)
	CL_API_ARGS2(CLCreateUserEvent, cl_event, cl_context, ARG_PTR(cl_int))
	CL_API_ARGS4(CLCreateCommandQueue, cl_command_queue, cl_context, cl_device_id, cl_command_queue_properties, ARG_PTR(cl_int))
	CL_API_ARGS6(CLGetSupportedImageFormat, cl_int, cl_context, cl_mem_flags, cl_mem_object_type, cl_uint, ARG_PTR(cl_image_format), ARG_PTR(cl_uint))
	CL_API_ARGS5(CLGetCommandQueueInfo, cl_int, cl_command_queue, cl_command_queue_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS9(CLEnqueueCopyBuffer, cl_int, cl_command_queue, cl_mem, cl_mem, size_t, size_t, size_t, cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS13(CLEnqueueCopyBufferRect, cl_int, cl_command_queue, cl_mem, cl_mem, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), size_t, size_t, size_t, size_t, cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS9(CLEnqueueCopyImage, cl_int, cl_command_queue, cl_mem, cl_mem, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS9(CLEnqueueCopyBufferToImage, cl_int, cl_command_queue, cl_mem, cl_mem, size_t, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS9(CLEnqueueCopyImageToBuffer, cl_int, cl_command_queue, cl_mem, cl_mem, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), size_t, cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS9(CLEnqueueReadBuffer, cl_int, cl_command_queue, cl_mem, cl_bool, size_t, size_t, ARG_PTR(void), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS14(CLEnqueueReadBufferRect, cl_int, cl_command_queue, cl_mem, cl_bool, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), size_t, size_t, size_t, size_t, ARG_PTR(void), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS11(CLEnqueueReadImage, cl_int, cl_command_queue, cl_mem, cl_bool, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), size_t, size_t, ARG_PTR(void), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS9(CLEnqueueWriteBuffer, cl_int, cl_command_queue, cl_mem, cl_bool, size_t, size_t, ARG_CONST(ARG_PTR(void)), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS14(CLEnqueueWriteBufferRect, cl_int, cl_command_queue, cl_mem, cl_bool, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), size_t, size_t, size_t, size_t, ARG_PTR(void), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS11(CLEnqueueWriteImage, cl_int, cl_command_queue, cl_mem, cl_bool, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), size_t, size_t, ARG_CONST(void*), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS9(CLEnqueueNDRangeKernel, cl_int, cl_command_queue, cl_kernel, cl_uint, ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), ARG_CONST(ARG_PTR(size_t)), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS1(CLFinish, cl_int, cl_command_queue)
	CL_API_ARGS1(CLFlush, cl_int, cl_command_queue)
	CL_API_ARGS1(CLReleaseContext, cl_int, cl_context)
	CL_API_ARGS1(CLReleaseProgram, cl_int, cl_program)
	CL_API_ARGS1(CLReleaseKernel, cl_int, cl_kernel)
	CL_API_ARGS1(CLReleaseMemObject, cl_int, cl_mem)
	CL_API_ARGS1(CLReleaseCommandQueue, cl_int, cl_command_queue)
	CL_API_ARGS5(CLGetKernelInfo, cl_int, cl_kernel, cl_kernel_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS6(CLGetKernelWorkGroupInfo, cl_int, cl_kernel, cl_device_id, cl_kernel_work_group_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS6(CLGetKernelArgInfo, cl_int, cl_kernel, cl_uint, cl_kernel_arg_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS4(CLSetKernelArg, cl_int, cl_kernel, cl_uint, size_t, ARG_CONST(ARG_PTR(void)))
	CL_API_ARGS5(CLGetProgramInfo, cl_int, cl_program, cl_program_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS5(CLCreateProgramWithSource, cl_program, cl_context, cl_uint, ARG_CONST(ARG_PTR(ARG_PTR(char))), ARG_CONST(ARG_PTR(size_t)), ARG_PTR(cl_int))
	CL_API_ARGS6(CLGetProgramBuildInfo, cl_int, cl_program, cl_device_id, cl_program_build_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS6(CLBuildProgram, cl_int, cl_program, cl_uint, ARG_CONST(ARG_PTR(cl_device_id)), ARG_CONST(ARG_PTR(char)), CL_CallbackclBuildProgram, ARG_PTR(void))
	CL_API_ARGS2(CLEnqueueMarker, cl_int, cl_command_queue, ARG_PTR(cl_event))
	CL_API_ARGS1(CLEnqueueBarrier, cl_int, cl_command_queue)
	CL_API_ARGS3(CLEnqueueWaitForEvents, cl_int, cl_command_queue, cl_uint, ARG_CONST(ARG_PTR(cl_event)))
	CL_API_ARGS3(CLCreateKernel, cl_kernel, cl_program, ARG_CONST(ARG_PTR(char)), ARG_PTR(cl_int))
	CL_API_ARGS4(CLCreateKernelsInProgram, cl_int, cl_program, cl_uint, ARG_PTR(cl_kernel), ARG_PTR(cl_uint))
	// gl/cl sharing
	CL_API_ARGS4(CLCreateFromGLBuffer, cl_mem, cl_context, cl_mem_flags, GLuint, ARG_PTR(cl_int))
	CL_API_ARGS4(CLCreateFromGLRenderbuffer, cl_mem, cl_context, cl_mem_flags, GLuint, ARG_PTR(cl_int))
	CL_API_ARGS6(CLCreateFromGLTexture2D, cl_mem, cl_context, cl_mem_flags, GLuint, GLint, GLuint, ARG_PTR(cl_int))
	CL_API_ARGS3(CLGetGLObjectInfo, cl_int, cl_mem, ARG_PTR(cl_gl_object_type), ARG_PTR(GLuint))
	CL_API_ARGS5(CLGetGLTextureInfo, cl_int, cl_mem, cl_gl_texture_info, size_t, ARG_PTR(void), ARG_PTR(size_t))
	CL_API_ARGS6(CLEnqueueAcquireGLObjects, cl_int, cl_command_queue, cl_uint, ARG_CONST(ARG_PTR(cl_mem)), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))
	CL_API_ARGS6(CLEnqueueReleaseGLObjects, cl_int, cl_command_queue, cl_uint, ARG_CONST(ARG_PTR(cl_mem)), cl_uint, ARG_CONST(ARG_PTR(cl_event)), ARG_PTR(cl_event))

	CL_API_ARGS3(VKCreateInstance, VkResult, ARG_CONST(ARG_PTR(VkInstanceCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkInstance));
	CL_API_ARGS3(VKEnumeratePhysicalDevices, VkResult, VkInstance, ARG_PTR(uint32_t), ARG_PTR(VkPhysicalDevice));
	CL_API_ARGS2(VKGetPhysicalDeviceProperties, ARG_PTR(void), VkPhysicalDevice, ARG_PTR(VkPhysicalDeviceProperties));
	CL_API_ARGS3(VKGetPhysicalDeviceQueueFamilyProperties, ARG_PTR(void), VkPhysicalDevice, ARG_PTR(uint32_t), ARG_PTR(VkQueueFamilyProperties));
	CL_API_ARGS4(VKCreateDevice, VkResult, VkPhysicalDevice, ARG_CONST(ARG_PTR(VkDeviceCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkDevice));
	CL_API_ARGS4(VKGetDeviceQueue, ARG_PTR(void), VkDevice, uint32_t, uint32_t, ARG_PTR(VkQueue));
	CL_API_ARGS2(VKGetPhysicalDeviceMemoryProperties, ARG_PTR(void), VkPhysicalDevice, ARG_PTR(VkPhysicalDeviceMemoryProperties));
	CL_API_ARGS4(VKCreateCommandPool, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkCommandPoolCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkCommandPool));
	CL_API_ARGS3(VKAllocateCommandBuffers, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkCommandBufferAllocateInfo)), ARG_PTR(VkCommandBuffer));
	CL_API_ARGS4(VKCreateBuffer, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkBufferCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkBuffer));
	CL_API_ARGS3(VKGetBufferMemoryRequirements, ARG_PTR(void), VkDevice, VkBuffer, ARG_PTR(VkMemoryRequirements));
	CL_API_ARGS4(VKAllocateMemory, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkMemoryAllocateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkDeviceMemory));
	CL_API_ARGS4(VKBindBufferMemory, VkResult, VkDevice, VkBuffer, VkDeviceMemory, VkDeviceSize);
	CL_API_ARGS6(VKMapMemory, VkResult, VkDevice, VkDeviceMemory, VkDeviceSize, VkDeviceSize, VkMemoryMapFlags, ARG_PTR(ARG_PTR(void)));
	CL_API_ARGS2(VKUnmapMemory, ARG_PTR(void), VkDevice, VkDeviceMemory);
	CL_API_ARGS4(VKCreateShaderModule, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkShaderModuleCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkShaderModule));
	CL_API_ARGS4(VKCreateDescriptorSetLayout, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkDescriptorSetLayoutCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkDescriptorSetLayout));
	CL_API_ARGS4(VKCreatePipelineLayout, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkPipelineLayoutCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkPipelineLayout));
	CL_API_ARGS4(VKCreatePipelineCache, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkPipelineCacheCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkPipelineCache));
	CL_API_ARGS6(VKCreateComputePipelines, VkResult, VkDevice, VkPipelineCache, uint32_t, ARG_CONST(ARG_PTR(VkComputePipelineCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkPipeline));
	CL_API_ARGS4(VKCreateDescriptorPool, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkDescriptorPoolCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkDescriptorPool));
	CL_API_ARGS3(VKAllocateDescriptorSets, VkResult, VkDevice, ARG_CONST(ARG_PTR(VkDescriptorSetAllocateInfo)), ARG_PTR(VkDescriptorSet));
	CL_API_ARGS5(VKUpdateDescriptorSets, ARG_PTR(void), VkDevice, uint32_t, ARG_CONST(ARG_PTR(VkWriteDescriptorSet)), uint32_t, ARG_CONST(ARG_PTR(VkCopyDescriptorSet)));
	CL_API_ARGS2(VKBeginCommandBuffer, VkResult, VkCommandBuffer, ARG_CONST(ARG_PTR(VkCommandBufferBeginInfo)));
	CL_API_ARGS8(VKCmdBindDescriptorSets, ARG_PTR(void), VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t, uint32_t, ARG_CONST(ARG_PTR(VkDescriptorSet)), uint32_t, ARG_CONST(ARG_PTR(uint32_t)));
	CL_API_ARGS3(VKCmdBindPipeline, ARG_PTR(void), VkCommandBuffer, VkPipelineBindPoint, VkPipeline);
	CL_API_ARGS4(VKCmdDispatch, ARG_PTR(void), VkCommandBuffer, uint32_t, uint32_t, uint32_t);
	CL_API_ARGS10(VKCmdPipelineBarrier, ARG_PTR(void), VkCommandBuffer, VkPipelineStageFlags, VkPipelineStageFlags, VkDependencyFlags, uint32_t, ARG_CONST(ARG_PTR(VkMemoryBarrier)), uint32_t, ARG_CONST(ARG_PTR(VkBufferMemoryBarrier)), uint32_t, ARG_CONST(ARG_PTR(VkImageMemoryBarrier)));
	CL_API_ARGS1(VKEndCommandBuffer, VkResult, VkCommandBuffer);
	CL_API_ARGS4(VKQueueSubmit, VkResult, VkQueue, uint32_t, ARG_CONST(ARG_PTR(VkSubmitInfo)), VkFence);
	CL_API_ARGS1(VKQueueWaitIdle, VkResult, VkQueue);
	CL_API_ARGS1(VKDeviceWaitIdle, VkResult, VkDevice);
	CL_API_ARGS5(VKCmdCopyBuffer, ARG_PTR(void), VkCommandBuffer, VkBuffer, VkBuffer, uint32_t, ARG_CONST(ARG_PTR(VkBufferCopy)));
	CL_API_ARGS3(VKFreeMemory, ARG_PTR(void), VkDevice, VkDeviceMemory, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	CL_API_ARGS3(VKDestroyBuffer, ARG_PTR(void), VkDevice, VkBuffer, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	CL_API_ARGS4(VKFreeDescriptorSets, VkResult, VkDevice, VkDescriptorPool, uint32_t, ARG_CONST(ARG_PTR(VkDescriptorSet)));
	CL_API_ARGS3(VKDestroyDescriptorPool, ARG_PTR(void), VkDevice, VkDescriptorPool, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	CL_API_ARGS4(VKFreeCommandBuffers, ARG_PTR(void), VkDevice, VkCommandPool, uint32_t, ARG_CONST(ARG_PTR(VkCommandBuffer)));
	CL_API_ARGS3(VKDestroyCommandPool, ARG_PTR(void), VkDevice, VkCommandPool, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	CL_API_ARGS2(VKDestroyDevice, ARG_PTR(void), VkDevice, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));

	class CLApi {
	public :
		CLApi();
		~CLApi();
		void InitApi(base::NativeLibrary nativeLib);
		bool doClTest();
		
#if defined(OS_LINUX)
		static gpu::GpuChannel* parent_channel_;
#elif defined(OS_ANDROID)
		void setChannel(gpu::GpuChannel* channel);
#endif

		void InitVulkanApi(base::NativeLibrary vulkanLib);

		VkDevice vkDevice;
		VkBufferCreateInfo vkBufferInfo[8];
		VkBuffer vkComputeBuffer[8];
		VkDeviceMemory vkDeviceMemory[8];
		VkCommandBuffer vkComputeCmdBuffer;
		VkQueue vkQueue = NULL;;
		int bufferCount = 8;
		int BUFFER_SIZE = 1024 * 4 * 4;

		void initNBody(const char* filePath);
		void doNBody();
		void doReadVulkanBuffer();
		void doWriteVulkanBuffer();
		unsigned getShaderCodeFromUrl(std::string address, char* sourceData, unsigned maxSourceSize);

		VkResult doVKCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
		VkResult doVKEnumeratePhysicalDevices(VkInstance pCreateInfo, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
		void doVKGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
		void doVKGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties);
		VkResult doVKCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
		void doVKGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
		void doVKGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties);
		VkResult doVKCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
		VkResult doVKAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);
		VkResult doVKCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);
		void doVKGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
		VkResult doVKAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
		VkResult doVKBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
		VkResult doVKMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
		void doVKUnmapMemory(VkDevice device, VkDeviceMemory memory);
		VkResult doVKCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
		VkResult doVKCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
		VkResult doVKCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
		VkResult doVKCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);
		VkResult doVKCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
		VkResult doVKCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);
		VkResult doVKAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets);
		void doVKUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies);
		VkResult doVKBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
		void doVKCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
		void doVKCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
		void doVKCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
		void doVKCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
		VkResult doVKEndCommandBuffer(VkCommandBuffer commandBuffer);
		VkResult doVKQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
		VkResult doVKQueueWaitIdle(VkQueue queue);
		VkResult doVKDeviceWaitIdle(VkDevice device);
		void doVKCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
		void doVKFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
		VkResult doVKFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets);
		void doVKDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator);
		void doVKFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
		void doVKDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);

		cl_int doclGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms);
		cl_int doClGetPlatformInfo(
				cl_platform_id platform,
				cl_platform_info param_name,
				size_t param_value_size,
				char* param_value,
				size_t* param_value_size_ret);
		cl_int doClGetDeviceIDs(
				cl_platform_id platform,
				cl_device_type device_type,
				cl_uint num_entries,
				cl_device_id* devices,
				cl_uint* num_devices);
		cl_int doClGetDeviceInfo(
				cl_device_id device,
				cl_device_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		cl_context doClCreateContext(
				cl_context_properties* properties,
				cl_uint num_devices,
				cl_device_id* devices,
				CL_CallbackclCreateContextFromType callback,
				void* user_data,
				cl_int* errcode_ret);
		cl_context doClCreateContextFromType(
				cl_context_properties* properties,
				cl_device_type device_type,
				CL_CallbackclCreateContextFromType callback,
				void* user_data,
				cl_int* errcode_ret);
		cl_int doClWaitForEvents(
				cl_uint num_events,
				const cl_event* event_list);
		void doClCreateBuffer();
		cl_int doClGetMemObjectInfo(
				cl_mem memobj,
				cl_mem_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		cl_mem doClCreateSubBuffer(
				cl_mem buffer,
				cl_mem_flags flags,
				cl_buffer_create_type buffer_create_type,
				const void* buffer_create_info,
				cl_int* errcode_ret);
		cl_sampler doClCreateSampler(
				cl_context context,
				cl_bool normalized_coords,
				cl_addressing_mode addressing_mode,
				cl_filter_mode filter_mode,
				cl_int* errcode_ret);
		cl_int doClGetSamplerInfo(
				cl_sampler sampler,
				cl_sampler_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		cl_int doClReleaseSampler(
				cl_sampler sampler);
		cl_int doClGetImageInfo(
				cl_mem image,
				cl_image_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		cl_int doClGetEventInfo (
				cl_event event,
				cl_event_info param_name,
				size_t param_value_size,
				void *param_value,
				size_t *param_value_size_ret);
		cl_int doClGetEventProfilingInfo (
				cl_event event,
				cl_profiling_info param_name,
				size_t param_value_size,
				void *param_value,
				size_t *param_value_size_ret);
		cl_int doClSetEventCallback (
				cl_event event,
				cl_int  command_exec_callback_type ,
				CL_CallbackclSetEvent callback,
				void *user_data);
		cl_int doClReleaseEvent (
				cl_event event);
		cl_int doClGetContextInfo(
				cl_context context,
				cl_context_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		void doClCreateImage2D();
		cl_int doClSetUserEventStatus(
				cl_event,
				cl_int);
		cl_event doClCreateUserEvent(
				cl_context,
				cl_int*);
		cl_command_queue doClCreateCommandQueue(
				cl_context,
				cl_device_id,
				cl_command_queue_properties,
				cl_int*);
		cl_int doClGetSupportedImageFormat(
				cl_context,
				cl_mem_flags,
				cl_mem_object_type,
				cl_uint,
				cl_image_format*,
				cl_uint*);
		cl_int doClGetCommandQueueInfo(
				cl_command_queue command_queue,
				cl_command_queue_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		void doClEnqueueCopyBuffer();
		void doClEnqueueCopyBufferRect();
		void doClEnqueueCopyImage();
		void doClEnqueueCopyBufferToImage();
		void doClEnqueueCopyImageToBuffer();
		void doClEnqueueReadBuffer();
		void doClEnqueueReadBufferRect();
		void doClEnqueueReadImage();
		void doClEnqueueWriteBuffer();
		void doClEnqueueWriteBufferRect();
		void doClEnqueueWriteImage();
		void doClEnqueueNDRangeKernel();
		void doClFinish();
		cl_int doClFlush(
				cl_command_queue command_queue);
		cl_int doClReleaseContext(
				cl_context context);
		cl_int doClReleaseProgram(
				cl_program program);
		cl_int doClReleaseKernel(
				cl_kernel kernel);
		cl_int doClReleaseMemObject(
				cl_mem memobj);
		cl_int doClReleaseCommandQueue(
				cl_command_queue command_queue);
		cl_int doClGetKernelInfo(
				cl_kernel kernel,
				cl_kernel_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		cl_int doClGetKernelWorkGroupInfo(
				cl_kernel kernel,
				cl_device_id device,
				cl_kernel_work_group_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		cl_int doClGetKernelArgInfo(
				cl_kernel kernel,
				cl_uint arg_indx,
				cl_kernel_arg_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);
		void doClSetKernelArg();
		cl_int doClEnqueueMarker(
				cl_command_queue command_queue,
				cl_event* event);
		cl_int doClEnqueueBarrier(
				cl_command_queue command_queue);
		cl_int doClEnqueueWaitForEvents(
				cl_command_queue command_queue,
				cl_uint num_events,
				const cl_event* event_list);

		cl_int doClGetProgramInfo(
				cl_program program,
				cl_program_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);

		cl_program doClCreateProgramWithSource(
				cl_context context,
				cl_uint count,
				const char** strings,
				const size_t* lengths,
				cl_int* errcode_ret);

		static void WebCLCallbackPtr(cl_event event, cl_int cmd_sts, void* userData);
		static void WebCLCallbackPtrProgram(cl_program program, void* userData);

		cl_int doClGetProgramBuildInfo(
				cl_program program,
				cl_device_id device,
				cl_program_build_info param_name,
				size_t param_value_size,
				void* param_value,
				size_t* param_value_size_ret);

		cl_int doClBuildProgram(
				cl_program program,
				cl_uint num_devices,
				const cl_device_id* device_list,
				const char* options,
				CL_CallbackclBuildProgram pfn_notify,
				void* user_data);

		cl_kernel doClCreateKernel(
				cl_program program,
				const char* kernel_name,
				cl_int* errcode_ret);

		cl_int doClCreateKernelsInProgram(
				cl_program program,
				cl_uint num_kernels,
				cl_kernel* kernels,
				cl_uint* num_kernels_ret);

		// gl/cl sharing
		void doClCreateBufferFromGLBuffer();
		void doClCreateImageFromGLRenderbuffer();
		void doClCreateImageFromGLTexture();
		void doGetGLObjectInfo();
		void doEnqueueAcquireGLObjects();
		void doEnqueueReleaseGLObjects();

		void addCommandQueueGarbage(int data_type, cl_point ptr);

		void* handleBlockedCallData(int data_type, size_t data_size);

		bool setSharedMemory(base::SharedMemoryHandle dataHandle, base::SharedMemoryHandle operationHandle, base::SharedMemoryHandle resultHandle, base::SharedMemoryHandle eventsHandle);
		bool clearSharedMemory();

	private:
		gpu::GpuChannel* gpu_channel_;

		bool clLibraryLoaded_ = false;

		base::NativeLibrary opencl_library_ = 0;
		base::NativeLibrary vulkan_library_ = 0;

		VKCreateInstance vk_create_instance_ = 0;
		VKEnumeratePhysicalDevices vk_enumerate_physical_device_ = 0;
		VKGetPhysicalDeviceProperties vk_get_physical_device_properties_ = 0;
		VKGetPhysicalDeviceQueueFamilyProperties vk_get_physical_device_queue_family_properties_ = 0;
		VKCreateDevice vk_create_device_ = 0;
		VKGetDeviceQueue vk_get_device_queue_ = 0;
		VKGetPhysicalDeviceMemoryProperties vk_get_physical_device_memory_properties_ = 0;
		VKCreateCommandPool vk_create_command_pool_ = 0;
		VKAllocateCommandBuffers vk_allocate_command_buffers_ = 0;
		VKCreateBuffer vk_create_buffer_ = 0;
		VKGetBufferMemoryRequirements vk_get_buffer_memory_requirements_ = 0;
		VKAllocateMemory vk_allocate_memory_ = 0;
		VKBindBufferMemory vk_bind_buffer_memory_ = 0;
		VKMapMemory vk_map_memory_ = 0;
		VKUnmapMemory vk_unmap_memory_ = 0;
		VKCreateShaderModule vk_create_shader_module_ = 0;
		VKCreateDescriptorSetLayout vk_create_descriptor_set_layout_ = 0;
		VKCreatePipelineLayout vk_create_pipeline_layout_ = 0;
		VKCreatePipelineCache vk_create_pipeline_cache_ = 0;
		VKCreateComputePipelines vk_create_compute_pipelines_ = 0;
		VKCreateDescriptorPool vk_create_descriptor_pool_ = 0;
		VKAllocateDescriptorSets vk_allocate_descriptor_sets_ = 0;
		VKUpdateDescriptorSets vk_update_descriptor_sets_ = 0;
		VKBeginCommandBuffer vk_begin_command_buffer_ = 0;
		VKCmdBindDescriptorSets vk_cmd_bind_descriptor_sets_ = 0;
		VKCmdBindPipeline vk_cmd_bind_pipeline_ = 0;
		VKCmdDispatch vk_cmd_dispatch_ = 0;
		VKCmdPipelineBarrier vk_cmd_pipeline_barrier_ = 0;
		VKEndCommandBuffer vk_end_command_buffer_ = 0;
		VKQueueSubmit vk_queue_submit_ = 0;
		VKQueueWaitIdle vk_queue_wait_idle_ = 0;
		VKDeviceWaitIdle vk_device_wait_idle_ = 0;
		VKCmdCopyBuffer vk_cmd_copy_buffer_ = 0;
		VKFreeMemory vk_free_memory_ = 0;
		VKDestroyBuffer vk_destroy_buffer_ = 0;
		VKFreeDescriptorSets vk_free_descriptor_sets_ = 0;
		VKDestroyDescriptorPool vk_destroy_descriptor_pool_ = 0;
		VKFreeCommandBuffers vk_free_command_buffers_ = 0;
		VKDestroyCommandPool vk_destroy_command_pool_ = 0;
		VKDestroyDevice vk_destroy_device_ = 0;

		//API list
		CLGetPlatformIDs cl_get_platform_ids_ = 0;
		CLGetInfo cl_get_info_ = 0;
		CLGetDeviceIDs cl_get_device_ids_ = 0;
		CLGetDeviceInfo cl_get_device_info_ = 0;
		CLCreateContextFromType cl_create_context_from_type_ = 0;
		CLCreateContext cl_create_context_ = 0;
		CLWaitForEvents cl_wait_for_events_ = 0;
		CLCreateBuffer cl_create_buffer_ = 0;
		CLGetMemObjectInfo cl_get_mem_object_info_ = 0;
		CLCreateSubBuffer cl_create_sub_buffer_ = 0;
		CLCreateSampler cl_create_sampler_ = 0;
		CLGetSamplerInfo cl_get_sampler_info_ = 0;
		CLReleaseSampler cl_release_sampler_ = 0;
		CLGetImageInfo cl_get_image_info_ = 0;
		CLGetEventInfo cl_get_event_info_ = 0;
		CLGetEventProfilingInfo cl_get_event_profiling_info_ = 0;
		CLSetEventCallback cl_set_event_callback_ = 0;
		CLReleaseEvent cl_release_event_ = 0;
		CLGetContextInfo cl_get_context_info = 0;
		CLCreateImage2D cl_create_image_2d_ = 0;
		CLSetUserEventStatus cl_set_user_event_status_ = 0;
		CLCreateUserEvent cl_create_user_event_ = 0;
		CLCreateCommandQueue cl_create_command_queue_ = 0;
		CLGetSupportedImageFormat cl_get_supported_image_format_ = 0;
		CLGetCommandQueueInfo cl_get_command_queue_info_ = 0;
		CLReleaseContext cl_release_context_ = 0;
		CLReleaseProgram cl_release_program_ = 0;
		CLReleaseKernel cl_release_kernel_ = 0;
		CLReleaseMemObject cl_release_mem_object_ = 0;
		CLReleaseCommandQueue cl_release_command_queue_ = 0;
		CLEnqueueCopyBuffer cl_enqueue_copy_buffer_ = 0;
		CLEnqueueCopyBufferRect cl_enqueue_copy_buffer_rect_ = 0;
		CLEnqueueCopyImage cl_enqueue_copy_image_ = 0;
		CLEnqueueCopyBufferToImage cl_enqueue_copy_buffer_to_image_ = 0;
		CLEnqueueCopyImageToBuffer cl_enqueue_copy_image_to_buffer_ = 0;
		CLEnqueueReadBuffer cl_enqueue_read_buffer_ = 0;
		CLEnqueueReadBufferRect cl_enqueue_read_buffer_rect_ = 0;
		CLEnqueueReadImage cl_enqueue_read_image_ = 0;
		CLEnqueueWriteBuffer cl_enqueue_write_buffer_ = 0;
		CLEnqueueWriteBufferRect cl_enqueue_write_buffer_rect_ = 0;
		CLEnqueueWriteImage cl_enqueue_write_image_ = 0;
		CLEnqueueNDRangeKernel cl_enqueue_n_d_range_kernel_ = 0;
		CLFinish cl_finish_ = 0;
		CLFlush cl_flush_ = 0;
		CLGetKernelInfo cl_get_kernel_info_ = 0;
		CLGetKernelWorkGroupInfo cl_get_kernel_work_group_info_ = 0;
		CLGetKernelArgInfo cl_get_kernel_arg_info_ = 0;
		CLSetKernelArg cl_set_kernel_arg_ = 0;
		CLGetProgramInfo cl_get_program_info_ = 0;
		CLCreateProgramWithSource cl_create_program_with_source_ = 0;
		CLGetProgramBuildInfo cl_get_program_build_info_ = 0;
		CLBuildProgram cl_build_program_ = 0;
		CLEnqueueMarker cl_enqueue_marker_ = 0;
		CLEnqueueBarrier cl_enqueue_barrier_ = 0;
		CLEnqueueWaitForEvents cl_enqueue_wait_for_events_ = 0;
		CLCreateKernel cl_create_kernel_ = 0;
		CLCreateKernelsInProgram cl_create_kernels_in_program_ = 0;
		// gl/cl sharing
		CLCreateFromGLBuffer cl_create_from_gl_buffer_ = 0;
		CLCreateFromGLRenderbuffer cl_create_from_gl_render_buffer_ = 0;
		CLCreateFromGLTexture2D cl_create_from_gl_texture_2d_ = 0;
		CLGetGLObjectInfo cl_get_gl_object_info_ = 0;
		CLGetGLTextureInfo cl_get_gl_texture_info_ = 0;
		CLEnqueueAcquireGLObjects cl_enqueue_acquire_globjects_ = 0;
		CLEnqueueReleaseGLObjects cl_enqueue_release_globjects_ = 0;

		//commandQueue garbage pointer vector
		std::vector<cl_point> garbage_ptr_list_unsigned_char;
		std::vector<cl_point> garbage_ptr_list_unsigned;
		std::vector<cl_point> garbage_ptr_list_int;
		std::vector<cl_point> garbage_ptr_list_float;
		std::vector<cl_point> garbage_ptr_list_double;
		std::vector<cl_point> garbage_ptr_list_cl_mem;

	    std::unique_ptr<base::SharedMemory> mSharedData;
	    std::unique_ptr<base::SharedMemory> mSharedOperation;
	    std::unique_ptr<base::SharedMemory> mSharedResult;
	    std::unique_ptr<base::SharedMemory> mSharedEvents;

	    void* mSharedDataPtr;
	    BaseOperationData* mSharedOperationPtr;
	    BaseResultData* mSharedResultPtr;
	    cl_point* mSharedEventsPtr;
	};

	void InitializeStaticCLBindings(CLApi* apiImpl);
}

#endif  // UI_OPENCL_OPENCL_IMPLEMENTATION_H_
