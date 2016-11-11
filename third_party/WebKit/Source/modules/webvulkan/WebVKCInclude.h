// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/native_vulkan/vulkan_include.h"

#include "base/memory/shared_memory.h"

namespace gpu {
	class GpuChannelHost;

	//SHM control functions
	bool webvkc_SetSharedHandles(GpuChannelHost*, base::SharedMemoryHandle, base::SharedMemoryHandle, base::SharedMemoryHandle);
	bool webvkc_ClearSharedHandles(GpuChannelHost*);
	bool webvkc_TriggerSharedOperation(GpuChannelHost*, int operation);

	//WebVKC
	VKCResult webvkc_createInstance(GpuChannelHost*, std::string& applicationName, std::string& engineName,
		uint32_t& applicationVersion, uint32_t& engineVersion, uint32_t& apiVersion,
		std::vector<std::string>& enabledLayerNames, std::vector<std::string>& enabledExtensionNames, VKCPoint* vkcInstance);
	VKCResult webvkc_destroyInstance(GpuChannelHost*, VKCPoint*);
	VKCResult webvkc_enumeratePhysicalDeviceCount(GpuChannelHost*, VKCPoint*, VKCuint*, VKCPoint*);
	VKCResult webvkc_destroyPhysicalDevice(GpuChannelHost*, VKCPoint*);
	VKCResult webvkc_createDevice(GpuChannelHost*, VKCuint&, VKCPoint&, VKCPoint*, VKCPoint*);
	VKCResult webvkc_destroyDevice(GpuChannelHost*, VKCPoint*, VKCPoint*);
	VKCResult webvkc_getDeviceInfo(GpuChannelHost*, VKCuint&, VKCPoint&, VKCenum&, void*);
	VKCResult webvkc_createBuffer(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCuint&, VKCuint&, VKCPoint*, VKCPoint*);
	VKCResult webvkc_releaseBuffer(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCPoint&);
	VKCResult webvkc_fillBuffer(GpuChannelHost*, VKCPoint&, VKCPoint&, std::vector<VKCuint>&);
	VKCResult webvkc_createCommandQueue(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCuint&, VKCPoint*, VKCPoint*);
	VKCResult webvkc_releaseCommandQueue(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCPoint&);
	VKCResult webvkc_createDescriptorSetLayout(GpuChannelHost*, VKCPoint&, VKCuint&, VKCPoint*);
	VKCResult webvkc_releaseDescriptorSetLayout(GpuChannelHost*, VKCPoint&, VKCPoint&);
	VKCResult webvkc_createDescriptorPool(GpuChannelHost*, VKCPoint&, VKCuint&, VKCPoint*);
	VKCResult webvkc_releaseDescriptorPool(GpuChannelHost*, VKCPoint&, VKCPoint&);
	VKCResult webvkc_createDescriptorSet(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCPoint&, VKCPoint*);
	VKCResult webvkc_releaseDescriptorSet(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCPoint&);
	VKCResult webvkc_createPipelineLayout(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCPoint*);
	VKCResult webvkc_releasePipelineLayout(GpuChannelHost*, VKCPoint&, VKCPoint&);
	VKCResult webvkc_createShaderModule(GpuChannelHost*, VKCPoint&, std::string&, VKCPoint*);
	VKCResult webvkc_releaseShaderModule(GpuChannelHost*, VKCPoint&, VKCPoint&);
	VKCResult webvkc_createPipeline(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCPoint&, VKCPoint*, VKCPoint*);
	VKCResult webvkc_releasePipeline(GpuChannelHost*, VKCPoint&, VKCPoint&, VKCPoint&);
	VKCResult webvkc_updateDescriptorSets(GpuChannelHost*, VKCPoint&, VKCPoint&, std::vector<VKCPoint>&);
	VKCResult webvkc_beginQueue(GpuChannelHost*, VKCPoint&, VKCPoint, VKCPoint, VKCPoint);
	VKCResult webvkc_endQueue(GpuChannelHost*, VKCPoint&);
	VKCResult webvkc_dispatch(GpuChannelHost*, VKCPoint&, VKCuint&, VKCuint&, VKCuint&);
	VKCResult webvkc_pipelineBarrier(GpuChannelHost*, VKCPoint&);
	VKCResult webvkc_cmdCopyBuffer(GpuChannelHost*, VKCPoint&, VKCPoint, VKCPoint, VKCuint&);
	VKCResult webvkc_queueSubmit(GpuChannelHost*, VKCPoint&, VKCPoint);
	VKCResult webvkc_deviceWaitIdle(GpuChannelHost*, VKCPoint&);
}
