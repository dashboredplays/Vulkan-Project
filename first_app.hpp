#pragma once

#include "vulkan_window.hpp"
#include "vulkan_pipeline.hpp"
#include "vulkan_swap_chain.hpp"
#include "vulkan_model.hpp"

#include <memory>
#include <vector>

namespace lve {
	class FirstApp {

		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

         FirstApp();
         ~FirstApp();

         FirstApp(const FirstApp&) = delete;
		   FirstApp& operator=(const FirstApp&) = delete;

			void run();

		private:
         void loadModels();
         void createPipelineLayout();
         void createPipeline();
         void createCommandBuffers();
         void drawFrame();

			LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan" };
         LveDevice lveDevice{lveWindow};
         //order matters, initialized from top to bottom and destructed from bottom to top
         LveSwapChain lveSwapChain{lveDevice, lveWindow.getExtent()};
         //smart pointer, simulates pointer with automatic memory management
			std::unique_ptr<LvePipeline> lvePipeline;
         VkPipelineLayout pipelineLayout;
         std::vector<VkCommandBuffer> commandBuffers;
         std::unique_ptr<LveModel> lveModel;
	};
}
