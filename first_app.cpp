#include "first_app.hpp"

#include <stdexcept>
#include <array>
#include <cassert>

namespace lve {

	FirstApp::FirstApp() {
      loadModels();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();

		if (!lvePipeline) {
			throw std::runtime_error("failed to create pipeline");
		}
	}

	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run() {

		while (!lveWindow.shouldClose()) {
			//keystrokes, exit clicks, etc.
			glfwPollEvents();
         drawFrame();
		}
	}

   void FirstApp::loadModels() {
      std::vector<LveModel::Vertex> vertices {
         {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
         {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
         {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
      };

      lveModel = std::make_unique<LveModel>(lveDevice, vertices);
   }

	void FirstApp::createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		//0 to have empty pipeline layout
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		//way to send small amount of data to shaders
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void FirstApp::createPipeline() {
		// auto pipelineConfig = LvePipeline::defaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());
      assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
      assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
      PipelineConfigInfo pipelineConfig{};
      LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = lveSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(lveDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);

		if (!lvePipeline) {
			throw std::runtime_error("failed to create graphics pipeline");
		}
	}

   //recreate swap chain if window is resized
   void FirstApp::recreateSwapChain() {
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
         assert(
            lveSwapChain->imageCount() == oldSwapChain->imageCount() &&
            "Swap chain image count has changed!");
      }
      //pipeline depends on swap chain, so recreate pipeline
      //if render pass compatible do nothing, else recreate render pass (do later)
      createPipeline();
   }

   //in vulkan, you can't execute commands directly with function calls.
   //first, record to command buffer, then submit to queue to be executed. Allow sequence of commands to be recorded once, then reused for multiple frames.
   //record command buffers once at init, reuse for each frame OR record command buffer every frame.
   //can't resubmit while in pending state. This is limited by LveSwapChain MAX_FRAMES_IN_FLIGHT. The cpu will block next call until the previous call is done.
   //specify target output frame buffer
	void FirstApp::createCommandBuffers() {
      //resize to be same size as swapchain image count
      commandBuffers.resize(lveSwapChain->imageCount());

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

   void FirstApp::freeCommandBuffers() {
      vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
      commandBuffers.clear();
   }

      //record / draw commands
   void FirstApp::recordCommandBuffer(int imageIndex) {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
         throw std::runtime_error("failed to begin recording command buffer");
      }

      //begin render pass, create another struct
      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = lveSwapChain->getRenderPass();
      //which framebuffer to use
      renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

      renderPassInfo.renderArea.offset = { 0, 0 };
      renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

      std::array<VkClearValue, 2> clearValues{};
      //currently, we have 0 as the color attachment, and 1 as the depth attachment in the frame buffer
      clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
      clearValues[1].depthStencil = { 1.0f, 0 };
      renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
      renderPassInfo.pClearValues = clearValues.data();

      //imbeded in the primary command buffer, no mixing in with secondary command buffers
      vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
      viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;
      VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
      vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
      vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

      //use -> to access member of a structure through a pointer
      lvePipeline->bind(commandBuffers[imageIndex]);
      lveModel->bind(commandBuffers[imageIndex]);
      lveModel->draw(commandBuffers[imageIndex]);
      //draw 3 vertices in 1 instance
      //0, 0 because we aren't using offsets
      //vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

      vkCmdEndRenderPass(commandBuffers[imageIndex]);
      if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
         throw std::runtime_error("failed to record command buffer");
      }
   }

	void FirstApp::drawFrame() {
      uint32_t imageIndex;
      //fetches index of frame we should render next
      auto result = lveSwapChain->acquireNextImage(&imageIndex);

      //if a surface has changed in a way that is no longer compatible with the swapchain
      if (result == VK_ERROR_OUT_OF_DATE_KHR) {
         recreateSwapChain();
         return;
      }

      //fix later, this can run if the window is resized
      if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
         throw std::runtime_error("failed to acquire next image");
      }

      recordCommandBuffer(imageIndex);

      //submit provided command buffer to device graphics queue, command buffer will be executed, swap chain presets associated image at appropriate time
      result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
      if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
         lveWindow.resetWindowResizedFlag();
         recreateSwapChain();
         return;
      }
      if (result != VK_SUCCESS) {
         throw std::runtime_error("failed to submit command buffer");
      }
   }
}
