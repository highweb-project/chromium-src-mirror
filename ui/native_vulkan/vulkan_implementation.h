// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_NATIVE_VULKAN_IMPLEMENTATION_H_
#define UI_NATIVE_VULKAN_IMPLEMENTATION_H_

#include "base/native_library.h"
#include "base/memory/ref_counted.h"
#include "base/memory/shared_memory.h"

#include "ui/native_vulkan/vulkan_include.h"

namespace gpu {
class GpuChannel;
}

namespace gfx {

	//define Vulkan function pointer type
	VK_API_ARGS3(VKCreateInstance, VKCResult, ARG_CONST(ARG_PTR(VkInstanceCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkInstance));
	VK_API_ARGS3(VKEnumeratePhysicalDevices, VKCResult, VkInstance, ARG_PTR(uint32_t), ARG_PTR(VkPhysicalDevice));
	VK_API_ARGS2(VKGetPhysicalDeviceProperties, ARG_PTR(void), VkPhysicalDevice, ARG_PTR(VkPhysicalDeviceProperties));
	VK_API_ARGS3(VKGetPhysicalDeviceQueueFamilyProperties, ARG_PTR(void), VkPhysicalDevice, ARG_PTR(uint32_t), ARG_PTR(VkQueueFamilyProperties));
	VK_API_ARGS4(VKCreateDevice, VKCResult, VkPhysicalDevice, ARG_CONST(ARG_PTR(VkDeviceCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkDevice));
	VK_API_ARGS4(VKGetDeviceQueue, ARG_PTR(void), VkDevice, uint32_t, uint32_t, ARG_PTR(VkQueue));
	VK_API_ARGS2(VKGetPhysicalDeviceMemoryProperties, ARG_PTR(void), VkPhysicalDevice, ARG_PTR(VkPhysicalDeviceMemoryProperties));
	VK_API_ARGS4(VKCreateCommandPool, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkCommandPoolCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkCommandPool));
	VK_API_ARGS3(VKAllocateCommandBuffers, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkCommandBufferAllocateInfo)), ARG_PTR(VkCommandBuffer));
	VK_API_ARGS4(VKCreateBuffer, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkBufferCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkBuffer));
	VK_API_ARGS3(VKGetBufferMemoryRequirements, ARG_PTR(void), VkDevice, VkBuffer, ARG_PTR(VkMemoryRequirements));
	VK_API_ARGS4(VKAllocateMemory, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkMemoryAllocateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkDeviceMemory));
	VK_API_ARGS4(VKBindBufferMemory, VKCResult, VkDevice, VkBuffer, VkDeviceMemory, VkDeviceSize);
	VK_API_ARGS6(VKMapMemory, VKCResult, VkDevice, VkDeviceMemory, VkDeviceSize, VkDeviceSize, VkMemoryMapFlags, ARG_PTR(ARG_PTR(void)));
	VK_API_ARGS2(VKUnmapMemory, ARG_PTR(void), VkDevice, VkDeviceMemory);
	VK_API_ARGS4(VKCreateShaderModule, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkShaderModuleCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkShaderModule));
	VK_API_ARGS4(VKCreateDescriptorSetLayout, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkDescriptorSetLayoutCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkDescriptorSetLayout));
	VK_API_ARGS4(VKCreatePipelineLayout, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkPipelineLayoutCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkPipelineLayout));
	VK_API_ARGS4(VKCreatePipelineCache, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkPipelineCacheCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkPipelineCache));
	VK_API_ARGS6(VKCreateComputePipelines, VKCResult, VkDevice, VkPipelineCache, uint32_t, ARG_CONST(ARG_PTR(VkComputePipelineCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkPipeline));
	VK_API_ARGS4(VKCreateDescriptorPool, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkDescriptorPoolCreateInfo)), ARG_CONST(ARG_PTR(VkAllocationCallbacks)), ARG_PTR(VkDescriptorPool));
	VK_API_ARGS3(VKAllocateDescriptorSets, VKCResult, VkDevice, ARG_CONST(ARG_PTR(VkDescriptorSetAllocateInfo)), ARG_PTR(VkDescriptorSet));
	VK_API_ARGS5(VKUpdateDescriptorSets, ARG_PTR(void), VkDevice, uint32_t, ARG_CONST(ARG_PTR(VkWriteDescriptorSet)), uint32_t, ARG_CONST(ARG_PTR(VkCopyDescriptorSet)));
	VK_API_ARGS2(VKBeginCommandBuffer, VKCResult, VkCommandBuffer, ARG_CONST(ARG_PTR(VkCommandBufferBeginInfo)));
	VK_API_ARGS8(VKCmdBindDescriptorSets, ARG_PTR(void), VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t, uint32_t, ARG_CONST(ARG_PTR(VkDescriptorSet)), uint32_t, ARG_CONST(ARG_PTR(uint32_t)));
	VK_API_ARGS3(VKCmdBindPipeline, ARG_PTR(void), VkCommandBuffer, VkPipelineBindPoint, VkPipeline);
	VK_API_ARGS4(VKCmdDispatch, ARG_PTR(void), VkCommandBuffer, uint32_t, uint32_t, uint32_t);
	VK_API_ARGS10(VKCmdPipelineBarrier, ARG_PTR(void), VkCommandBuffer, VkPipelineStageFlags, VkPipelineStageFlags, VkDependencyFlags, uint32_t, ARG_CONST(ARG_PTR(VkMemoryBarrier)), uint32_t, ARG_CONST(ARG_PTR(VkBufferMemoryBarrier)), uint32_t, ARG_CONST(ARG_PTR(VkImageMemoryBarrier)));
	VK_API_ARGS1(VKEndCommandBuffer, VKCResult, VkCommandBuffer);
	VK_API_ARGS4(VKQueueSubmit, VKCResult, VkQueue, uint32_t, ARG_CONST(ARG_PTR(VkSubmitInfo)), VkFence);
	VK_API_ARGS1(VKQueueWaitIdle, VKCResult, VkQueue);
	VK_API_ARGS1(VKDeviceWaitIdle, VKCResult, VkDevice);
	VK_API_ARGS5(VKCmdCopyBuffer, ARG_PTR(void), VkCommandBuffer, VkBuffer, VkBuffer, uint32_t, ARG_CONST(ARG_PTR(VkBufferCopy)));
	VK_API_ARGS3(VKFreeMemory, ARG_PTR(void), VkDevice, VkDeviceMemory, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS3(VKDestroyBuffer, ARG_PTR(void), VkDevice, VkBuffer, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS4(VKFreeDescriptorSets, VKCResult, VkDevice, VkDescriptorPool, uint32_t, ARG_CONST(ARG_PTR(VkDescriptorSet)));
	VK_API_ARGS3(VKDestroyDescriptorPool, ARG_PTR(void), VkDevice, VkDescriptorPool, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS4(VKFreeCommandBuffers, ARG_PTR(void), VkDevice, VkCommandPool, uint32_t, ARG_CONST(ARG_PTR(VkCommandBuffer)));
	VK_API_ARGS3(VKDestroyCommandPool, ARG_PTR(void), VkDevice, VkCommandPool, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS2(VKDestroyDevice, ARG_PTR(void), VkDevice, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS2(VKDestroyInstance, ARG_PTR(void), VkInstance, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS3(VKDestroyDescriptorSetLayout, ARG_PTR(void), VkDevice, VkDescriptorSetLayout, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS3(VKDestroyPipelineLayout, ARG_PTR(void), VkDevice, VkPipelineLayout, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS3(VKDestroyShaderModule, ARG_PTR(void), VkDevice, VkShaderModule, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS3(VKDestroyPipelineCache, ARG_PTR(void), VkDevice, VkPipelineCache, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS3(VKDestroyPipeline, ARG_PTR(void), VkDevice, VkPipeline, ARG_CONST(ARG_PTR(VkAllocationCallbacks)));
	VK_API_ARGS4(VKEnumerateDeviceExtensionProperties, VKCResult, VkPhysicalDevice, ARG_CONST(ARG_PTR(char)), ARG_PTR(uint32_t), ARG_PTR(VkExtensionProperties));
	VK_API_ARGS3(VKEnumerateDeviceLayerProperties, VKCResult, VkPhysicalDevice, ARG_PTR(uint32_t), ARG_PTR(VkLayerProperties));
	VK_API_ARGS2(VKEnumerateInstanceLayerProperties, VKCResult, ARG_PTR(uint32_t), ARG_PTR(VkLayerProperties));
	VK_API_ARGS3(VKEnumerateInstanceExtensionProperties, VKCResult, ARG_CONST(ARG_PTR(char)), ARG_PTR(uint32_t), ARG_PTR(VkExtensionProperties));

	class VKCApi {
	public :
		VKCApi();
		~VKCApi();
		void InitApi(base::NativeLibrary nativeLib);

#if defined(OS_LINUX)
		static gpu::GpuChannel* parent_channel_;
#elif defined(OS_ANDROID)
		void setChannel(gpu::GpuChannel* channel);
#endif

		VKCResult doVKCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
		VKCResult doVKEnumeratePhysicalDevices(VkInstance pCreateInfo, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
		void doVKGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
		void doVKGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties);
		VKCResult doVKCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
		void doVKGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
		void doVKGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties);
		VKCResult doVKCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
		VKCResult doVKAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);
		VKCResult doVKCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);
		void doVKGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
		VKCResult doVKAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
		VKCResult doVKBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
		VKCResult doVKMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
		void doVKUnmapMemory(VkDevice device, VkDeviceMemory memory);
		VKCResult doVKCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
		VKCResult doVKCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
		VKCResult doVKCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
		VKCResult doVKCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);
		VKCResult doVKCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
		VKCResult doVKCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);
		VKCResult doVKAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets);
		void doVKUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies);
		VKCResult doVKBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
		void doVKCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
		void doVKCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
		void doVKCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
		void doVKCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
		VKCResult doVKEndCommandBuffer(VkCommandBuffer commandBuffer);
		VKCResult doVKQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
		VKCResult doVKQueueWaitIdle(VkQueue queue);
		VKCResult doVKDeviceWaitIdle(VkDevice device);
		void doVKCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
		void doVKFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
		VKCResult doVKFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets);
		void doVKDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator);
		void doVKFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
		void doVKDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyDescriptorSetLayout(VkDevice, VkDescriptorSetLayout, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyPipelineLayout(VkDevice, VkPipelineLayout, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyShaderModule(VkDevice, VkShaderModule, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyPipelineCache(VkDevice, VkPipelineCache, const VkAllocationCallbacks* pAllocator);
		void doVKDestroyPipeline(VkDevice, VkPipeline, const VkAllocationCallbacks* pAllocator);
		VKCResult doVKEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
		VKCResult doVKEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties);
		VKCResult doVKEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties);
		VKCResult doVKEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);

		bool setSharedMemory(base::SharedMemoryHandle dataHandle, base::SharedMemoryHandle operationHandle, base::SharedMemoryHandle resultHandle);
		bool clearSharedMemory();

		int vkcCreateInstance(const std::vector<std::string>& names, const std::vector<uint32_t>& versions, const std::vector<std::string>& enabledLayerNames, const std::vector<std::string>& enabledExtensionNames, VKCPoint* vkcInstance);
		int vkcDestroyInstance(VKCPoint vkcInstance);
		int vkcEnumeratePhysicalDevice(VKCPoint vkcInstance, VKCuint* physicalDeviceCount, VKCPoint* physicalDeviceList);
		int vkcDestroyPhysicalDevice(VKCPoint physicalDeviceList);
		int vkcCreateDevice(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, VKCPoint* vkcDevice, VKCPoint* vkcQueue);
		int vkcDestroyDevice(VKCPoint vkcDevice, VKCPoint vkcQueue);
		int vkcGetDeviceInfo(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, const VKCuint& name, void* data);
		int vkcCreateBuffer(const VKCPoint& vkcDevice, const VKCPoint& physicalDeviceList, const VKCuint& vdIndex, const VKCuint& sizeInBytes, VKCPoint* vkcBuffer, VKCPoint* vkcMemory);
		int vkcReleaseBuffer(const VKCPoint& vkcDevice, const VKCPoint& vkcBuffer, const VKCPoint& vkcMemory);
		int vkcFillBuffer(const VKCPoint& vkcDevice, const VKCPoint& vkcMemory, const std::vector<VKCuint>& uintVector);
		void vkcReadBuffer();
		void vkcWriteBuffer();
		int vkcCreateCommandQueue(const VKCPoint& vkcDevice, const VKCPoint& physicalDeviceList, const VKCuint& vdIndex, VKCPoint* vkcCMDBuffer, VKCPoint* vkcCMDPool);
		int vkcReleaseCommandQueue(const VKCPoint& vkcDevice, const VKCPoint& vkcCMDBuffer, const VKCPoint& vkcCMDPool);
		int vkcCreateDescriptorSetLayout(const VKCPoint& vkcDevice, const VKCuint& useBufferCount, VKCPoint* vkcDescriptorSetLayout);
		int vkcReleaseDescriptorSetLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSetLayout);
		int vkcCreateDescriptorPool(const VKCPoint& vkcDevice, const VKCuint& useBufferCount, VKCPoint* vkcDescriptorPool);
		int vkcReleaseDescriptorPool(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool);
		int vkcCreateDescriptorSet(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, const VKCPoint& vkcDescriptorSetLayout, VKCPoint* vkcDescriptorSet);
		int vkcReleaseDescriptorSet(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, const VKCPoint& vkcDescriptorSet);
		int vkcCreatePipelineLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSetLayout, VKCPoint* vkcPipelineLayout);
		int vkcReleasePipelineLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineLayout);
		int vkcCreateShaderModule(const VKCPoint& vkcDevice, const std::string& shaderPath, VKCPoint* vkcShaderModule);
		int vkcReleaseShaderModule(const VKCPoint& vkcDevice, const VKCPoint& vkcShaderModule);
		int vkcCreatePipeline(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineLayout, const VKCPoint& vkcShaderModule, VKCPoint* vkcPipelineCache, VKCPoint* vkcPipeline);
		int vkcReleasePipeline(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineCache, const VKCPoint& vkcPipeline);
		int vkcUpdateDescriptorSets(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSet, const std::vector<VKCPoint>& bufferVector);
		int vkcBeginQueue(const VKCPoint& vkcCMDBuffer, const VKCPoint& vkcPipeline, const VKCPoint& vkcPipelineLayout, const VKCPoint& vkcDescriptorSet);
		int vkcEndQueue(const VKCPoint& vkcCMDBuffer);
		int vkcDispatch(const VKCPoint& vkcCMDBuffer, const VKCuint& workGroupX, const VKCuint& workGroupY, const VKCuint& workGroupZ);
		int vkcPipelineBarrier(const VKCPoint& vkcCMDBuffer);
		int vkcCmdCopyBuffer(const VKCPoint& vkcCMDBuffer, const VKCPoint& srcBuffer, const VKCPoint& dstBuffer, const VKCuint& copySize);
		int vkcQueueSubmit(const VKCPoint& vkcQuque, const VKCPoint& vkcCMDBuffer);
		int vkcDeviceWaitIdle(const VKCPoint& vkcDevice);

	private:

		VKCuint getShaderCode(std::string shaderPath, char* shaderCode, VKCuint maxSourceLength);

		#if defined(OS_ANDROID)
		gpu::GpuChannel* gpu_channel_;
		#endif

		bool vkcLibraryLoaded_ = false;

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
		VKDestroyInstance vk_destroy_instance_ = 0;
		VKDestroyDescriptorSetLayout vk_destroy_descriptor_set_layout_ = 0;
		VKDestroyPipelineLayout vk_destroy_pipeline_layout_ = 0;
		VKDestroyShaderModule vk_destroy_shader_module_ = 0;
		VKDestroyPipelineCache vk_destroy_pipeline_cache_ = 0;
		VKDestroyPipeline vk_destroy_pipeline_ = 0;
		VKEnumerateDeviceLayerProperties vk_enumerate_device_layer_properties_ = 0;
		VKEnumerateDeviceExtensionProperties vk_enumerate_device_extension_properties_ = 0;
		VKEnumerateInstanceLayerProperties vk_enumerate_instance_layer_properties_ = 0;
		VKEnumerateInstanceExtensionProperties vk_enumerate_instance_extension_properties_ = 0;

		std::unique_ptr<base::SharedMemory> mSharedData;
		std::unique_ptr<base::SharedMemory> mSharedOperation;
		std::unique_ptr<base::SharedMemory> mSharedResult;
		std::unique_ptr<base::SharedMemory> mSharedEvents;

		void* mSharedDataPtr;
		BaseVKCOperationData* mSharedOperationPtr;
		BaseVKCResultData* mSharedResultPtr;
	};

	void InitializeStaticVKCBindings(VKCApi* apiImpl);
}

#endif  // UI_NATIVE_VULKAN_IMPLEMENTATION_H_
