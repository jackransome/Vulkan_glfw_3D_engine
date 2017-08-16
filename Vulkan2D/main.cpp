#include "Graphics.h"
#include "Input.h" 
#include "SpriteSheet.h"
#include <functional>

Input input;
Graphics graphics;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	input.key_callback(key, scancode, action, mods);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	input.mouse_button_callback(button, action, mods);
}

static void window_focus_callback(GLFWwindow* window, int focused) {
	input.window_focus_callback(focused);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	input.scroll_callback(xoffset, yoffset);
}

//entry point of the application
int main() {
	SpriteSheet testSprite1;
	testSprite1.init(&graphics, glm::vec2(0, 208), 10, 17, 7, 5, 7);

	SpriteSheet testSprite2;
	testSprite2.init(&graphics, glm::vec2(20, 225), 13, 25, 7, 8, 7);

	glfwSetKeyCallback(graphics.window, key_callback);
	glfwSetScrollCallback(graphics.window, scroll_callback);
	glfwSetMouseButtonCallback(graphics.window, mouse_button_callback);
	glfwSetWindowFocusCallback(graphics.window, window_focus_callback);

	try {
		graphics.initWindow();
		graphics.drawRect(0, 0, 1000, 1000, 1, 0, 1, 1);
		graphics.initVulkan();
		input.init(graphics.window);
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	while (!glfwWindowShouldClose(graphics.window) && !input.EXIT) {
		input.run();

		testSprite1.run();
		testSprite1.draw(input.mousePosInWindow.x + 200, input.mousePosInWindow.y + 200);
		testSprite2.run();
		testSprite2.draw(input.mousePosInWindow.x, input.mousePosInWindow.y);
		graphics.drawRect(0, 0, 1000, 1000, 1, 0, 1, 1);
		graphics.drawRect(-30, -30, 100, 100, 0, 1, 0, 1);		
		
		graphics.drawFrame();
		graphics.handleTiming();
	}
	return EXIT_SUCCESS;
}