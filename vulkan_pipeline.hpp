#pragma once

#include "vulkan_device.hpp"

#include <string>
#include <vector>

namespace lve {

   //structs are used to store several related variables in one place 
   struct PipelineConfigInfo {
      PipelineConfigInfo(const PipelineConfigInfo&) = delete;
      PipelineConfigInfo() = default;
      PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
      VkViewport viewport;
      VkRect2D scissor;
      VkPipelineViewportStateCreateInfo viewportInfo;
      VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
      VkPipelineRasterizationStateCreateInfo rasterizationInfo;
      VkPipelineMultisampleStateCreateInfo multisampleInfo;
      VkPipelineColorBlendAttachmentState colorBlendAttachment;
      VkPipelineColorBlendStateCreateInfo colorBlendInfo;
      VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
      VkPipelineLayout pipelineLayout = nullptr;
      VkRenderPass renderPass = nullptr;
      uint32_t subpass = 0;
   };

   class LvePipeline {
      public:
         LvePipeline(
            LveDevice& device, 
            const std::string& vertFilepath, 
            const std::string& fragFilepath, 
            const PipelineConfigInfo& configInfo);
            //distructor for managing lifetime of resources
         ~LvePipeline();

         LvePipeline(const LvePipeline&) = delete;
         void operator=(const LvePipeline&) = delete;

         void bind(VkCommandBuffer commandBuffer);

         //static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
         static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t width, uint32_t height);

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