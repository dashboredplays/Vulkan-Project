#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
using namespace std;

//lve = little vulkan engine
namespace lve {

	class LveWindow {
		public:
		LveWindow(int w, int h, string name);
		//distructor for window
		~LveWindow();

		//don't want to copy lve window, have 2 pointers. Other would terminate pointer, and would have dangling pointer
		LveWindow(const LveWindow&) = delete;
		LveWindow& operator=(const LveWindow&) = delete;
      LveWindow() = default;

		bool shouldClose() {
			return glfwWindowShouldClose(window);
		}
      VkExtent2D getExtent() {
         return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
      }
      bool wasWindowResized() {
         return framebufferResized;
      }
      void resetWindowResizedFlag() {
         framebufferResized = false;
      }

      void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

		private:
         static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
			void initWindow();

			int width;
			int height;
			bool framebufferResized = false;
			
			string windowName;
			GLFWwindow *window;
		};
}
