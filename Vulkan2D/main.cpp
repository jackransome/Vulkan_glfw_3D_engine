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
		return EXIT_FAILURE;
	}
	BoundingBox blueBox;
	blueBox.x = -500;
	blueBox.y = -500;
	blueBox.w = 500;
	blueBox.h = 500;

	BoundingBox redBox;
	redBox.x = -250;
	redBox.y = 200;
	redBox.w = 500;
	redBox.h = 50;

	BoundingBox greenBox;
	greenBox.x = -500;
	greenBox.y = 600;
	greenBox.w = 1000;
	greenBox.h = 10;
	srand(time(0));
	std::vector<Platform> platforms;
	std::vector<BloodParticle> bloodParticles;
	for (int i = 0; i < 60; i++) {
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
			for (int i = 0; i < 60; i++) {
				platforms.push_back(Platform(((double)rand() / (RAND_MAX)) * 1800 - 900, ((double)rand() / (RAND_MAX)) * 1800 - 900, ((double)rand() / (RAND_MAX)) * 100 + 80, ((double)rand() / (RAND_MAX)) * 40 + 20, &graphics));
			}
		}
		BoundingBox p = player.getBoundingBox();
		bloodParticles.push_back(BloodParticle(p.x, p.y, ((double)rand() / (RAND_MAX)) * 10 - 5, ((double)rand() / (RAND_MAX)) * 10 - 5, 5, 5, &graphics));
		player.draw();
		
		for (int i = 0; i < bloodParticles.size(); i++) {
			bloodParticles[i].draw();
			bloodParticles[i].updatePosition();
		}

		for (int i = 0; i < platforms.size(); i++) {
			platforms[i].draw();
			for (int j = 0; j < bloodParticles.size(); j++) {
				if (cd.CheckRectangleIntersect(platforms[i].getBoundingBoxPointer(), bloodParticles[j].getBoundingBoxPointer())){
					bloodParticles.erase(bloodParticles.begin() + j);
					j--;
				}
			}
		}

		graphics.drawFrame();
		graphics.handleTiming();
	}
	return EXIT_SUCCESS;
}