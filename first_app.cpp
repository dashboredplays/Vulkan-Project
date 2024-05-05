#include "first_app.hpp"
#include "render_system.hpp"
#include "vulkan_camera.hpp"

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
      LveCamera camera{};
      //camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
      camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		while (!lveWindow.shouldClose()) {
			//keystrokes, exit clicks, etc.
			glfwPollEvents();

         //using aspect ratio will keep the image from being stretched
         float aspect = lveRenderer.getAspectRatio();
         //kept up to date with window size (aspect ratio)
         //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
         //common values are between 45 and 60 degrees
         camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
         
         if (auto commandBuffer = lveRenderer.beginFrame()) {
            
            //begin offscreen shadow pass
            // render shadow casting objects
            //end offscreen shadow pass
            
            lveRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
         }
		}
	}

      // temporary helper function, creates a 1x1x1 cube centered at offset
   std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
   std::vector<LveModel::Vertex> vertices{
   
         // left face (white)
         {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
         {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
         {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
         {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
         {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
         {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
   
         // right face (yellow)
         {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
         {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
         {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
         {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
         {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
         {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
   
         // top face (orange, remember y axis points down)
         {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
         {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
         {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
         {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
         {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
         {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
   
         // bottom face (red)
         {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
         {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
         {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
         {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
         {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
         {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
   
         // nose face (blue)
         {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
         {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
         {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
         {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
         {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
         {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
   
         // tail face (green)
         {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
         {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
         {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
         {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
         {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
         {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
   
      };
      for (auto& v : vertices) {
         v.position += offset;
      }
      return std::make_unique<LveModel>(device, vertices);
   }

   void FirstApp::loadGameObjects() {
      //viewing box: -1<x<1, -1<y<1, 0<z<1
      //only things that are inside the box will be rendered
      std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, {0.f, 0.f, 0.f});

      auto cube = LveGameObject::createGameObject();
      cube.model = lveModel;
      //translations of x, y, and z (z for depth)
      cube.transform.translation = {0.f, 0.f, 2.5f};
      cube.transform.scale = {0.5f, 0.5f, 0.5f};
      gameObjects.push_back(std::move(cube));
   }


}
