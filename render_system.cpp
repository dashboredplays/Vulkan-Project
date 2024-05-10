#include "render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

namespace lve {

   struct SimplePushConstantData {
      //identity matrix (just main diagonal)
      glm::mat4 transform{1.f};
      glm::mat4 normalMatrix{1.f};
   };

   //: lveDevice{device} initializes lveDevice with device
	SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass) : lveDevice{device} {
		createPipelineLayout();
      createPipeline(renderPass);
	}

   SimpleRenderSystem::~SimpleRenderSystem() {
      vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
   }

	void SimpleRenderSystem::createPipelineLayout() {
      VkPushConstantRange pushConstantRange{};
      pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      pushConstantRange.offset = 0;
      pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		//0 to have empty pipeline layout
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		//way to send small amount of data to shaders
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
		// auto pipelineConfig = LvePipeline::defaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());
      assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
      PipelineConfigInfo pipelineConfig{};
      LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(lveDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);

		if (!lvePipeline) {
			throw std::runtime_error("failed to create graphics pipeline");
		}
	}

   //in vulkan, you can't execute commands directly with function calls.
   //first, record to command buffer, then submit to queue to be executed. Allow sequence of commands to be recorded once, then reused for multiple frames.
   //record command buffers once at init, reuse for each frame OR record command buffer every frame.
   //can't resubmit while in pending state. This is limited by LveSwapChain MAX_FRAMES_IN_FLIGHT. The cpu will block next call until the previous call is done.
   //specify target output frame buffer


   void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects, const LveCamera &camera) {
      lvePipeline->bind(commandBuffer);

      auto projectionView = camera.getProjection() * camera.getView();

      for (auto& obj : gameObjects) {
         SimplePushConstantData push{};
         auto modelMatrix = obj.transform.mat4();
         push.transform = projectionView * modelMatrix;
         //glm will automatically convert mat3 to a padded mat4 (1 on last diagonal)
         push.normalMatrix = obj.transform.normalMatrix();

         vkCmdPushConstants(
            commandBuffer,  
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(SimplePushConstantData), 
            &push);
         obj.model->bind(commandBuffer);
         obj.model->draw(commandBuffer);
      }
   }

}
