#include "Graphics.h"
#include "Input.h"
#include "CollisionDetection.h"
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
		bool lastF;
		std::vector<CollisionBox> boxes;
		CollisionBox test;
		test.dimensions = glm::vec3(1, 1, 1);
		test.velocity = glm::vec3(0, 0, 0);
		test.position = glm::vec3(0, 0, 0);
		boxes.push_back(test);
		glm::vec3 inputVelocity = glm::vec3(0,0,0);
		CollisionBox camera;
		camera.dimensions = glm::vec3(0.01, 0.01, 0.01);
		camera.velocity = glm::vec3(0, 0, 0);
		bool lastSpace;
		while (!gfx.shouldClose) {

			gfx.setCameraAngle(input.cameraAngle);
			input.run();
			gfx.run();

			inputVelocity = glm::vec3(0, 0, 0);
			
			if (input.keys.w) {
				inputVelocity.z = 0.01;
			}
			if (input.keys.a) {
				inputVelocity.x = -0.01;
			}
			if (input.keys.s) {
				inputVelocity.z = -0.01;
			}
			if (input.keys.d) {
				inputVelocity.x = 0.01;
			}
			if (input.keys.space) {
				inputVelocity.y = 0.01;
			}
			if (input.keys.leftShift) {
				inputVelocity.y = -0.01;
			}

			camera.velocity = gfx.getProperCameraVelocity(inputVelocity);
			for (int i = 0; i < boxes.size(); i++) {
				collisionDetection::correctCollisionBoxes(&camera, &boxes[i]);
			}
			gfx.setCameraPos(camera.position);
			gfx.setCameraPos(gfx.getCameraPos()+camera.velocity);
			if (input.keys.f && !lastF) {
				gfx.addObject(gfx.getCameraPos().x, gfx.getCameraPos().y, gfx.getCameraPos().z, 1);
				test.dimensions = glm::vec3(1, 1, 1);
				test.velocity = glm::vec3(0, 0, 0);
				test.position = glm::vec3(gfx.getCameraPos().x, gfx.getCameraPos().y, gfx.getCameraPos().z);
				boxes.push_back(test);
			}
			lastF = input.keys.f;
			camera.position = gfx.getCameraPos();

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