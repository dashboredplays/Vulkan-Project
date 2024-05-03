#pragma once

#include "vulkan_window.hpp"
#include "game_object.hpp"
#include "vulkan_renderer.hpp"

#include <memory>
#include <vector>

namespace lve {
	class FirstApp {

		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

         FirstApp();
         ~FirstApp();

         FirstApp(const FirstApp&) = delete;
		   FirstApp& operator=(const FirstApp&) = delete;

			void run();

		private:
         void loadGameObjects();

			LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan" };
         LveDevice lveDevice{lveWindow};
         LveRenderer lveRenderer{lveWindow, lveDevice};
         //order matters, initialized from top to bottom and destructed from bottom to top
         //using unique pointer rather than stack allocated variable, can easily create new swap chain with updated width and height by constructing new object. Has small performance cost
         //using this also means in implimentation file (.cpp), we can use -> operator to access members, not . operator (this.that vs this->that)
         //smart pointer, simulates pointer with automatic memory management
         std::vector<LveGameObject> gameObjects;
	};
}
