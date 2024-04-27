#include "vulkan_pipeline.hpp"

#include "vulkan_model.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace lve {

   LvePipeline::LvePipeline(
            LveDevice& device, 
            const std::string& vertFilepath, 
            const std::string& fragFilepath, 
            const PipelineConfigInfo& configInfo) 
            : lveDevice{device} {
      try {
         createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
      } catch (const std::exception& e) {
         std::cout << "Error creating graphics pipeline: " << e.what() << std::endl;
      }
   }

   LvePipeline::~LvePipeline() {
      vkDestroyShaderModule(lveDevice.device(), vertShaderModule, nullptr);
      vkDestroyShaderModule(lveDevice.device(), fragShaderModule, nullptr);
      vkDestroyPipeline(lveDevice.device(), graphicsPipeline, nullptr);
   }

   std::vector<char> LvePipeline::readFile(const std::string& filepath) {

      std::ifstream file(filepath, std::ios::ate | std::ios::binary);

      if (!file.is_open()) {
         throw std::runtime_error("failed to open file: " + filepath);
      }

      size_t fileSize = static_cast<size_t>(file.tellg());
      std::vector<char> buffer(fileSize);

      file.seekg(0);
      file.read(buffer.data(), fileSize);

      file.close();
      return buffer;
   }

   void LvePipeline::createGraphicsPipeline(
      const std::string& vertFilepath, 
      const std::string& fragFilepath, 
      const PipelineConfigInfo& configInfo) {

      //VK_NULL_HANDLE vs nullptr: VK_NULL_HANDLE is a 64-bit integer, while nullptr is a pointer.
      //assert is a macro that will terminate the program if the condition is false
      assert(configInfo.pipelineLayout != VK_NULL_HANDLE && 
      "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
      assert(configInfo.renderPass != VK_NULL_HANDLE &&
      "Cannot create graphics pipeline: no renderPass provided in configInfo");

      std::cout << "Vertex shader file path: " << vertFilepath << std::endl;
      std::cout << "Fragment shader file path: " << fragFilepath << std::endl;

      auto vertCode = readFile(vertFilepath);
      auto fragCode = readFile(fragFilepath);

      std::cout << "vertCode size: " << vertCode.size() << '\n';
      std::cout << "fragCode size: " << fragCode.size() << '\n';

      //& is a pointer (memory address)
      createShaderModule(vertCode, &vertShaderModule);
      createShaderModule(fragCode, &fragShaderModule);

      //array of 2 structs. This struct describes a shader stage in a pipeline.
      VkPipelineShaderStageCreateInfo shaderStages[2];
   //for vertex shader module
      shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
      shaderStages[0].module = vertShaderModule;
      //name of entry function in shader
      shaderStages[0].pName = "main";
      shaderStages[0].flags = 0;
      shaderStages[0].pNext = nullptr;
      shaderStages[0].pSpecializationInfo = nullptr;
   //for fragment shader module
      shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      shaderStages[1].module = fragShaderModule;
      shaderStages[1].pName = "main";
      shaderStages[1].flags = 0;
      shaderStages[1].pNext = nullptr;
      shaderStages[1].pSpecializationInfo = nullptr;

      //if you use pipelineInfo but it gets destroyed, then the pipeline will be destroyed as well
      auto bindingDescriptions = LveModel::Vertex::getBindingDescriptions();
      auto attributeDescriptions = LveModel::Vertex::getAttributeDescriptions();
      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
      vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
      vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
      vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

      // VkPipelineViewportStateCreateInfo viewportInfo{};
      // viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      // viewportInfo.viewportCount = 1;
      // viewportInfo.pViewports = &configInfo.viewport;
      // viewportInfo.scissorCount = 1;
      // viewportInfo.pScissors = &configInfo.scissor;


      VkGraphicsPipelineCreateInfo pipelineInfo{};
      pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      //how many programmable stages are in the pipeline
      pipelineInfo.stageCount = 2;
      pipelineInfo.pStages = shaderStages;
      //wire up create info to config info. Can use this code to create pipelines in the future
      pipelineInfo.pVertexInputState = &vertexInputInfo;
      pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
      pipelineInfo.pViewportState = &configInfo.viewportInfo;
      //pipelineInfo.pViewportState = &viewportInfo;
      pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
      pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
      pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
      pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
      pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

      pipelineInfo.layout = configInfo.pipelineLayout;
      pipelineInfo.renderPass = configInfo.renderPass;
      pipelineInfo.subpass = configInfo.subpass;

      //optimize performance by reusing parts of pipeline
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
      pipelineInfo.basePipelineIndex = -1;

      if (vkCreateGraphicsPipelines(lveDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
         throw std::runtime_error("failed to create graphics pipeline");
      }
   }

   void LvePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
      //common pattern in vulkan: rather than calling function with a lot of parameters, create a struct and call a function with a pointer to it
      VkShaderModuleCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      //size of vector array
      createInfo.codeSize = code.size();
      //pointer to code (expects uint32 not char). Be careful, they aren't the same size. 
      createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

      if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
         std::cerr << "Failed to create vertex shader module!" << std::endl;
         throw std::runtime_error("failed to create shader module");
      }
   }

   void LvePipeline::bind(VkCommandBuffer commandBuffer) {
      //BIND_POINT_GRAPHICS signifies this is graphics pipeline (also has compute and ray tracing pipelines)
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
   }

   void LvePipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
      //first stage of pipeline. Takes list of numbers as vertices. (ex. list: a, b, c, d, e, f, which are (a, b), (c, d), (e, f)).
      //input assembler with TOPOLGY_TRIANGLE_LIST will take 3 vertices at a time and assemble them into a triangle
      //also can have triangle strip, which is taking lines from an existing triangle and creating a new one
      //look into other topologies with VkPrimitiveTopology
      configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

      configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      configInfo.viewportInfo.viewportCount = 1;
      configInfo.viewportInfo.pViewports = nullptr;
      configInfo.viewportInfo.scissorCount = 1;
      configInfo.viewportInfo.pScissors = nullptr;

      //rasterization stage: breaks up geometry into fragments for each pixel that triangle overlaps
      //always set struct type member first
      configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      //forces z component to be between 0 and 1 - we DON'T want this to be true. 
      //If we have a negative value, the object would be "behind the camera" and if over 1, then it would be too far away.
      configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
      configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
      //do we want to draw lines or fill in triangles
      configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
      configInfo.rasterizationInfo.lineWidth = 1.0f;
      //this is to set which side of the triangle we want to see. 
      //if the points are counted in a clockwise direction (winding order), then we would be seeing the clockwise face.
      //imagine the top of the triangle is 0, and the bottom 2 points are 1 and 2. We can swap the order of 1 and 2 to change the face.
      //you can imagine this by physically drawing a triangle on a piece of paper and looking at it from the front and back. 1 and 2 will be in swapped positions.
      //for now, it will be set it to none
      configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
      configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
      configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
      //can alter depth value by adding a constant value or a value proportional to the slope of the triangle
      //disabled for now
      configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
      configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
      configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

      //multisampling: how rasterizer handles edges of objects. 
      //without it, the result of a pixel being within a triangle is binary, and can cause jagged edges (aliasing)
      //MSAA (multisample anti-aliasing) is a technique to smooth out jagged edges by shading pixel by variable amount
      configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
      configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
      configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
      configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
      configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

      //how we combine colors in the framebuffers.
      configInfo.colorBlendAttachment.colorWriteMask =
         VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
         VK_COLOR_COMPONENT_A_BIT;
      configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
      configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   
      configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  
      configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             
      configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   
      configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  
      configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              
      
      configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
      configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
      configInfo.colorBlendInfo.attachmentCount = 1;
      configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
      configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
      configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
      configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
      configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

      //depth testing: stores value for every pixel like color.
      //if a cloud is drawn behind a mountain with cloud = 0.8 and mountain = 0.5, the cloud won't be drawn as it is behind the mountain.
      configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
      configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
      configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
      configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
      configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
      configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
      configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
      configInfo.depthStencilInfo.front = {};  // Optional
      configInfo.depthStencilInfo.back = {};   // Optional

      configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
      configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
      configInfo.dynamicStateInfo.dynamicStateCount =
      static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
      configInfo.dynamicStateInfo.flags = 0;

   }
}