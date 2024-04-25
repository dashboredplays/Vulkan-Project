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

		bool shouldClose() {
			return glfwWindowShouldClose(window);
		}

      void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

		private:
			void initWindow();

			const int width;
			const int height;
			
			string windowName;
			GLFWwindow *window;
		};
}
