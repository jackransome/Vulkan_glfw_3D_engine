#include "Graphics.h"
#include "Input.h"

Graphics gfx;
Input input;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	input.key_callback(key, scancode, action, mods);
}

int main() {


	try {

		gfx.init();
		input.init(gfx.getWindowPointer());
		glfwSetKeyCallback(gfx.getWindowPointer(), key_callback);

		while (!gfx.shouldClose) {

			gfx.setCameraAngle(input.cameraAngle);
			input.run();
			gfx.run();
			
			if (input.keys.w) {
				gfx.changeCameraPos(0, 0, 0.01);
			}
			if (input.keys.a) {
				gfx.changeCameraPos(-0.01, 0, 0);
			}
			if (input.keys.s) {
				gfx.changeCameraPos(0, 0, -0.01);
			}
			if (input.keys.d) {
				gfx.changeCameraPos(0.01, 0, 0);
			}
			if (input.keys.space) {
				gfx.changeCameraPos(0.00, 0.01, 0);
			}
			if (input.keys.leftShift) {
				gfx.changeCameraPos(0.00, -0.01, 0);
			}
		}
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		for (int i = 0; i < 100000000; i++){}
		return EXIT_FAILURE;
	}
	gfx.cleanup();
	return EXIT_SUCCESS;
}