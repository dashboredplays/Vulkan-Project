#include "first_app.hpp"

namespace lve {
	void FirstApp::run() {

		while (!lveWindow.shouldClose()) {
			//keystrokes, exit clicks, etc.
			glfwPollEvents();
		}
	}
}