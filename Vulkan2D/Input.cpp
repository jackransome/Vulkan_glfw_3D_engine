#include "Input.h"

void Input::init(GLFWwindow * window)
{
	windowPointer = window;
}
//======================================================================||
// function that uses glfw to get key inputs (presses and releases) and ||
// uses them to set the keyValues sturct's properties, so that anywhere ||
// I to know what keys are pressed, I can just find out using the       ||
// keyValues struct.                                                    ||
//======================================================================||

void Input::run() {
	glfwPollEvents();
	glfwGetCursorPos(windowPointer, &xpos, &ypos);
	if (inFocus) {

		glfwSetCursorPos(windowPointer, windowWidth / 2, windowHeight / 2);
		cameraAngle.x += 0.003 * float(windowWidth / 2 - xpos);
		cameraAngle.y += 0.003 * float(windowHeight / 2 - ypos);
		if (cameraAngle.y < -3.14 / 2) {
			cameraAngle.y = -3.14 / 2;
		}
		if (cameraAngle.y > 3.14 / 2) {
			cameraAngle.y = 3.14 / 2;
		}
		glfwSetInputMode(windowPointer, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else {
		glfwSetInputMode(windowPointer, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	glfwGetFramebufferSize(windowPointer, &windowWidth, &windowHeight);
	mousePosInWindow = glm::vec2(2 * (xpos - windowWidth / 2), 2 * (ypos - windowHeight / 2));
}

void Input::clearInputString() {
	inputString = "";
}
//Callback function for scrolling with the mouse
void Input::scroll_callback(double xoffset, double yoffset)
{
	if (yoffset != 0) {
		mouseScrollValue = yoffset;
	}
}
//Callback function for all keyboard input
void Input::key_callback(int key, int scancode, int action, int mods)
{
	//Code for allowing the user to type out strings (for a textbox)
	if (stringInputEnabled) {
		if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_A ||
				key == GLFW_KEY_B ||
				key == GLFW_KEY_C ||
				key == GLFW_KEY_D ||
				key == GLFW_KEY_E ||
				key == GLFW_KEY_F ||
				key == GLFW_KEY_G ||
				key == GLFW_KEY_H ||
				key == GLFW_KEY_I ||
				key == GLFW_KEY_J ||
				key == GLFW_KEY_K ||
				key == GLFW_KEY_L ||
				key == GLFW_KEY_M ||
				key == GLFW_KEY_N ||
				key == GLFW_KEY_O ||
				key == GLFW_KEY_P ||
				key == GLFW_KEY_Q ||
				key == GLFW_KEY_R ||
				key == GLFW_KEY_S ||
				key == GLFW_KEY_T ||
				key == GLFW_KEY_U ||
				key == GLFW_KEY_V ||
				key == GLFW_KEY_W ||
				key == GLFW_KEY_X ||
				key == GLFW_KEY_Y ||
				key == GLFW_KEY_Z ||
				key == GLFW_KEY_1 ||
				key == GLFW_KEY_2 ||
				key == GLFW_KEY_3 ||
				key == GLFW_KEY_4 ||
				key == GLFW_KEY_5 ||
				key == GLFW_KEY_6 ||
				key == GLFW_KEY_7 ||
				key == GLFW_KEY_8 ||
				key == GLFW_KEY_9 ||
				key == GLFW_KEY_0 ||
				key == GLFW_KEY_MINUS
				) {
				if (keys.leftShift || keys.rightShift) {
					if (key == GLFW_KEY_MINUS) {
						inputString += "_";
					}
					else {
						inputString += glfwGetKeyName(key, 0)[0];
					}
				}
				else {
					if (glfwGetKeyName(key, 0)[0] > 64) {
						inputString += glfwGetKeyName(key, 0)[0] + 32;
					}
					else {
						if (key == GLFW_KEY_MINUS) {
							inputString += "-";
						}
						else {
							inputString += glfwGetKeyName(key, 0)[0];
						}
					}
				}
			}
			if (key == GLFW_KEY_SPACE) {
				inputString += " ";
			}
			if (key == GLFW_KEY_BACKSPACE && inputString.size() > 0) {
				inputString.resize(inputString.size() - 1);
				if (keys.ctrl) {
					inputString = "";
				}
			}
		}
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		keys.n1 = true;
	}
	if (key == GLFW_KEY_1 && action == GLFW_RELEASE) {
		keys.n1 = false;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		keys.n2 = true;
	}
	if (key == GLFW_KEY_2 && action == GLFW_RELEASE) {
		keys.n2 = false;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		keys.n3 = true;
	}
	if (key == GLFW_KEY_3 && action == GLFW_RELEASE) {
		keys.n3 = false;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		keys.n4 = true;
	}
	if (key == GLFW_KEY_4 && action == GLFW_RELEASE) {
		keys.n4 = false;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		keys.n5 = true;
	}
	if (key == GLFW_KEY_5 && action == GLFW_RELEASE) {
		keys.n5 = false;
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
		keys.n6 = true;
	}
	if (key == GLFW_KEY_6 && action == GLFW_RELEASE) {
		keys.n6 = false;
	}
	if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
		keys.n7 = true;
	}
	if (key == GLFW_KEY_7 && action == GLFW_RELEASE) {
		keys.n7 = false;
	}
	if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
		keys.n8 = true;
	}
	if (key == GLFW_KEY_8 && action == GLFW_RELEASE) {
		keys.n8 = false;
	}
	if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
		keys.n9 = true;
	}
	if (key == GLFW_KEY_9 && action == GLFW_RELEASE) {
		keys.n9 = false;
	}
	if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
		keys.n0 = true;
	}
	if (key == GLFW_KEY_0 && action == GLFW_RELEASE) {
		keys.n0 = false;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		keys.escape = true;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		keys.escape = false;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		keys.space = true;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
		keys.space = false;
	}

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		keys.leftShift = true;
	}
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
		keys.leftShift = false;
	}
	if (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_PRESS) {
		keys.rightShift = true;
	}
	if (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_RELEASE) {
		keys.rightShift = false;
	}

	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
		keys.ctrl = true;
	}
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
		keys.ctrl = false;
	}

	if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
		keys.tab = true;
	}
	if (key == GLFW_KEY_TAB && action == GLFW_RELEASE) {
		keys.tab = false;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		keys.w = true;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		keys.a = true;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		keys.s = true;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		keys.d = true;
	}
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		keys.f = true;
	}
	if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		keys.w = false;
	}
	if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		keys.a = false;
	}
	if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		keys.s = false;
	}
	if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		keys.d = false;
	}
	if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
		keys.f = false;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		keys.q = true;
	}
	if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
		keys.q = false;
	}
}
//Callback function for mouse click input
void Input::mouse_button_callback(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		keys.mouseLeft = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		keys.mouseLeft = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		keys.mouseRight = true;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		keys.mouseRight = false;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
		keys.mouseMiddle = true;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
		keys.mouseMiddle = false;
	}
}

//Another callback, this time for detecting wether or not the window is in focus
void Input::window_focus_callback(int focused)
{
	if (focused)
	{
		inFocus = true;
	}
	else
	{
		inFocus = false;
	}
}