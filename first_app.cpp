#include "first_app.hpp"
#include "render_system.hpp"
#include "vulkan_camera.hpp"

#include "movement_controller.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
//time library to get current system time
#include <chrono>
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
      // camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
      // camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

      //used to store camera's orientation and position
      auto viewerObject = LveGameObject::createGameObject();
      KeyboardMovementController cameraController{};

      //high precision clock
      auto currentTime = std::chrono::high_resolution_clock::now();

		while (!lveWindow.shouldClose()) {
			//keystrokes, exit clicks, etc.
			glfwPollEvents();
         //need to be below the poll events call
         auto newTime = std::chrono::high_resolution_clock::now();
         //amount of seconds elapsed
         float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
         currentTime = newTime;

         //update viewer object's transform component based on keyboard input, propotional to amount of time elapsed since last frame
         cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
         camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

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

   void FirstApp::loadGameObjects() {
      //viewing box: -1<x<1, -1<y<1, 0<z<1
      //only things that are inside the box will be rendered
      std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");

      auto gameObj = LveGameObject::createGameObject();
      gameObj.model = lveModel;
      //translations of x, y, and z (z for depth)
      gameObj.transform.translation = {0.f, 0.f, 2.5f};
      gameObj.transform.scale = glm::vec3(0.3f);
      gameObjects.push_back(std::move(gameObj));
   }


}
