// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Copyright (c) 2017 Selvas AI, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_NATIVE_VULKAN_VULKAN_SHADER_COMPILER_H_
#define GPU_NATIVE_VULKAN_VULKAN_SHADER_COMPILER_H_

#include <string>
#include <vulkan/vulkan.h>

#include "base/macros.h"

namespace gpu {

class VulkanShaderCompiler {
 public:
  enum class ShaderType {
    VERTEX,
    FRAGMENT,
    COMPUTE,
  };

  VulkanShaderCompiler();
  ~VulkanShaderCompiler();

  std::string InitializeGLSL(ShaderType type,
                      std::string name,
                      std::string entry_point,
                      std::string source);
 private:
  std::string InitializeSPIRVSource(ShaderType type,
                       std::string source);
  DISALLOW_COPY_AND_ASSIGN(VulkanShaderCompiler);
};

}  // namespace gpu

#endif  // GPU_NATIVE_VULKAN_VULKAN_SHADER_COMPILER_H_
