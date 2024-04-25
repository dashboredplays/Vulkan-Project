#include "vulkan_window.hpp"

#include <stdexcept>
namespace lve {
	LveWindow::LveWindow(int w, int h, string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	LveWindow::~LveWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void LveWindow::initWindow() {
		glfwInit();
		//no api since we aren't using open gl
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//don't resize window
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		//null pointer - object handle doesn't point to object
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	}

   void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
      if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
         throw std::runtime_error("failed to create window surface");
      }
   }
}