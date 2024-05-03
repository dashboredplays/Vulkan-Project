#include "first_app.hpp"
#include "render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

namespace lve {

	FirstApp::FirstApp() {
      loadGameObjects();
	}

	FirstApp::~FirstApp() {

	}

	void FirstApp::run() {
      SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};

		while (!lveWindow.shouldClose()) {
			//keystrokes, exit clicks, etc.
			glfwPollEvents();
         
         if (auto commandBuffer = lveRenderer.beginFrame()) {
            
            //begin offscreen shadow pass
            // render shadow casting objects
            //end offscreen shadow pass
            
            lveRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
         }
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


}
