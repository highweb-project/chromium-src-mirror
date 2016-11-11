#include "ui/native_vulkan/vulkan_implementation.h"

#include "base/files/file_path.h"
#include "gpu/ipc/service/gpu_channel.h"
#include "dirent.h"
#include <array>

#include "gpu/ipc/common/gpu_messages.h"
#include "gpu/ipc/service/gpu_channel_manager.h"

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#if defined(OS_LINUX)
gpu::GpuChannel* gfx::VKCApi::parent_channel_;
#endif

namespace gfx {

void InitializeStaticVKCBindings(VKCApi* apiImpl) {
	VKCLOG(INFO) << "InitializeStaticVKCBindings";

	// determine so library
	std::string vulkanSoFile = "libvulkan.so";

	#if defined(OS_ANDROID)
	// vulkanSoFile = "/system/lib64/libvulkan_sec.so";
	// DIR *dir;
	// struct dirent *ent;
	// struct stat st;
	//
	// std::string directory = "/system/lib64";
	//
	// dir = opendir("/system/lib64");
	// if (NULL != dir) {
	// 	while ((ent = readdir(dir)) != NULL) {
	// 		// const std::string file_name = ent->d_name;
	// 		const std::string file_name = "libvulkan_sec.so";
	// 		const std::string full_file_name = directory + "/" + file_name;
	//
	// 		if (file_name[0] == '.')
	// 		   continue;
	//
	// 		if (stat(full_file_name.c_str(), &st) == -1)
	// 		   continue;
	//
	// 		const bool is_directory = (st.st_mode & S_IFDIR) != 0;
	//
	// 		if (is_directory)
	// 		   continue;
	//
	// 		DLOG(INFO) << "full_file_name : " << full_file_name;
	//
	// 		base::FilePath icdFilePath = base::FilePath(full_file_name);
	// 		std::string soFileName;
	//
	// 		bool result = base::ReadFileToString(icdFilePath, &soFileName);
	// 		DLOG(INFO) << "base::ReadFileToString result : " << result;
	//
	// 		if(result) {
	// 			// http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
	// 			soFileName.erase(soFileName.find_last_not_of(" \n\r\t")+1);
	//
	// 			DLOG(INFO) << "soFileName : " << soFileName;
	// 			DLOG(INFO) << "soFileName.length() : " << soFileName.length();
	// 			// vulkanSoFile = soFileName;
	// 		}
	//
	// 		break;
	// 	}
	// 	// vulkanSoFile = directory + "/" + file_name;
	// 	closedir(dir);
	// }
	#endif

	//load so library
	base::FilePath fileNameVulkan = base::FilePath(vulkanSoFile);
	base::NativeLibraryLoadError errorVulkan;
	base::NativeLibrary vulkan_library = base::LoadNativeLibrary(fileNameVulkan, &errorVulkan);
	if (!vulkan_library) {
		VKCLOG(INFO) << "load vulkan library failed!!";
		// [ERROR:native_library_posix.cc(41)] dlclose failed: NULL library handle
		// base::UnloadNativeLibrary(opencl_library);
	}
	else {
		VKCLOG(INFO) << "load vulkan library success!!";
		apiImpl->InitApi(vulkan_library);
	}

}

VKCApi::VKCApi(){
	vkcLibraryLoaded_ = false;
}

VKCResult VKCApi::doVKCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
	VKCLOG(INFO) << "VKCApi::doCreateInstance Vulkan";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_instance_(pCreateInfo, pAllocator, pInstance);
	VKCLOG(INFO) << "vkCreateInstance err : " << err;
	return err;
}

VKCResult VKCApi::doVKEnumeratePhysicalDevices(VkInstance pCreateInfo, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
	VKCLOG(INFO) << "VKCApi::doVKEnumeratePhysicalDevices";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_enumerate_physical_device_(pCreateInfo, pPhysicalDeviceCount, pPhysicalDevices);
	VKCLOG(INFO) << "vkEnumeratePhysicalDevices : " << err;
	return err;
}

void VKCApi::doVKGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
	VKCLOG(INFO) << "VKCApi::doVKGetPhysicalDeviceProperties";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_get_physical_device_properties_(physicalDevice, pProperties);
}

void VKCApi::doVKGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
	VKCLOG(INFO) << "VKCApi::doVKGetPhysicalDeviceQueueFamilyProperties";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_get_physical_device_queue_family_properties_(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKCResult VKCApi::doVKCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
	VKCLOG(INFO) << "VKCApi::doVKCreateDevice";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_device_(physicalDevice, pCreateInfo, pAllocator, pDevice);
	VKCLOG(INFO) << "vkCreateDevice : " << err << " " << pDevice;
	return err;
}

void VKCApi::doVKGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
	VKCLOG(INFO) << "VKCApi::doVKGetDeviceQueue " << pQueue;
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_get_device_queue_(device, queueFamilyIndex, queueIndex, pQueue);
	VKCLOG(INFO) << "vkGetDeviceQueue : " << pQueue;
}

void VKCApi::doVKGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
	VKCLOG(INFO) << "VKCApi::doVKGetPhysicalDeviceMemoryProperties " << pMemoryProperties;
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_get_physical_device_memory_properties_(physicalDevice, pMemoryProperties);
	VKCLOG(INFO) << "vkGetPhysicalDeviceMemoryProperties : " << pMemoryProperties;
}

VKCResult VKCApi::doVKCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
	VKCLOG(INFO) << "VKCApi::doVKCreateCommandPool";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_command_pool_(device, pCreateInfo, pAllocator, pCommandPool);
	VKCLOG(INFO) << "vkCreateCommandPool : " << err;
	return err;
}

VKCResult VKCApi::doVKAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) {
	VKCLOG(INFO) << "VKCApi::doVKAllocateCommandBuffers";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_allocate_command_buffers_(device, pAllocateInfo, pCommandBuffers);
	VKCLOG(INFO) << "vkAllocateCommandBuffers : " << err;
	return err;
}

VKCResult VKCApi::doVKCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
	VKCLOG(INFO) << "VKCApi::doVKCreateBuffer";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_buffer_(device, pCreateInfo, pAllocator, pBuffer);
	VKCLOG(INFO) << "vkCreateBuffer : " << err;
	return err;
}

void VKCApi::doVKGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
	VKCLOG(INFO) << "VKCApi::doVKGetBufferMemoryRequirements " << pMemoryRequirements;
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_get_buffer_memory_requirements_(device, buffer, pMemoryRequirements);
	VKCLOG(INFO) << "vkGetBufferMemoryRequirements : " << pMemoryRequirements;
}

VKCResult VKCApi::doVKAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
	VKCLOG(INFO) << "VKCApi::doVKAllocateMemory";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_allocate_memory_(device, pAllocateInfo, pAllocator, pMemory);
	VKCLOG(INFO) << "vkAllocateMemory : " << err;
	return err;
}

VKCResult VKCApi::doVKBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
	VKCLOG(INFO) << "VKCApi::doVKBindBufferMemory";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_bind_buffer_memory_(device, buffer, memory, memoryOffset);
	VKCLOG(INFO) << "vkBindBufferMemory : " << err;
	return err;
}

VKCResult VKCApi::doVKMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)  {
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_map_memory_(device, memory, offset, size, flags, ppData);
	return err;
}

void VKCApi::doVKUnmapMemory(VkDevice device, VkDeviceMemory memory) {
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_unmap_memory_(device, memory);
}

VKCResult VKCApi::doVKCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
	VKCLOG(INFO) << "VKCApi::doVKCreateShaderModule";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_shader_module_(device, pCreateInfo, pAllocator, pShaderModule);
	VKCLOG(INFO) << "vkCreateShaderModule : " << err;
	return err;
}

VKCResult VKCApi::doVKCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) {
	VKCLOG(INFO) << "VKCApi::doVKCreateDescriptorSetLayout";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_descriptor_set_layout_(device, pCreateInfo, pAllocator, pSetLayout);
	VKCLOG(INFO) << "vkCreateDescriptorSetLayout : " << err;
	return err;
}

VKCResult VKCApi::doVKCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
	VKCLOG(INFO) << "VKCApi::doVKCreatePipelineLayout";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_pipeline_layout_(device, pCreateInfo, pAllocator, pPipelineLayout);
	VKCLOG(INFO) << "vkCreatePipelineLayout : " << err;
	return err;
}

VKCResult VKCApi::doVKCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
	VKCLOG(INFO) << "VKCApi::doVKCreatePipelineCache";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_pipeline_cache_(device, pCreateInfo, pAllocator, pPipelineCache);
	VKCLOG(INFO) << "vkCreatePipelineCache : " << err;
	return err;
}

VKCResult VKCApi::doVKCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
	VKCLOG(INFO) << "VKCApi::doVKCreateComputePipelines";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_compute_pipelines_(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
	VKCLOG(INFO) << "vkCreateComputePipelines : " << err;
	return err;
}

VKCResult VKCApi::doVKCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
	VKCLOG(INFO) << "VKCApi::doVKCreateDescriptorPool";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_create_descriptor_pool_(device, pCreateInfo, pAllocator, pDescriptorPool);
	VKCLOG(INFO) << "vkCreateDescriptorPool : " << err;
	return err;
}

VKCResult VKCApi::doVKAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) {
	VKCLOG(INFO) << "VKCApi::doVKAllocateDescriptorSets";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_allocate_descriptor_sets_(device, pAllocateInfo, pDescriptorSets);
	VKCLOG(INFO) << "vkAllocateDescriptorSets : " << err;
	return err;
}

void VKCApi::doVKUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
	VKCLOG(INFO) << "VKCApi::doVKUpdateDescriptorSets " << pDescriptorCopies;
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_update_descriptor_sets_(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
	VKCLOG(INFO) << "vkUpdateDescriptorSets : " << pDescriptorCopies;
}

VKCResult VKCApi::doVKBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
	VKCLOG(INFO) << "VKCApi::doVKBeginCommandBuffer";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_begin_command_buffer_(commandBuffer, pBeginInfo);
	VKCLOG(INFO) << "vkBeginCommandBuffer : " << err;
	return err;
}

void VKCApi::doVKCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
	VKCLOG(INFO) << "VKCApi::doVKCmdBindDescriptorSets";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_cmd_bind_descriptor_sets_(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
	VKCLOG(INFO) << "VKCApi::doVKCmdBindDescriptorSets finish";
}

void VKCApi::doVKCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
	VKCLOG(INFO) << "VKCApi::doVKCmdBindPipeline";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_cmd_bind_pipeline_(commandBuffer, pipelineBindPoint, pipeline);
}

void VKCApi::doVKCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_cmd_dispatch_(commandBuffer, x, y, z);
}

void VKCApi::doVKCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_cmd_pipeline_barrier_(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKCResult VKCApi::doVKEndCommandBuffer(VkCommandBuffer commandBuffer) {
	VKCLOG(INFO) << "VKCApi::doVKEndCommandBuffer";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_end_command_buffer_(commandBuffer);
	VKCLOG(INFO) << "vkEndCommandBuffer : " << err;
	return err;
}

VKCResult VKCApi::doVKQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_queue_submit_(queue, submitCount, pSubmits, fence);
	return err;
}

VKCResult VKCApi::doVKQueueWaitIdle(VkQueue queue) {
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_queue_wait_idle_(queue);
	return err;
}

VKCResult VKCApi::doVKDeviceWaitIdle(VkDevice device) {
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_device_wait_idle_(device);
	return err;
}

void VKCApi::doVKCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
	VKCLOG(INFO) << "VKCApi::doVKCmdCopyBuffer";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_cmd_copy_buffer_(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void VKCApi::doVKFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKFreeMemory";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_free_memory_(device, memory, pAllocator);
}

void VKCApi::doVKDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyBuffer";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_buffer_(device, buffer, pAllocator);
}

VKCResult VKCApi::doVKFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) {
	VKCLOG(INFO) << "VKCApi::doVKFreeDescriptorSets";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult err = vk_free_descriptor_sets_(device, descriptorPool, descriptorSetCount, pDescriptorSets);
	VKCLOG(INFO) << "vkFreeDescriptorSets : " << err;
	return err;
}

void VKCApi::doVKDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyDescriptorPool";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_descriptor_pool_(device, descriptorPool, pAllocator);
}

void VKCApi::doVKFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
	VKCLOG(INFO) << "VKCApi::doVKFreeCommandBuffers";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_free_command_buffers_(device, commandPool, commandBufferCount, pCommandBuffers);
}

void VKCApi::doVKDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyCommandPool";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_command_pool_(device, commandPool, pAllocator);
}

void VKCApi::doVKDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyDevice";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_device_(device, pAllocator);
}

void VKCApi::doVKDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyInstance";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_instance_(instance, pAllocator);
}

void VKCApi::doVKDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyDescriptorSetLayout";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_descriptor_set_layout_(device, descriptorSetLayout, pAllocator);
}

void VKCApi::doVKDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyPipelineLayout";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_pipeline_layout_(device, pipelineLayout, pAllocator);
}

void VKCApi::doVKDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyShaderModule";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_shader_module_(device, shaderModule, pAllocator);
}

void VKCApi::doVKDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyPipelineCache";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_pipeline_cache_(device, pipelineCache, pAllocator);
}

void VKCApi::doVKDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
	VKCLOG(INFO) << "VKCApi::doVKDestroyPipeline";
	if (!vkcLibraryLoaded_) {
		return;
	}
	vk_destroy_pipeline_(device, pipeline, pAllocator);
}

VKCResult VKCApi::doVKEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
	VKCLOG(INFO) << "VKCApi::doVKEnumerateDeviceLayerProperties";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult result = vk_enumerate_device_layer_properties_(physicalDevice, pPropertyCount, pProperties);
	return result;
}

VKCResult VKCApi::doVKEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
	VKCLOG(INFO) << "VKCApi::doVKEnumerateDeviceExtensionProperties";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult result = vk_enumerate_device_extension_properties_(physicalDevice, pLayerName, pPropertyCount, pProperties);
	return result;
}
VKCResult VKCApi::doVKEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
		VKCLOG(INFO) << "doVKEnumerateInstanceLayerProperties";
		if (!vkcLibraryLoaded_) {
			return VK_NOT_READY;
		}
		VKCResult result = vk_enumerate_instance_layer_properties_(pPropertyCount, pProperties);
		return result;
}

VKCResult VKCApi::doVKEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
	VKCLOG(INFO) << "doVKEnumerateInstanceExtensionProperties";
	if (!vkcLibraryLoaded_) {
		return VK_NOT_READY;
	}
	VKCResult result = vk_enumerate_instance_extension_properties_(pLayerName, pPropertyCount, pProperties);
	return result;
}

VKCApi::~VKCApi(){
}

static void* handleFuncLookupFail(std::string api_name) {
	VKCLOG(INFO) << "handleFuncLookupFail, api_name : " << api_name;

	return nullptr;
}

#if defined(OS_ANDROID)
void VKCApi::setChannel(gpu::GpuChannel* channel) {
	VKCLOG(INFO) << "VKCApi::setChannel, VKCApi : " << this << ", channel : " << channel;
	gpu_channel_ = channel;
};
#endif

void VKCApi::InitApi(base::NativeLibrary vulkanLib) {
	VKCLOG(INFO) << "CLApi::InitVulkanApi";
	vulkan_library_ = vulkanLib;
	vk_create_instance_ = VK_API_LOAD(vulkan_library_, "vkCreateInstance", VKCreateInstance);
	vk_enumerate_physical_device_ = VK_API_LOAD(vulkan_library_, "vkEnumeratePhysicalDevices", VKEnumeratePhysicalDevices);
	vk_get_physical_device_properties_ = VK_API_LOAD(vulkan_library_, "vkGetPhysicalDeviceProperties", VKGetPhysicalDeviceProperties);
	vk_get_physical_device_queue_family_properties_ = VK_API_LOAD(vulkan_library_, "vkGetPhysicalDeviceQueueFamilyProperties", VKGetPhysicalDeviceQueueFamilyProperties);
	vk_create_device_ = VK_API_LOAD(vulkan_library_, "vkCreateDevice", VKCreateDevice);
	vk_get_device_queue_ = VK_API_LOAD(vulkan_library_, "vkGetDeviceQueue", VKGetDeviceQueue);
	vk_get_physical_device_memory_properties_ = VK_API_LOAD(vulkan_library_, "vkGetPhysicalDeviceMemoryProperties", VKGetPhysicalDeviceMemoryProperties);
	vk_create_command_pool_ = VK_API_LOAD(vulkan_library_, "vkCreateCommandPool", VKCreateCommandPool);
	vk_allocate_command_buffers_ = VK_API_LOAD(vulkan_library_, "vkAllocateCommandBuffers", VKAllocateCommandBuffers);
	vk_create_buffer_ = VK_API_LOAD(vulkan_library_, "vkCreateBuffer", VKCreateBuffer);
	vk_get_buffer_memory_requirements_ = VK_API_LOAD(vulkan_library_, "vkGetBufferMemoryRequirements", VKGetBufferMemoryRequirements);
	vk_allocate_memory_ = VK_API_LOAD(vulkan_library_, "vkAllocateMemory", VKAllocateMemory);
	vk_bind_buffer_memory_ = VK_API_LOAD(vulkan_library_, "vkBindBufferMemory", VKBindBufferMemory);
	vk_map_memory_ = VK_API_LOAD(vulkan_library_, "vkMapMemory", VKMapMemory);
	vk_unmap_memory_ = VK_API_LOAD(vulkan_library_, "vkUnmapMemory", VKUnmapMemory);
	vk_create_shader_module_ = VK_API_LOAD(vulkan_library_, "vkCreateShaderModule", VKCreateShaderModule);
	vk_create_descriptor_set_layout_ = VK_API_LOAD(vulkan_library_, "vkCreateDescriptorSetLayout", VKCreateDescriptorSetLayout);
	vk_create_pipeline_layout_ = VK_API_LOAD(vulkan_library_, "vkCreatePipelineLayout", VKCreatePipelineLayout);
	vk_create_pipeline_cache_ = VK_API_LOAD(vulkan_library_, "vkCreatePipelineCache", VKCreatePipelineCache);
	vk_create_compute_pipelines_ = VK_API_LOAD(vulkan_library_, "vkCreateComputePipelines", VKCreateComputePipelines);
	vk_create_descriptor_pool_ = VK_API_LOAD(vulkan_library_, "vkCreateDescriptorPool", VKCreateDescriptorPool);
	vk_allocate_descriptor_sets_ = VK_API_LOAD(vulkan_library_, "vkAllocateDescriptorSets", VKAllocateDescriptorSets);
	vk_update_descriptor_sets_ = VK_API_LOAD(vulkan_library_, "vkUpdateDescriptorSets", VKUpdateDescriptorSets);
	vk_begin_command_buffer_ = VK_API_LOAD(vulkan_library_, "vkBeginCommandBuffer", VKBeginCommandBuffer);
	vk_cmd_bind_pipeline_ = VK_API_LOAD(vulkan_library_, "vkCmdBindPipeline", VKCmdBindPipeline);
	vk_cmd_bind_descriptor_sets_ = VK_API_LOAD(vulkan_library_, "vkCmdBindDescriptorSets", VKCmdBindDescriptorSets);
	vk_cmd_dispatch_ = VK_API_LOAD(vulkan_library_, "vkCmdDispatch", VKCmdDispatch);
	vk_cmd_pipeline_barrier_ = VK_API_LOAD(vulkan_library_, "vkCmdPipelineBarrier", VKCmdPipelineBarrier);
	vk_end_command_buffer_ = VK_API_LOAD(vulkan_library_, "vkEndCommandBuffer", VKEndCommandBuffer);
	vk_queue_submit_ = VK_API_LOAD(vulkan_library_, "vkQueueSubmit", VKQueueSubmit);
	vk_queue_wait_idle_ = VK_API_LOAD(vulkan_library_, "vkQueueWaitIdle", VKQueueWaitIdle);
	vk_device_wait_idle_ = VK_API_LOAD(vulkan_library_, "vkDeviceWaitIdle", VKDeviceWaitIdle);
	vk_cmd_copy_buffer_ = VK_API_LOAD(vulkan_library_, "vkCmdCopyBuffer", VKCmdCopyBuffer);
	vk_free_memory_ = VK_API_LOAD(vulkan_library_, "vkFreeMemory", VKFreeMemory);
	vk_destroy_buffer_ = VK_API_LOAD(vulkan_library_, "vkDestroyBuffer", VKDestroyBuffer);
	vk_free_descriptor_sets_ = VK_API_LOAD(vulkan_library_, "vkFreeDescriptorSets", VKFreeDescriptorSets);
	vk_destroy_descriptor_pool_ = VK_API_LOAD(vulkan_library_, "vkDestroyDescriptorPool", VKDestroyDescriptorPool);
	vk_free_command_buffers_ = VK_API_LOAD(vulkan_library_, "vkFreeCommandBuffers", VKFreeCommandBuffers);
	vk_destroy_command_pool_ = VK_API_LOAD(vulkan_library_, "vkDestroyCommandPool", VKDestroyCommandPool);
	vk_destroy_device_ = VK_API_LOAD(vulkan_library_, "vkDestroyDevice", VKDestroyDevice);
	vk_destroy_instance_ = VK_API_LOAD(vulkan_library_, "vkDestroyInstance", VKDestroyInstance);
	vk_destroy_descriptor_set_layout_ = VK_API_LOAD(vulkan_library_, "vkDestroyDescriptorSetLayout", VKDestroyDescriptorSetLayout);
	vk_destroy_pipeline_layout_ = VK_API_LOAD(vulkan_library_, "vkDestroyPipelineLayout", VKDestroyPipelineLayout);
	vk_destroy_shader_module_ = VK_API_LOAD(vulkan_library_, "vkDestroyShaderModule", VKDestroyShaderModule);
	vk_destroy_pipeline_cache_ = VK_API_LOAD(vulkan_library_, "vkDestroyPipelineCache", VKDestroyPipelineCache);
	vk_destroy_pipeline_ = VK_API_LOAD(vulkan_library_, "vkDestroyPipeline", VKDestroyPipeline);
	vk_enumerate_device_layer_properties_ = VK_API_LOAD(vulkan_library_, "vkEnumerateDeviceLayerProperties", VKEnumerateDeviceLayerProperties);
	vk_enumerate_device_extension_properties_ = VK_API_LOAD(vulkan_library_, "vkEnumerateDeviceExtensionProperties", VKEnumerateDeviceExtensionProperties);
	vk_enumerate_instance_layer_properties_ = VK_API_LOAD(vulkan_library_, "vkEnumerateInstanceLayerProperties", VKEnumerateInstanceLayerProperties);
	vk_enumerate_instance_extension_properties_ = VK_API_LOAD(vulkan_library_, "vkEnumerateInstanceExtensionProperties", VKEnumerateInstanceExtensionProperties);
	vkcLibraryLoaded_ = true;
	VKCLOG(INFO) << "vkcLibraryLoaded_ : " << vkcLibraryLoaded_;
}

bool VKCApi::setSharedMemory(base::SharedMemoryHandle dataHandle, base::SharedMemoryHandle operationHandle, base::SharedMemoryHandle resultHandle)
{
	VKCLOG(INFO) << "setSharedMemory";

	bool result = true;

	mSharedData.reset(new base::SharedMemory(dataHandle, false));
	result &= mSharedData->Map(1024*1024*20);
	mSharedDataPtr = mSharedData->memory();
	result &= mSharedDataPtr?true:false;

	mSharedOperation.reset(new base::SharedMemory(operationHandle, false));
	result &= mSharedOperation->Map(sizeof(BaseVKCOperationData));
	mSharedOperationPtr = static_cast<BaseVKCOperationData*>(mSharedOperation->memory());
	result &= mSharedOperationPtr?true:false;

	mSharedResult.reset(new base::SharedMemory(resultHandle, false));
	result &= mSharedResult->Map(sizeof(BaseVKCResultData));
	mSharedResultPtr = static_cast<BaseVKCResultData*>(mSharedResult->memory());
	result &= mSharedResultPtr?true:false;

	VKCLOG(INFO) << ">> result=" << result;

	return result;
}

bool VKCApi::clearSharedMemory()
{
	VKCLOG(INFO) << "cleartSharedMemory";

	bool result = true;

	result &= mSharedData->Unmap();
	result &= mSharedOperation->Unmap();
	result &= mSharedResult->Unmap();

	mSharedData->Close();
	mSharedOperation->Close();
	mSharedOperation->Close();

	mSharedData.reset(nullptr);
	mSharedOperation.reset(nullptr);
	mSharedResult.reset(nullptr);

	VKCLOG(INFO) << ">> result=" << result;

	return result;
}

int VKCApi::vkcCreateInstance(const std::vector<std::string>& names, const std::vector<uint32_t>& versions,
	const std::vector<std::string>& enabledLayerNames, const std::vector<std::string>& enabledExtensionNames, VKCPoint* vkcInstance) {
	VKCLOG(INFO) << "createInstance";
	if (names.size() < 2 || versions.size() < 3) {
		return VK_INCOMPLETE;
	}
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = names[0].c_str();
	app_info.pEngineName = names[1].c_str();
	app_info.applicationVersion = versions[0];
	app_info.engineVersion = versions[1];
	app_info.apiVersion = versions[2];
	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.pApplicationInfo = &app_info;

	inst_info.enabledLayerCount = enabledLayerNames.size();
	std::vector<const char*> layerNames;
	for(unsigned i = 0; i < enabledLayerNames.size(); i++) {
		layerNames.push_back(enabledLayerNames[i].data());
	}
	inst_info.ppEnabledLayerNames = layerNames.data();

	inst_info.enabledExtensionCount = enabledExtensionNames.size();
	std::vector<const char*> extensionNames;
	for(unsigned i = 0; i < enabledExtensionNames.size(); i++) {
		extensionNames.push_back(enabledExtensionNames[i].c_str());
	}
	inst_info.ppEnabledExtensionNames = extensionNames.data();

	VkResult err;
	VkInstance* instance = new VkInstance();;
	err = doVKCreateInstance(&inst_info, NULL, instance);
	VKCLOG(INFO) << "vkCreateInstance err : " << err << ", " << (VKCPoint)instance;
	if (err != VK_SUCCESS) {
		delete instance;
		instance = nullptr;
	}
	*vkcInstance = (VKCPoint)instance;

	uint32_t layerCount = 0;
	doVKEnumerateInstanceLayerProperties(&layerCount, NULL);
	VkLayerProperties* layers = new VkLayerProperties[layerCount];
	doVKEnumerateInstanceLayerProperties(&layerCount, layers);
	VKCLOG(INFO) << "layers " << layerCount;
	for(uint32_t i = 0; i < layerCount; i++) {
		VKCLOG(INFO) << "layers " << i << " : " << layers[i].layerName << ", " << layers[i].specVersion << ", " << layers[i].implementationVersion << ", " << layers[i].description;
		uint32_t extensionCount = 0;
		doVKEnumerateInstanceExtensionProperties(layers[i].layerName, &extensionCount, NULL);
		VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
		doVKEnumerateInstanceExtensionProperties(layers[i].layerName, &extensionCount, extensions);
		VKCLOG(INFO) << "extensions " << extensionCount;
		for(uint32_t i = 0; i < extensionCount; i++) {
			VKCLOG(INFO) << i << " : " << extensions[i].extensionName << ", " << extensions[i].specVersion;
		}
	}


	return (int)err;
}

int VKCApi::vkcDestroyInstance(VKCPoint vkcInstance) {
	VKCLOG(INFO) << "destroyInstance";

	doVKDestroyInstance(*((VkInstance*)vkcInstance), NULL);
	if (vkcInstance != 0) {
		delete ((VkInstance*)vkcInstance);
	}
	return (int)VK_SUCCESS;
}

int VKCApi::vkcEnumeratePhysicalDevice(VKCPoint vkcInstance, VKCuint* physicalDeviceCount, VKCPoint* physicalDeviceList) {
	VKCLOG(INFO) << "physicalDeviceList";

	uint32_t gpu_count = 0;
	VKCResult result = doVKEnumeratePhysicalDevices(*((VkInstance*)vkcInstance), &gpu_count, NULL);
	*physicalDeviceCount = gpu_count;
	if (result != VK_SUCCESS) {
		return (int)result;
	} else if(physicalDeviceCount <= 0) {
		return (int)VK_ERROR_INITIALIZATION_FAILED;
	}

	VkPhysicalDevice* pDevice = new VkPhysicalDevice[gpu_count];
	result = doVKEnumeratePhysicalDevices(*((VkInstance*)vkcInstance), &gpu_count, pDevice);

	if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
		delete[] pDevice;
		pDevice = nullptr;
	}

	*physicalDeviceList = (VKCPoint)pDevice;

	uint32_t layerCount = 0;
	doVKEnumerateDeviceLayerProperties(*pDevice, &layerCount, NULL);
	VkLayerProperties* layers = new VkLayerProperties[layerCount];
	doVKEnumerateDeviceLayerProperties(*pDevice, &layerCount, layers);
	VKCLOG(INFO) << "layers " << layerCount;
	for(uint32_t i = 0; i < layerCount; i++) {
		VKCLOG(INFO) << "layers " << i << " : " << layers[i].layerName << ", " << layers[i].specVersion << ", " << layers[i].implementationVersion << ", " << layers[i].description;
		uint32_t extensionCount = 0;
		doVKEnumerateDeviceExtensionProperties(*pDevice, layers[i].layerName, &extensionCount, NULL);
		VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
		doVKEnumerateDeviceExtensionProperties(*pDevice, layers[i].layerName, &extensionCount, extensions);
		VKCLOG(INFO) << "extensions " << extensionCount;
		for(uint32_t i = 0; i < extensionCount; i++) {
			VKCLOG(INFO) << i << " : " << extensions[i].extensionName << ", " << extensions[i].specVersion;
		}
	}

	return (int)result;
}

int VKCApi::vkcDestroyPhysicalDevice(VKCPoint physicalDeviceList) {
	VKCLOG(INFO) << "destroy PhysicalDeviceList";

	VkPhysicalDevice* pDevice = (VkPhysicalDevice*)physicalDeviceList;
	if (pDevice != nullptr)
		delete[] pDevice;

	return (int)VK_SUCCESS;
}

int VKCApi::vkcCreateDevice(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, VKCPoint* vkcDevice, VKCPoint* vkcQueue) {
	VKCLOG(INFO) << "createDevice " << vdIndex << ", " << physicalDeviceList;

	VkPhysicalDevice* pDevice = (VkPhysicalDevice*)physicalDeviceList;

	VkPhysicalDeviceProperties properties = {};
	doVKGetPhysicalDeviceProperties(pDevice[vdIndex], &properties);

	VKCLOG(INFO) << "getPhysicalDeviceProperties";
	VKCLOG(INFO) << "apiVersion " << properties.apiVersion;
	VKCLOG(INFO) << "driverVersion " << properties.driverVersion;
	VKCLOG(INFO) << "vendorID " << properties.vendorID;
	VKCLOG(INFO) << "deviceID " << properties.deviceID;
	VKCLOG(INFO) << "deviceType " << properties.deviceType;
	VKCLOG(INFO) << "deviceName : " << properties.deviceName;

	uint32_t queueFamilyCount = 0;
	doVKGetPhysicalDeviceQueueFamilyProperties(pDevice[vdIndex], &queueFamilyCount, NULL);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties.resize(queueFamilyCount);
	doVKGetPhysicalDeviceQueueFamilyProperties(pDevice[vdIndex], &queueFamilyCount, queueFamilyProperties.data());
	VKCLOG(INFO) << "vkGetPhysicalDeviceQueueFamilyProperties " << queueFamilyCount << ", " << queueFamilyProperties.size();

	uint32_t queueIndex = 0;
	for(uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			queueIndex = i;
			break;
		}
	}
	if (queueIndex > queueFamilyCount) {
		VKCLOG(INFO) << "vkcCreateDevice error queueIndex : " << queueIndex;
		return (int)VK_ERROR_INITIALIZATION_FAILED;
	}

	std::array<float, 1> queuePriorities = { {1.0f} };

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queuePriorities.data();

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = NULL;

	VkDevice* device = new VkDevice();

	VKCResult result = doVKCreateDevice(pDevice[vdIndex], &deviceCreateInfo, NULL, device);

	if (result != VKCResult::VK_SUCCESS) {
		delete device;
		device = nullptr;
	}
	*vkcDevice = (VKCPoint)device;

	VkQueue* queue = new VkQueue();

	doVKGetDeviceQueue(*device, queueIndex, 0, queue);

	*vkcQueue = (VKCPoint)queue;

	VKCLOG(INFO) << "doVKCCreateDevice : " << result << ", " << *vkcDevice;
	return (int)result;
}

int VKCApi::vkcDestroyDevice(VKCPoint vkcDevice, VKCPoint vkcQueue) {
	VKCLOG(INFO) << "DestroyDevice : " << vkcDevice;

	doVKDestroyDevice(*((VkDevice*)vkcDevice), NULL);
	if (vkcDevice != 0) {
		delete ((VkDevice*)vkcDevice);
	}
	if (vkcQueue != 0) {
		delete ((VkQueue*)vkcQueue);
	}
	return (int)VK_SUCCESS;
}

int VKCApi::vkcGetDeviceInfo(const VKCuint& vdIndex, const VKCPoint& physicalDeviceList, const VKCuint& name, void* data) {
	VKCLOG(INFO) << "getDeviceInfo : " << vdIndex << ", " << physicalDeviceList << ", " << name;
	VkPhysicalDevice* pDevice = (VkPhysicalDevice*)physicalDeviceList;

	VkPhysicalDeviceProperties properties = {};

	doVKGetPhysicalDeviceProperties(pDevice[vdIndex], &properties);


	switch(name) {
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_apiVersion: {
			VKCuint* data_uint = (VKCuint*)data;
			*data_uint = properties.apiVersion;
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_driverVersion: {
			VKCuint* data_uint = (VKCuint*)data;
			*data_uint = properties.driverVersion;
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_vendorID: {
			VKCuint* data_uint = (VKCuint*)data;
			*data_uint = properties.vendorID;
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_deviceID: {
			VKCuint* data_uint = (VKCuint*)data;
			*data_uint = properties.deviceID;
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_deviceType: {
			VKCuint* data_uint = (VKCuint*)data;
			*data_uint = properties.deviceType;
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_deviceName: {
			std::string* data_string = (std::string*)data;
			*data_string = std::string(properties.deviceName);
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_maxMemoryAllocationCount: {
			VKCuint* data_uint = (VKCuint*)data;
			*data_uint = properties.limits.maxMemoryAllocationCount;
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_maxComputeWorkGroupCount: {
			std::vector<VKCuint>* data_array = (std::vector<VKCuint>*)data;
			for(int i = 0; i < 3; i++) {
				data_array->push_back(properties.limits.maxComputeWorkGroupCount[i]);
			}
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_maxComputeWorkGroupInvocations: {
			VKCuint* data_uint = (VKCuint*)data;
			*data_uint = properties.limits.maxComputeWorkGroupInvocations;
			return (int)VK_SUCCESS;
			break;
		}
		case VULKAN_DEVICE_GETINFO_NAME_TABLE::VKC_maxComputeWorkGroupSize: {
			std::vector<VKCuint>* data_array = (std::vector<VKCuint>*)data;
			for(int i = 0; i < 3; i++) {
				data_array->push_back(properties.limits.maxComputeWorkGroupSize[i]);
			}
			return (int)VK_SUCCESS;
			break;
		}
	}
	return (int)VKC_ARGUMENT_NOT_VALID;
}

int VKCApi::vkcCreateBuffer(const VKCPoint& vkcDevice, const VKCPoint& physicalDeviceList, const VKCuint& vdIndex, const VKCuint& sizeInBytes, VKCPoint* vkcBuffer, VKCPoint* vkcMemory) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkPhysicalDevice* pDevice = (VkPhysicalDevice*)physicalDeviceList;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.flags = 0;
	bufferInfo.size = sizeInBytes;

	VkBuffer* buffer = new VkBuffer();

	VkResult result = doVKCreateBuffer(*device, &bufferInfo, nullptr, buffer);

	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "doCreateBuffer : " << result;
		delete buffer;
		*vkcBuffer = 0;
		*vkcMemory = 0;
		return (int)result;
	}

	VkMemoryRequirements memoryRequirements = {};
	doVKGetBufferMemoryRequirements(*device, *buffer, &memoryRequirements);

	VKCLOG(INFO) << "buffer size match " << memoryRequirements.size << ", " << sizeInBytes;

	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	doVKGetPhysicalDeviceMemoryProperties(pDevice[vdIndex], &memoryProperties);

	uint32_t typeIndex = 0;
	uint32_t typeBits = memoryRequirements.memoryTypeBits;
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				typeIndex = i;
				break;
			}
		}
		typeBits >>= 1;
	}

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = sizeInBytes;
	allocateInfo.memoryTypeIndex = typeIndex;

	VkDeviceMemory* deviceMemory = new VkDeviceMemory();
	result = doVKAllocateMemory(*device, &allocateInfo, nullptr, deviceMemory);

	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "doVKAllocateMemory : " << result;
		doVKDestroyBuffer(*device, *buffer, nullptr);
		delete buffer;
		delete deviceMemory;
		*vkcBuffer = 0;
		*vkcMemory = 0;
		return (int)result;
	}

	result = doVKBindBufferMemory(*device, *buffer, *deviceMemory, 0);

	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "doVKBindBufferMemory : " << result;
		doVKDestroyBuffer(*device, *buffer, nullptr);
		doVKFreeMemory(*device, *deviceMemory, nullptr);
		delete buffer;
		delete deviceMemory;
		*vkcBuffer = 0;
		*vkcMemory = 0;
		return (int)result;
	}

	*vkcBuffer = (VKCPoint)buffer;
	*vkcMemory = (VKCPoint)deviceMemory;

	return (int)result;
}

int VKCApi::vkcReleaseBuffer(const VKCPoint& vkcDevice, const VKCPoint& vkcBuffer, const VKCPoint& vkcMemory) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkBuffer* buffer = (VkBuffer*)vkcBuffer;
	VkDeviceMemory* memory = (VkDeviceMemory*)vkcMemory;

	doVKUnmapMemory(*device, *memory);
	doVKFreeMemory(*device, *memory, nullptr);
	doVKDestroyBuffer(*device, *buffer, nullptr);

	return (int)VK_SUCCESS;
}

int VKCApi::vkcFillBuffer(const VKCPoint& vkcDevice, const VKCPoint& vkcMemory, const std::vector<VKCuint>& uintVector) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkDeviceMemory* memory = (VkDeviceMemory*)vkcMemory;
	VkDeviceSize offset = uintVector[0];
	VKCuint data = uintVector[1];
	VkDeviceSize size = uintVector[2];

	void* deviceMemoryPtr = nullptr;
	unsigned dataSize = (size - offset) / sizeof(VKCuint);

	uint32_t* dataPtr = new uint32_t[dataSize];

	for(unsigned i = 0; i < dataSize; i++) {
		dataPtr[i] = data;
	}

	VKCResult result = doVKMapMemory(*device, *memory, offset, VK_WHOLE_SIZE, 0, &deviceMemoryPtr);

	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "memory mapping fail : " << result;
		return (int)result;
	}

	memcpy(deviceMemoryPtr, dataPtr, (size - offset));

	doVKUnmapMemory(*device, *memory);

	VKCLOG(INFO) << "fillBuffer complete " << device << ", " << memory << ", " << offset << ", " << data << ", " << size;

	delete[] dataPtr;

	return (int)result;
}

void VKCApi::vkcReadBuffer() {
	VkResult result;
	void* dataPtr;
	unsigned index = mSharedOperationPtr->uint_01;
	unsigned size = mSharedOperationPtr->uint_02;
	VkDevice* device = (VkDevice*)mSharedOperationPtr->point_01;
	VkDeviceMemory* memory = (VkDeviceMemory*)mSharedOperationPtr->point_02;

	result = doVKMapMemory(*device, *memory, index, VK_WHOLE_SIZE, 0, &dataPtr);

	if (result != VK_SUCCESS) {
		mSharedResultPtr->result_01 = result;
		return;
	}

	memcpy(mSharedDataPtr, dataPtr, size);

	doVKUnmapMemory(*device, *memory);

	mSharedResultPtr->result_01 = VK_SUCCESS;

	VKCLOG(INFO) << ">>result= "<< mSharedResultPtr->result_01 << ", " << index << ", " << size;
}

void VKCApi::vkcWriteBuffer() {
	VkResult result;
	void* dataPtr;
	unsigned index = mSharedOperationPtr->uint_01;
	unsigned size = mSharedOperationPtr->uint_02;
	VkDevice* device = (VkDevice*)mSharedOperationPtr->point_01;
	VkDeviceMemory* memory = (VkDeviceMemory*)mSharedOperationPtr->point_02;

	result = doVKMapMemory(*device, *memory, index, VK_WHOLE_SIZE, 0, &dataPtr);

	if (result != VK_SUCCESS) {
		mSharedResultPtr->result_01 = result;
		return;
	}

	memcpy(dataPtr, mSharedDataPtr, size);

	doVKUnmapMemory(*device, *memory);

	mSharedResultPtr->result_01 = VK_SUCCESS;

	VKCLOG(INFO) << ">>result= "<< mSharedResultPtr->result_01 << ", " << index << ", " << size;
}

int VKCApi::vkcCreateCommandQueue(const VKCPoint& vkcDevice, const VKCPoint& physicalDeviceList, const VKCuint& vdIndex, VKCPoint* vkcCMDBuffer, VKCPoint* vkcCMDPool) {
	VkResult result;
	VkDevice* device = (VkDevice*)vkcDevice;
	VkPhysicalDevice* pDevice = (VkPhysicalDevice*)physicalDeviceList;

	uint32_t queueFamilyCount = 0;
	doVKGetPhysicalDeviceQueueFamilyProperties(pDevice[vdIndex], &queueFamilyCount, NULL);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties.resize(queueFamilyCount);
	doVKGetPhysicalDeviceQueueFamilyProperties(pDevice[vdIndex], &queueFamilyCount, queueFamilyProperties.data());
	VKCLOG(INFO) << "vkGetPhysicalDeviceQueueFamilyProperties " << queueFamilyCount << ", " << queueFamilyProperties.size();

	uint32_t queueIndex = 0;
	for(uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			queueIndex = i;
			break;
		}
	}
	if (queueIndex > queueFamilyCount) {
		VKCLOG(INFO) << "vkcCreateDevice error queueIndex : " << queueIndex;
		return (int)VK_ERROR_INITIALIZATION_FAILED;
	}

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkCommandPool* cmdPool = new VkCommandPool();

	result = doVKCreateCommandPool(*device, &cmdPoolInfo, nullptr, cmdPool);

	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "createCommandPool fail : " << result;
		delete cmdPool;
		*vkcCMDPool = 0;
		return (int)result;
	}

	*vkcCMDPool = (VKCPoint)cmdPool;

	VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {};
	cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocateInfo.commandPool = *cmdPool;
	cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer* cmdBuffer = new VkCommandBuffer();

	result = doVKAllocateCommandBuffers(*device, &cmdBufferAllocateInfo, cmdBuffer);

	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "vkAllocateCommandBuffers fail : " << result;
		doVKDestroyCommandPool(*device, *cmdPool, nullptr);
		delete cmdPool;
		*vkcCMDPool = 0;
		delete cmdBuffer;
		*vkcCMDBuffer = 0;
		return (int)result;
	}

	*vkcCMDBuffer = (VKCPoint)cmdBuffer;

	return (int)result;
}

int VKCApi::vkcReleaseCommandQueue(const VKCPoint& vkcDevice, const VKCPoint& vkcCMDBuffer, const VKCPoint& vkcCMDPool) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkCommandPool* cmdPool = (VkCommandPool*)vkcCMDPool;
	VkCommandBuffer* cmdBuffer = (VkCommandBuffer*)vkcCMDBuffer;

	doVKFreeCommandBuffers(*device, *cmdPool, 1, cmdBuffer);
	doVKDestroyCommandPool(*device, *cmdPool, nullptr);

	return (int)VK_SUCCESS;
}

int VKCApi::vkcCreateDescriptorSetLayout(const VKCPoint& vkcDevice, const VKCuint& useBufferCount, VKCPoint* vkcDescriptorSetLayout) {
	VkDevice* device = (VkDevice*)vkcDevice;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for(VKCuint i = 0; i < useBufferCount; i++) {
		VkDescriptorSetLayoutBinding layout = {};
		layout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layout.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layout.binding = i;
		layout.descriptorCount = 1;
		layout.pImmutableSamplers = NULL;
		bindings.push_back(layout);
	}

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = NULL;
	layoutCreateInfo.pBindings = bindings.data();
	layoutCreateInfo.bindingCount = bindings.size();

	VkDescriptorSetLayout* descriptorLayout = new VkDescriptorSetLayout();

	VkResult result = doVKCreateDescriptorSetLayout(*device, &layoutCreateInfo, nullptr, descriptorLayout);
	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "createDescriptorSetLayout fail : " << result;
		delete descriptorLayout;
		*vkcDescriptorSetLayout = 0;
		return (int)result;
	}

	*vkcDescriptorSetLayout = (VKCPoint)descriptorLayout;
	return (int)result;
}

int VKCApi::vkcReleaseDescriptorSetLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSetLayout) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkDescriptorSetLayout* descriptorLayout = (VkDescriptorSetLayout*)vkcDescriptorSetLayout;

	doVKDestroyDescriptorSetLayout(*device, *descriptorLayout, nullptr);

	delete descriptorLayout;
	return (int)VK_SUCCESS;
}

int VKCApi::vkcCreateDescriptorPool(const VKCPoint& vkcDevice, const VKCuint& useBufferCount, VKCPoint* vkcDescriptorPool) {
	VkDevice* device = (VkDevice*)vkcDevice;

	std::vector<VkDescriptorPoolSize> poolSize;

	VkDescriptorPoolSize size = {};
	size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	size.descriptorCount = useBufferCount;
	poolSize.push_back(size);

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.pNext = NULL;
	poolCreateInfo.poolSizeCount = poolSize.size();
	poolCreateInfo.pPoolSizes = poolSize.data();
	poolCreateInfo.maxSets = 1;

	VkDescriptorPool* pool = new VkDescriptorPool();

	VkResult result = doVKCreateDescriptorPool(*device, &poolCreateInfo, nullptr, pool);

	if (result != VK_SUCCESS) {
		delete pool;
		*vkcDescriptorPool = 0;
		return (int)result;
	}

	*vkcDescriptorPool = (VKCPoint)pool;
	return result;
}

int VKCApi::vkcReleaseDescriptorPool(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkDescriptorPool* pool = (VkDescriptorPool*)vkcDescriptorPool;

	doVKDestroyDescriptorPool(*device, *pool, nullptr);
	delete pool;
	return (int)VK_SUCCESS;
}

int VKCApi::vkcCreateDescriptorSet(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, const VKCPoint& vkcDescriptorSetLayout, VKCPoint* vkcDescriptorSet) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkDescriptorPool* pool = (VkDescriptorPool*)vkcDescriptorPool;
	VkDescriptorSetLayout* layout = (VkDescriptorSetLayout*)vkcDescriptorSetLayout;

	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.pNext = NULL;
	allocateInfo.descriptorPool = *pool;
	allocateInfo.pSetLayouts = layout;
	allocateInfo.descriptorSetCount = 1;
	VkDescriptorSet* descriptorSet = new VkDescriptorSet();

	VkResult result = doVKAllocateDescriptorSets(*device, &allocateInfo, descriptorSet);

	if (result == VK_SUCCESS) {
		*vkcDescriptorSet = (VKCPoint)descriptorSet;
	}

	return (int)result;
}

int VKCApi::vkcReleaseDescriptorSet(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorPool, const VKCPoint& vkcDescriptorSet) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkDescriptorPool* pool = (VkDescriptorPool*)vkcDescriptorPool;
	VkDescriptorSet* sets = (VkDescriptorSet*)vkcDescriptorSet;

	VkResult result = doVKFreeDescriptorSets(*device, *pool, 1, sets);

	delete sets;
	return (int)result;
}

int VKCApi::vkcCreatePipelineLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSetLayout, VKCPoint* vkcPipelineLayout) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkDescriptorSetLayout* layout = (VkDescriptorSetLayout*)vkcDescriptorSetLayout;

	VkPipelineLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.setLayoutCount = 1;
	createInfo.pSetLayouts = layout;

	VkPipelineLayout* pipelineLayout = new VkPipelineLayout();

	VkResult result = doVKCreatePipelineLayout(*device, &createInfo, nullptr, pipelineLayout);

	if (result != VK_SUCCESS) {
		delete pipelineLayout;
		pipelineLayout = nullptr;
		*vkcPipelineLayout = 0;
	} else {
		*vkcPipelineLayout = (VKCPoint)pipelineLayout;
	}

	return (int)result;
}

int VKCApi::vkcReleasePipelineLayout(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineLayout) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkPipelineLayout* layout = (VkPipelineLayout*)vkcPipelineLayout;

	doVKDestroyPipelineLayout(*device, *layout, nullptr);

	delete layout;
	return (int)VK_SUCCESS;
}

int VKCApi::vkcCreateShaderModule(const VKCPoint& vkcDevice, const std::string& shaderPath, VKCPoint* vkcShaderModule) {
	VkDevice* device = (VkDevice*)vkcDevice;

	char* shaderCode = (char*)malloc(65535);
	VKCuint codeLength = getShaderCode(shaderPath, shaderCode, 65535);

	if (codeLength == 0) {
		return (int)VKC_GET_SHADER_CODE_FAIL;
	}

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.codeSize = codeLength;
	createInfo.flags = 0;
	createInfo.pCode = (uint32_t*)shaderCode;

	VkShaderModule* shaderModule = new VkShaderModule();

	VkResult result = doVKCreateShaderModule(*device, &createInfo, nullptr, shaderModule);

	if (result != VK_SUCCESS) {
		delete shaderModule;
		*vkcShaderModule = 0;
	} else {
		*vkcShaderModule = (VKCPoint)shaderModule;
	}

	return (int)result;

}

int VKCApi::vkcReleaseShaderModule(const VKCPoint& vkcDevice, const VKCPoint& vkcShaderModule) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkShaderModule* shaderModule = (VkShaderModule*)vkcShaderModule;

	doVKDestroyShaderModule(*device, *shaderModule, nullptr);

	delete shaderModule;
	return (int)VK_SUCCESS;
}

int VKCApi::vkcCreatePipeline(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineLayout, const VKCPoint& vkcShaderModule, VKCPoint* vkcPipelineCache, VKCPoint* vkcPipeline) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkPipelineLayout* layout = (VkPipelineLayout*)vkcPipelineLayout;
	VkShaderModule* module = (VkShaderModule*)vkcShaderModule;
	VkPipelineCache* cache = new VkPipelineCache();
	VkPipeline* pipeline = new VkPipeline();
	VKCResult result;

	VkPipelineCacheCreateInfo cacheCreateInfo = {};
	cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	cacheCreateInfo.pNext = NULL;
	cacheCreateInfo.pInitialData = NULL;
	cacheCreateInfo.initialDataSize = 0;
	cacheCreateInfo.flags = 0;

	result = doVKCreatePipelineCache(*device, &cacheCreateInfo, nullptr, cache);
	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "createPipelineCache fail : " << result;
		delete cache;
		delete pipeline;
		*vkcPipelineCache = 0;
		*vkcPipeline = 0;
		return result;
	}

	VkPipelineShaderStageCreateInfo stageInfo = {};
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo.module = *module;
	stageInfo.pName = "main";

	VkComputePipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.layout = *layout;
	createInfo.flags = 0;
	createInfo.stage = stageInfo;

	result = doVKCreateComputePipelines(*device, *cache, 1, &createInfo, nullptr, pipeline);
	if (result != VK_SUCCESS) {
		VKCLOG(INFO) << "createPipeline fail : " << result;
		delete cache;
		delete pipeline;
		*vkcPipelineCache = 0;
		*vkcPipeline = 0;
		return result;
	}

	*vkcPipelineCache = (VKCPoint)cache;
	*vkcPipeline = (VKCPoint)pipeline;

	return result;
}

int VKCApi::vkcReleasePipeline(const VKCPoint& vkcDevice, const VKCPoint& vkcPipelineCache, const VKCPoint& vkcPipeline) {
	VkDevice* device = (VkDevice*)vkcDevice;
	VkPipelineCache* cache = (VkPipelineCache*)vkcPipelineCache;
	VkPipeline* pipeline = (VkPipeline*)vkcPipeline;

	doVKDestroyPipeline(*device, *pipeline, nullptr);
	doVKDestroyPipelineCache(*device, *cache, nullptr);

	delete cache;
	delete pipeline;

	return (int)VK_SUCCESS;
}

int VKCApi::vkcUpdateDescriptorSets(const VKCPoint& vkcDevice, const VKCPoint& vkcDescriptorSet, const std::vector<VKCPoint>& bufferVector) {
	if (bufferVector.size() == 0) {
		return (int)VKC_FAILURE;
	}

	VkDevice* device = (VkDevice*)vkcDevice;
	VkDescriptorSet* descriptorSet = (VkDescriptorSet*)vkcDescriptorSet;

	std::vector<VkDescriptorBufferInfo> bufferInfo;
	for(VKCuint i = 0; i < bufferVector.size(); i++) {
		VkDescriptorBufferInfo info = {};
		info.buffer = *((VkBuffer*)bufferVector[i]);
		info.offset = 0;
		info.range = VK_WHOLE_SIZE;
		bufferInfo.push_back(info);
	}

	VkWriteDescriptorSet writeSet = {};
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.pNext = NULL;
	writeSet.dstSet = *descriptorSet;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeSet.dstBinding = 0;
	writeSet.descriptorCount = bufferVector.size();
	writeSet.pTexelBufferView = NULL;
	writeSet.pBufferInfo = bufferInfo.data();

	doVKUpdateDescriptorSets(*device, 1, &writeSet, 0, nullptr);

	return (int)VK_SUCCESS;
}

int VKCApi::vkcBeginQueue(const VKCPoint& vkcCMDBuffer, const VKCPoint& vkcPipeline, const VKCPoint& vkcPipelineLayout, const VKCPoint& vkcDescriptorSet) {
	VkCommandBuffer* cmdBuffer = (VkCommandBuffer*)vkcCMDBuffer;
	VkPipeline* pipeline = (VkPipeline*)vkcPipeline;
	VkPipelineLayout* pipelineLayout = (VkPipelineLayout*)vkcPipelineLayout;
	VkDescriptorSet* descriptorSet = (VkDescriptorSet*)vkcDescriptorSet;
	VkResult result;

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;

	result = doVKBeginCommandBuffer(*cmdBuffer, &beginInfo);
	if (result != VK_SUCCESS) {
		return result;
	}

	doVKCmdBindPipeline(*cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline);
	doVKCmdBindDescriptorSets(*cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *pipelineLayout, 0, 1, descriptorSet, 0, 0);

	return (int)VK_SUCCESS;
}

int VKCApi::vkcEndQueue(const VKCPoint& vkcCMDBuffer) {
	VkCommandBuffer* cmdBuffer = (VkCommandBuffer*)vkcCMDBuffer;

	VkResult result = doVKEndCommandBuffer(*cmdBuffer);

	return result;
}

int VKCApi::vkcDispatch(const VKCPoint& vkcCMDBuffer, const VKCuint& workGroupX, const VKCuint& workGroupY, const VKCuint& workGroupZ) {
	VkCommandBuffer* cmdBuffer = (VkCommandBuffer*)vkcCMDBuffer;

	doVKCmdDispatch(*cmdBuffer, workGroupX, workGroupY, workGroupZ);

	return (int)VK_SUCCESS;
}

int VKCApi::vkcPipelineBarrier(const VKCPoint& vkcCMDBuffer) {
	VkCommandBuffer* cmdBuffer = (VkCommandBuffer*)vkcCMDBuffer;

	doVKCmdPipelineBarrier(*cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	return (int)VK_SUCCESS;
}

int VKCApi::vkcCmdCopyBuffer(const VKCPoint& vkcCMDBuffer, const VKCPoint& srcBuffer, const VKCPoint& dstBuffer, const VKCuint& copySize) {
	VkCommandBuffer* cmdBuffer = (VkCommandBuffer*)vkcCMDBuffer;
	VkBuffer* src = (VkBuffer*)srcBuffer;
	VkBuffer* dst = (VkBuffer*)dstBuffer;

	VkBufferCopy copyRegion = {};
	copyRegion.size = copySize;

	doVKCmdCopyBuffer(*cmdBuffer, *src, *dst, 1, &copyRegion);

	return (int)VK_SUCCESS;
}

int VKCApi::vkcQueueSubmit(const VKCPoint& vkcQueue, const VKCPoint& vkcCMDBuffer) {
	VkQueue* queue = (VkQueue*)vkcQueue;
	VkCommandBuffer* buffer = (VkCommandBuffer*)vkcCMDBuffer;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = buffer;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.pSignalSemaphores = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.signalSemaphoreCount = 0;

	VkResult result = doVKQueueSubmit(*queue, 1, &submitInfo, VK_NULL_HANDLE);

	return (int)result;
}

int VKCApi::vkcDeviceWaitIdle(const VKCPoint& vkcDevice) {
	VkDevice* device = (VkDevice*)vkcDevice;

	VkResult result = doVKDeviceWaitIdle(*device);

	return (int)result;
}

VKCuint VKCApi::getShaderCode(std::string shaderPath, char* shaderCode, VKCuint maxSourceLength) {
	int sockfd, c;
	struct sockaddr_in addr;
	std::string address;
	std::string protocol;

	size_t protocolIndex = shaderPath.find("://");

	VKCLOG(INFO) << "protocolIndex : " << protocolIndex;

	if (protocolIndex != std::string::npos) {
		address = shaderPath.substr(protocolIndex + 3);
		protocol = shaderPath.substr(0, protocolIndex);
		VKCLOG(INFO) << protocol << ", " << address;
	} else {
		VKCLOG(INFO) << "not found protocol index";
		return 0;
	}

	if (protocol == "http" || protocol == "https") {
		VKCLOG(INFO) << "protocol is http";
		size_t hostNameIndex = address.find("/");
		VKCLOG(INFO) << "hostNameIndex : " << hostNameIndex;
		std::string hostName;
		std::string path;
		if (hostNameIndex != std::string::npos) {
			hostName = address.substr(0, hostNameIndex);
			path = address.substr(hostNameIndex);
		} else {
			hostName = address;
			path = "/";
		}
		VKCLOG(INFO) << "hostName : " << hostName;
		size_t portIndex = hostName.find(":");
		int port = 80;
		if (portIndex == std::string::npos) {
			VKCLOG(INFO) << "hostname not has port number";
		} else {
			VKCLOG(INFO) << "portIndex : " << portIndex << ", port : " << hostName.substr(portIndex + 1);
			port = stoi(hostName.substr(portIndex + 1));
			hostName = hostName.substr(0, portIndex);
		}
		VKCLOG(INFO) << "hostName : " << hostName << ", port : " << port;
		VKCLOG(INFO) << "path : " << path;

		struct hostent *host = gethostbyname(hostName.c_str());

		if (host == nullptr) {
			VKCLOG(INFO) << "wrong url " << h_errno;
			return 0;
		}

		sockfd = socket(PF_INET, SOCK_STREAM, 0);

		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr = *((struct in_addr *)host->h_addr);
		memset(addr.sin_zero, '\0', sizeof addr.sin_zero);

		c = connect(sockfd, (struct sockaddr *)&addr, sizeof addr);

		for(int i = 0; i < 5; i++) {
			if (c != -1) {
				VKCLOG(INFO) << "connect fail " << c << ", " << i;
				break;
			} else if(c == -1 && i == 4) {
				VKCLOG(INFO) << "connect fail " << c << ", " << i;
				return 0;
			}
			sleep(1);
			c = connect(sockfd, (struct sockaddr *)&addr, sizeof addr);
			VKCLOG(INFO) << "connect fail " << c << ", " << i;
			VKCLOG(INFO) << "errno : " << errno;
		}
		VKCLOG(INFO) << "connect";
		std::string packet = "GET " + path + "\r\n\r\n";

		send(sockfd, packet.c_str(), packet.size(), 0);

		int bytes = 1000;

		char buffer[bytes];
		VKCuint index = 0;
		int r = 1;
		while(r > 0) // I'll work on dynamically setting the size later on.
		{
			r = recv(sockfd, buffer, 128, 0);
			if (r <= 0) {
				break;
			}
			memcpy(&shaderCode[index], buffer, r);
			index += r;
			if (index >= maxSourceLength) {
				index = 0;
				break;
			}
		}
		VKCLOG(INFO) << "index : " << index;

		close(sockfd);
		return index;
	} else if(protocol == "file") {
		VKCLOG(INFO) << "protocol is File";
		VKCuint size = 0;

		size_t retval;

		if (address[0] != '/') {
			address = "/" + address;
		}

		FILE *fp = fopen(address.c_str(), "rb");
		if (!fp){
			VKCLOG(INFO) << "fp not open";
			return 0;
		}

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);

		if (maxSourceLength < size) {
			return 0;
		}

		fseek(fp, 0L, SEEK_SET);

		retval = fread(shaderCode, size, 1, fp);
		VKCLOG(INFO) << "readBinaryFile : " << (retval == 1);

		VKCLOG(INFO) << "VkShaderModule loadShader : " << (size > 0);

		return size;
	} else {
		return 0;
	}
}

} // namesapce gfx
