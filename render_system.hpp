#pragma once

#include "vulkan_pipeline.hpp"
#include "game_object.hpp"

#include <memory>
#include <vector>

namespace lve {
	class SimpleRenderSystem {

		public:

         SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass);
         ~SimpleRenderSystem();

         SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		   SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

         void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects);


		private:
         void createPipelineLayout();
         void createPipeline(VkRenderPass renderPass);

         LveDevice &lveDevice;
			std::unique_ptr<LvePipeline> lvePipeline;
         VkPipelineLayout pipelineLayout;
	};
}
