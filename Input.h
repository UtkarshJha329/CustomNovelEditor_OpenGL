#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Input {

public:

	inline static void SetMouseInput(GLFWwindow* window) {

		old_leftMouseButtonPressed	= leftMouseButtonPressed;
		old_rightMouseButtonPressed = rightMouseButtonPressed;
		old_leftMouseButtonHeld = leftMouseButtonHeld;
		old_rightMouseButtonHeld = rightMouseButtonHeld;
		old_leftMouseButtonReleased = leftMouseButtonReleased;
		old_rightMouseButtonReleased = rightMouseButtonReleased;

		leftMouseButtonPressed = false;
		rightMouseButtonPressed = false;
		leftMouseButtonHeld = false;
		rightMouseButtonHeld = false;
		leftMouseButtonReleased = false;
		rightMouseButtonReleased = false;

		leftMouseButtonReleased = old_leftMouseButtonPressed && !glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		rightMouseButtonReleased = old_rightMouseButtonPressed && !glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
		
		leftMouseButtonHeld = (old_leftMouseButtonPressed || old_leftMouseButtonHeld)
									&& glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		rightMouseButtonHeld = (old_rightMouseButtonPressed || old_rightMouseButtonHeld)
									&& glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

		leftMouseButtonPressed = !old_leftMouseButtonPressed && !old_leftMouseButtonHeld && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		rightMouseButtonPressed = !old_rightMouseButtonPressed && !old_rightMouseButtonHeld && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

	}


	static bool leftMouseButtonPressed;
	static bool rightMouseButtonPressed;
	static bool leftMouseButtonHeld;
	static bool rightMouseButtonHeld;
	static bool leftMouseButtonReleased;
	static bool rightMouseButtonReleased;

	static float mouseX, mouseY;

private:

	static bool old_leftMouseButtonPressed;
	static bool old_rightMouseButtonPressed;
	static bool old_leftMouseButtonHeld;
	static bool old_rightMouseButtonHeld;
	static bool old_leftMouseButtonReleased;
	static bool old_rightMouseButtonReleased;


};