#pragma once

#include "vulkan_window.hpp"
#include "vulkan_swap_chain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace lve {
	class LveRenderer {

		public:

         LveRenderer(LveWindow &window, LveDevice &device);
         ~LveRenderer();

         LveRenderer(const LveRenderer&) = delete;
		   LveRenderer& operator=(const LveRenderer&) = delete;

         VkRenderPass getSwapChainRenderPass() const {
            return lveSwapChain->getRenderPass();
         }

         float getAspectRatio() {
            return lveSwapChain->extentAspectRatio();
         }

         bool isFrameInProgress() { return isFrameStarted; }

         VkCommandBuffer getCurrentCommandBuffer() { 
            //add assertion to ensure frame has already started
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress.");
            return commandBuffers[currentFrameIndex]; 
         }

         int getFrameIndex() {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress.");
            return currentFrameIndex;
         }

         //start frame, record to command buffer, then end frame with that buffer being executed
         VkCommandBuffer beginFrame();
         void endFrame();
         void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
         void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		private:
         void createCommandBuffers();
         void freeCommandBuffers();
         void recreateSwapChain();

			LveWindow& lveWindow;
         LveDevice& lveDevice;
         std::unique_ptr<LveSwapChain> lveSwapChain;
         std::vector<VkCommandBuffer> commandBuffers;

         uint32_t currentImageIndex;
         int currentFrameIndex;
         bool isFrameStarted = false;
	};
}
