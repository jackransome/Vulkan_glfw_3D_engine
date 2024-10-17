#pragma once

#include <GLFW\glfw3.h>
#include <string>
#include <glm\glm.hpp>

struct keyValues { bool w = false; bool a = false; bool s = false; bool d = false; bool f = false; bool q = false; bool leftShift = false; bool rightShift = false; bool space = false; bool ctrl = false; bool tab = false; bool n1 = false; bool n2 = false; bool n3 = false; bool n4 = false; bool n5 = false; bool n6 = false; bool n7 = false; bool n8 = false; bool n9 = false; bool n0 = false;   bool mouseLeft = false; bool mouseRight = false; bool mouseMiddle = false; bool escape = false; };

class Input {
public:
	std::string inputString = "";
	keyValues keys;
	//int mouseScrollValue = 0;
	bool stringInputEnabled = true;
	bool EXIT = false;
	void run();
	double xpos, ypos;
	glm::vec3 cameraAngle = glm::vec3(0, 0, 0);
	int windowWidth, windowHeight;
	glm::vec2 mousePosInWindow;
	void init(GLFWwindow* window);
	GLFWwindow* windowPointer;
	void clearInputString();
	void scroll_callback(double xoffset, double yoffset);
	void key_callback(int key, int scancode, int action, int mods);
	void mouse_button_callback(int button, int action, int mods);
	bool inFocus = true;
	void window_focus_callback(int focused);
private:
	int mouseScrollValue = 0;
};
