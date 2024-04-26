#include "first_app.hpp"

#include <stdexcept>
#include <array>

namespace lve {

	FirstApp::FirstApp() {
		createPipelineLayout();
		createPipeline();
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
      PipelineConfigInfo pipelineConfig{};
      LvePipeline::defaultPipelineConfigInfo(pipelineConfig, lveSwapChain.width(), lveSwapChain.height());
		pipelineConfig.renderPass = lveSwapChain.getRenderPass();
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
	void FirstApp::createCommandBuffers() {
      //resize to be same size as swapchain image count
      commandBuffers.resize(lveSwapChain.imageCount());

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

      //record / draw commands
      for (int i = 0; i < commandBuffers.size(); i++) {
         VkCommandBufferBeginInfo beginInfo{};
         beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

         if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer");
         }

         //begin render pass, create another struct
         VkRenderPassBeginInfo renderPassInfo{};
         renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
         renderPassInfo.renderPass = lveSwapChain.getRenderPass();
         //which framebuffer to use
         renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

         renderPassInfo.renderArea.offset = { 0, 0 };
         renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

         std::array<VkClearValue, 2> clearValues{};
         //currently, we have 0 as the color attachment, and 1 as the depth attachment in the frame buffer
         clearValues[0].color = { 0.467f, 0.388f, 0.906f, 1.0f }; //light purple
         clearValues[1].depthStencil = { 1.0f, 0 };
         renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
         renderPassInfo.pClearValues = clearValues.data();

         //imbeded in the primary command buffer, no mixing in with secondary command buffers
         vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

         //use -> to access member of a structure through a pointer
         lvePipeline->bind(commandBuffers[i]);
         //draw 3 vertices in 1 instance
         //0, 0 because we aren't using offsets
         vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

         vkCmdEndRenderPass(commandBuffers[i]);
         if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer");
         }
      }
   }

	void FirstApp::drawFrame() {
      uint32_t imageIndex;
      //fetches index of frame we should render next
      auto result = lveSwapChain.acquireNextImage(&imageIndex);

      //fix later, this can run if the window is resized
      if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
         throw std::runtime_error("failed to acquire next image");
      }

      //submit provided command buffer to device graphics queue, command buffer will be executed, swap chain presets associated image at appropriate time
      result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
      if (result != VK_SUCCESS) {
         throw std::runtime_error("failed to submit command buffer");
      }
   }
}
