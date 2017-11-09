#include "gpu/native_vulkan/vulkan_shader_compiler.h"

#include <memory>

#include <shaderc/shaderc.h>
#include <sstream>

#include "base/logging.h"
#include "base/memory/ptr_util.h"

namespace gpu {

namespace {

//this class copy from gpu/vulkan/vulkan_shader_module.cc for compile compute shader
class HighwebVulkanShaderCompiler {
 public:
  class HighwebVKCCompileResult {
   public:
    explicit HighwebVKCCompileResult(shaderc_compilation_result_t compilation_result)
        : compilation_result_(compilation_result) {}

    ~HighwebVKCCompileResult() { shaderc_result_release(compilation_result_); }

    bool IsValid() const {
      return shaderc_compilation_status_success ==
             shaderc_result_get_compilation_status(compilation_result_);
    }

    std::string GetErrors() const {
      return shaderc_result_get_error_message(compilation_result_);
    }

    std::string GetResult() const {
      return std::string(shaderc_result_get_bytes(compilation_result_),
                         shaderc_result_get_length(compilation_result_));
    }

   private:
    shaderc_compilation_result_t compilation_result_;
  };

  HighwebVulkanShaderCompiler()
      : compiler_(shaderc_compiler_initialize()),
        compiler_options_(shaderc_compile_options_initialize()) {}

  ~HighwebVulkanShaderCompiler() { shaderc_compiler_release(compiler_); }

  void AddMacroDef(const std::string& name, const std::string& value) {
    shaderc_compile_options_add_macro_definition(compiler_options_,
                                                 name.c_str(), name.length(),
                                                 value.c_str(), value.length());
  }

  std::unique_ptr<HighwebVulkanShaderCompiler::HighwebVKCCompileResult> CompileShaderModule(
      gpu::VulkanShaderCompiler::ShaderType shader_type,
      const std::string& name,
      const std::string& entry_point,
      const std::string& source) {
        shaderc_shader_kind type = shaderc_glsl_vertex_shader;
        switch(shader_type) {
          case gpu::VulkanShaderCompiler::ShaderType::VERTEX: {
            type = shaderc_glsl_vertex_shader;
            break;
          }
          case gpu::VulkanShaderCompiler::ShaderType::FRAGMENT: {
            type = shaderc_glsl_fragment_shader;
            break;
          }
          case gpu::VulkanShaderCompiler::ShaderType::COMPUTE: {
            type = shaderc_glsl_compute_shader;
            break;
          }
        }
    return base::MakeUnique<HighwebVulkanShaderCompiler::HighwebVKCCompileResult>(
        shaderc_compile_into_spv(
            compiler_, source.c_str(), source.length(),
            type,
            name.c_str(), entry_point.c_str(), compiler_options_));
  }

 private:
  shaderc_compiler_t compiler_;
  shaderc_compile_options_t compiler_options_;
};

}  // namespace

VulkanShaderCompiler::VulkanShaderCompiler() {
}

VulkanShaderCompiler::~VulkanShaderCompiler() {
}

std::string VulkanShaderCompiler::InitializeGLSL(ShaderType type,
                                        std::string name,
                                        std::string entry_point,
                                        std::string source) {
  HighwebVulkanShaderCompiler shaderc_compiler;
  std::unique_ptr<HighwebVulkanShaderCompiler::HighwebVKCCompileResult> compilation_result(
      shaderc_compiler.CompileShaderModule(type, name, entry_point, source));

  if (!compilation_result->IsValid()) {
    // error_messages_ = compilation_result->GetErrors();
    LOG(ERROR) << "VulkanShaderCompiler error : " << compilation_result->GetErrors();
    return "";
  }

  return InitializeSPIRVSource(type, compilation_result->GetResult());
}

std::string VulkanShaderCompiler::InitializeSPIRVSource(ShaderType type,
                                         std::string source) {
  // Make sure source is a multiple of 4.
  const int padding = 4 - (source.length() % 4);
  if (padding < 4) {
    for (int i = 0; i < padding; ++i) {
      source += ' ';
    }
  }

  return source;
}

}  // namespace gpu
