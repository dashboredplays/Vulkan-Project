#include "first_app.hpp"

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
      glm::mat2 transform{1.f};
      glm::vec2 offset;
      //need alignas to ensure that the data is aligned to 16 bytes, which is a requirement for push constants. 
      //By default, glm::vec3 is not 16 byte aligned. So, we need to specify the alignment manually.
      alignas(16) glm::vec3 color;
   };

	FirstApp::FirstApp() {
      loadGameObjects();
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

   void FirstApp::loadGameObjects() {
      std::vector<LveModel::Vertex> vertices {
         {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
         {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
         {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
      };

      //make_shared: 1 model instance to be used by multiple game objects. Will stay in memory as long as at least one game object is using it.
      auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

      std::vector<glm::vec3> colors{
         {1.f, .7f, .73f},
         {1.f, .87f, .73f},
         {1.f, 1.f, .73f},
         {.73f, 1.f, .8f},
         {.73, .88f, 1.f}  //
      };
      for (auto& color : colors) {
         color = glm::pow(color, glm::vec3{2.2f});
      }
      for (int i = 0; i < 40; i++) {
         auto triangle = LveGameObject::createGameObject();
         triangle.model = lveModel;
         triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
         triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
         triangle.color = colors[i % colors.size()];
         gameObjects.push_back(std::move(triangle));
      }

      // auto triangle = LveGameObject::createGameObject();
      // triangle.model = lveModel;
      // triangle.color = {.1f, 0.8f, 0.1f};
      // triangle.transform2d.translation.x = 0.2f;
      // triangle.transform2d.scale = {2.f, 0.5f};
      // //radians. In this example, we are rotating the triangle by 0.25 * 2pi = 90 degrees
      // //this transformation will rotate counter clockwise
      // triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();

      // gameObjects.push_back(std::move(triangle));
   }

	void FirstApp::createPipelineLayout() {
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
      //this is the background color
      clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
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
      //use -> to access member of a structure through a pointer
      VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
      vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
      vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);
      
      renderGameObjects(commandBuffers[imageIndex]);

      //draw 3 vertices in 1 instance
      //0, 0 because we aren't using offsets
      //vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

      vkCmdEndRenderPass(commandBuffers[imageIndex]);
      if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
         throw std::runtime_error("failed to record command buffer");
      }
   }

   void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {
      lvePipeline->bind(commandBuffer);
      int i = 0;
      for (auto& obj: gameObjects) {
         i += 1;
         obj.transform2d.rotation = 
         glm::mod<float>(obj.transform2d.rotation + 0.00002f * i, 2.f * glm::pi<float>());

         SimplePushConstantData push{};
         push.offset = obj.transform2d.translation;
         push.color = obj.color;
         push.transform = obj.transform2d.mat2();

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
