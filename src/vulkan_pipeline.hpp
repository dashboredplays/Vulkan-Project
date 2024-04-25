#pragma once

#include "vulkan_device.hpp"

#include <string>
#include <vector>

namespace lve {

   //struct used to store several related variables in one place 
   struct PipelineConfigInfo {

   };
   class LvePipeline {
      public:
         LvePipeline(
            LveDevice& device, 
            const std::string& vertFilepath, 
            const std::string& fragFilepath, 
            const PipelineConfigInfo& configInfo);
            //distructor for managing lifetime of resources
         ~LvePipeline() {}

         LvePipeline(const LvePipeline&) = delete;
         void operator=(const LvePipeline&) = delete;

         static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

      private:
         static std::vector<char> readFile(const std::string& filepath);

         void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);

         //pointer to a pointer
         void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
         //could be memory unsafe (reference type member variable), but will outlive the LveDevice object
         LveDevice& lveDevice;
         //typedef pointer to a struct
         VkPipeline graphicsPipeline;
         VkShaderModule vertShaderModule;
         VkShaderModule fragShaderModule;
   };
}