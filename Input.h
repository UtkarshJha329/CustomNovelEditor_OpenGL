#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <unordered_map>

enum class KeyCode {
	SPACE = 32, APOSTROPHE, COMMA = 44, MINUS, PERIOD, SLASH,																						// 6
	ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,																						// 10
	SEMICOLON = 59, EQUAL = 61,																														// 2
	A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,																// 26
	LEFT_BRACKET, BACKSLASH, RIGHT_BRACKET, GRAVE_ACCENT = 96,																						// 4
	WORLD_1 = 161, WORLD_2,																															// 2
	ESCAPE = 256, ENTER, TAB, BACKSPACE, INSERT, DELETE,																							// 6
	RIGHT_ARROW, LEFT_ARRORW, DOWN_ARROW, UP_ARROW,																									// 4
	PAGE_UP, PAGE_DOWN, HOME, END,																													// 4
	CAPS_LOCK = 280, SCROLL_LOCK, NUM_LOCK, PRINT_SCREEN, PAUSE,																					// 5
	F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,						// 25
	KP_0 = 320, KP_1, KP_2, KP_3, KP_4, KP_5, KP_6, KP_7, KP_8, KP_9, KP_DECIMAL, KP_DIVIDE, KP_MULTIPLY, KP_SUBTRACT, KP_ADD, KP_ENTER, KP_EQUAL,	// 17
	LEFT_SHIFT = 340, LEFT_CTRL, LEFT_ALT, LEFT_SUPER, RIGHT_SHIFT, RIGHT_CTRL, RIGHT_ALT, RIGHT_SUPER, MENU										// 9 
};

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

	static bool KeyUp(GLFWwindow* gameWindow, KeyCode key) {
		return (glfwGetKey(gameWindow, (int)key) == GLFW_RELEASE);
	}

	static bool KeyDown(GLFWwindow* gameWindow, KeyCode key) {
		return keyDown[key];
	}

	static bool KeyHeld(GLFWwindow* gameWindow, KeyCode key) {
		return (glfwGetKey(gameWindow, (int)key) == GLFW_PRESS);
	}
	
	static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key < 0) {
			return;
		}

		KeyCode curKeyCode = static_cast<KeyCode>(key);

		if (action == GLFW_PRESS) {
			keyDown[curKeyCode] = true;
		}
	}

	static bool leftMouseButtonPressed;
	static bool rightMouseButtonPressed;
	static bool leftMouseButtonHeld;
	static bool rightMouseButtonHeld;
	static bool leftMouseButtonReleased;
	static bool rightMouseButtonReleased;

	static float mouseX, mouseY;

	static const unsigned int numKeyCodes = 120;
	static std::unordered_map<KeyCode, bool> keyDown;
	static KeyCode keyCodesVec[];

private:

	static bool old_leftMouseButtonPressed;
	static bool old_rightMouseButtonPressed;
	static bool old_leftMouseButtonHeld;
	static bool old_rightMouseButtonHeld;
	static bool old_leftMouseButtonReleased;
	static bool old_rightMouseButtonReleased;


};