#include "Graphics.h"
#include "Input.h" 
#include "SpriteSheet.h"
#include <functional>
#include "Player.h"
#include "CollisionDetection.h"
#include "Platform.h"
#include "BloodParticle.h"

Input input;
Graphics graphics;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	input.key_callback(key, scancode, action, mods);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	input.mouse_button_callback(button, action, mods);
}

void window_focus_callback(GLFWwindow* window, int focused) {
	input.window_focus_callback(focused);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	input.scroll_callback(xoffset, yoffset);
}

//entry point of the application
int main() {
	SpriteSheet testSprite1;
	testSprite1.init(&graphics, glm::vec2(0, 208), 10, 17, 7, 5, 7);

	SpriteSheet testSprite2;
	testSprite2.init(&graphics, glm::vec2(20, 225), 13, 25, 7, 8, 7);

	Player player (0, 0, &input, &graphics);

	try {
		graphics.initWindow();
		glfwSetKeyCallback(graphics.window, key_callback);
		glfwSetScrollCallback(graphics.window, scroll_callback);
		glfwSetMouseButtonCallback(graphics.window, mouse_button_callback);
		glfwSetWindowFocusCallback(graphics.window, window_focus_callback);
		graphics.drawRect(0, 0, 1000, 1000, 1, 0, 1, 1);
		graphics.initVulkan();
		input.init(graphics.window);

	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		while (true)
		{
			int x = 0;
		}
		return EXIT_FAILURE;
	}

	//seeding the random function with the current time
	srand(time(0));

	std::vector<Platform> platforms;
	for (int i = 0; i < 20; i++) {
		platforms.push_back(Platform(((double)rand() / (RAND_MAX))*1800 - 900, ((double)rand() / (RAND_MAX)) * 1800 - 900, ((double)rand() / (RAND_MAX))*100 + 80, ((double)rand() / (RAND_MAX))*40 + 20, &graphics));
	}

	CollisionDetection cd;
	while (!glfwWindowShouldClose(graphics.window) && !input.EXIT) {
		input.run();
		player.handleInput();
		for (int i = 0; i < platforms.size(); i++) {
			cd.correctPosition(player.getBoundingBoxPointer(), platforms[i].getBoundingBoxPointer());
		}
		player.updatePosition();
		
		if (input.keys.tab) {
			platforms.clear();
			for (int i = 0; i < 20; i++) {
				platforms.push_back(Platform(((double)rand() / (RAND_MAX)) * 1800 - 900, ((double)rand() / (RAND_MAX)) * 1800 - 900, ((double)rand() / (RAND_MAX)) * 160 + 60, ((double)rand() / (RAND_MAX)) * 80 + 10, &graphics));
			}
		}

		if (input.keys.n0) {
			graphics.drawLines = true;
		}
		if (input.keys.n1) {
			graphics.drawLines = false;
		}

		player.draw();
		for (int i = 0; i < platforms.size(); i++) {
			platforms[i].draw();
		}

		graphics.drawFrame();
		graphics.handleTiming();
	}
	return EXIT_SUCCESS;
}