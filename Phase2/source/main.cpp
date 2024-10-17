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
		bool last1;
		bool last2;
		bool last3;
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
		//TODO: add fps std::cout printing
		double lastTime = glfwGetTime();
		int frameCount = 0;	
		while (!gfx.shouldClose) {
			double currentTime = glfwGetTime();
			frameCount++;
			if (currentTime - lastTime >= 1.0) {
				std::cout << "FPS: " << frameCount << std::endl;
				frameCount = 0;
				lastTime = currentTime;
			}
	

			gfx.setCameraAngle(input.cameraAngle);
			input.run();
			gfx.run();

			inputVelocity = glm::vec3(0, 0, 0);
			float speed = 0.03f;
			if (input.keys.ctrl) {
				speed = 0.1f;
			}
			if (input.keys.w) {
				inputVelocity.z = speed;
			}
			if (input.keys.a) {
				inputVelocity.x = -speed;
			}
			if (input.keys.s) {
				inputVelocity.z = -speed;
			}
			if (input.keys.d) {
				inputVelocity.x = speed;
			}
			if (input.keys.space) {
				inputVelocity.y = speed;
			}
			if (input.keys.leftShift) {
				inputVelocity.y = -speed;
			}

			camera.velocity = gfx.getProperCameraVelocity(inputVelocity);
			for (int i = 0; i < boxes.size(); i++) {
				collisionDetection::correctCollisionBoxes(&camera, &boxes[i]);
			}
			gfx.setCameraPos(camera.position);
			gfx.setCameraPos(gfx.getCameraPos()+camera.velocity);
			if (input.keys.f && !lastF) {
				gfx.addRenderInstance(gfx.getCameraPos().x, gfx.getCameraPos().y, gfx.getCameraPos().z, 3);
				test.dimensions = glm::vec3(1, 1, 1);
				test.velocity = glm::vec3(0, 0, 0);
				test.position = glm::vec3(gfx.getCameraPos().x, gfx.getCameraPos().y, gfx.getCameraPos().z);
				boxes.push_back(test);
			}
			if (input.keys.n1 && !last1) {
				gfx.addLight(gfx.getCameraPos(), glm::vec3(2.0, 0.0,0.0), 1);
			}
			if (input.keys.n2 && !last2) {
				gfx.addLight(gfx.getCameraPos(), glm::vec3(0.0, 2.0,0.0), 1);
			}
			if (input.keys.n3 && !last3) {
				gfx.addLight(gfx.getCameraPos(), glm::vec3(0.0, 0.0, 2.0), 1);
			}
			lastF = input.keys.f;
			last1 = input.keys.n1;
			last2 = input.keys.n2;
			last3 = input.keys.n3;
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