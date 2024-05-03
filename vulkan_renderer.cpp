#include "vulkan_renderer.hpp"


#include <stdexcept>
#include <array>
#include <cassert>

namespace lve {

	LveRenderer::LveRenderer(LveWindow &window, LveDevice &device) : lveWindow{window}, lveDevice{device} {
		recreateSwapChain();
		createCommandBuffers();
	}

	LveRenderer::~LveRenderer() {
		freeCommandBuffers();
	}

   //recreate swap chain if window is resized
   void LveRenderer::recreateSwapChain() {
      //get current window size
      auto extent = lveWindow.getExtent();
      //while one dimension is sizeless, program will pause and wait (like during minimization)
      while (extent.width == 0 || extent.height == 0) {
         extent = lveWindow.getExtent();
         glfwWaitEvents();
      }

      //don't create new swap chain until resize is done
      vkDeviceWaitIdle(lveDevice.device());
      if (lveSwapChain == nullptr) {
         lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
      } else {
         std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
         lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);
         
         if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
            throw std::runtime_error("Swap chain image format has changed");
         }
      }
   }

   //in vulkan, you can't execute commands directly with function calls.
   //first, record to command buffer, then submit to queue to be executed. Allow sequence of commands to be recorded once, then reused for multiple frames.
   //record command buffers once at init, reuse for each frame OR record command buffer every frame.
   //can't resubmit while in pending state. This is limited by LveSwapChain MAX_FRAMES_IN_FLIGHT. The cpu will block next call until the previous call is done.
   //specify target output frame buffer
	void LveRenderer::createCommandBuffers() {
      //resize to be same size as swapchain image count
      //MAX_FRAMES_IN_FLIGHT is currently 2
      commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      //primary: can be submitted to queue for execution, but can't be called from other command buffers
      //secondary: can't be submitted directly, but can be called from primary command buffers
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      //command pool used to reduce cost of creating and destroying command buffers frequently
      allocInfo.commandPool = lveDevice.getCommandPool();
      allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
      
      if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
         throw std::runtime_error("failed to allocate command buffers");
      }
   }

   void LveRenderer::freeCommandBuffers() {
      vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
      commandBuffers.clear();
   }

   VkCommandBuffer LveRenderer::beginFrame() {
      assert(!isFrameStarted && "Can't call beginFrame while frame is in progress.");

      //fetches index of frame we should render next
      auto result = lveSwapChain->acquireNextImage(&currentImageIndex);

      //if a surface has changed in a way that is no longer compatible with the swapchain
      if (result == VK_ERROR_OUT_OF_DATE_KHR) {
         recreateSwapChain();
         return nullptr; //indicate frame has not successfully started
      }

      //fix later, this can run if the window is resized
      if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
         throw std::runtime_error("failed to acquire next image");
      }

      isFrameStarted = true;

      auto commandBuffer = getCurrentCommandBuffer();
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
         throw std::runtime_error("failed to begin recording command buffer");
      }

      return commandBuffer;
   }

      void LveRenderer::endFrame() {
         assert(isFrameStarted && "Can't call endFrame while frame is not in progress.");
         auto commandBuffer = getCurrentCommandBuffer();
         if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer");
         }

         auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
         if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
         }
         else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffer");
         }

         isFrameStarted = false;
         currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
      }
      void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
         assert(isFrameStarted && "Can't begin render pass when frame not in progress.");
         assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass with command buffer from a different frame.");

         VkRenderPassBeginInfo renderPassInfo{};
         renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
         renderPassInfo.renderPass = lveSwapChain->getRenderPass();
         //which framebuffer to use
         renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

         renderPassInfo.renderArea.offset = { 0, 0 };
         renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

         std::array<VkClearValue, 2> clearValues{};
         //currently, we have 0 as the color attachment, and 1 as the depth attachment in the frame buffer
         //this is the background color
         clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
         clearValues[1].depthStencil = { 1.0f, 0 };
         renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
         renderPassInfo.pClearValues = clearValues.data();

         //imbeded in the primary command buffer, no mixing in with secondary command buffers
         vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

         VkViewport viewport{};
         viewport.x = 0.0f;
         viewport.y = 0.0f;
         viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
         viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
         viewport.minDepth = 0.0f;
         viewport.maxDepth = 1.0f;
         //use -> to access member of a structure through a pointer
         VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
         vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
         vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
      }
      void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
         assert(isFrameStarted && "Can't end render pass when frame not in progress.");
         assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass with command buffer from a different frame.");

         vkCmdEndRenderPass(commandBuffer);
      }
}
