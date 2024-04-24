#pragma once

#include "vulkan_window.hpp"
#include "vulkan_pipeline.hpp"

namespace lve {
	class FirstApp {

		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

			void run();

		private:
			LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan" };
			LvePipeline LvePipeline{"shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv"};
	};
}
