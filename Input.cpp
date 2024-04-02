#include "Input.h"

bool Input::leftMouseButtonPressed = false;
bool Input::rightMouseButtonPressed = false;
bool Input::leftMouseButtonHeld = false;
bool Input::rightMouseButtonHeld = false;
bool Input::leftMouseButtonReleased = false;
bool Input::rightMouseButtonReleased = false;

float Input::mouseX = 0.0f, Input::mouseY = 0.0f;

bool Input::old_leftMouseButtonPressed = false;
bool Input::old_rightMouseButtonPressed = false;
bool Input::old_leftMouseButtonHeld = false;
bool Input::old_rightMouseButtonHeld = false;
bool Input::old_leftMouseButtonReleased = false;
bool Input::old_rightMouseButtonReleased = false;

KeyCode Input::keyCodesVec[] = {
	KeyCode::SPACE, KeyCode::APOSTROPHE, KeyCode::COMMA, KeyCode::MINUS, KeyCode::PERIOD, KeyCode::SLASH,																						// 6
	KeyCode::ZERO, KeyCode::ONE, KeyCode::TWO, KeyCode::THREE, KeyCode::FOUR, KeyCode::FIVE, KeyCode::SIX, KeyCode::SEVEN, KeyCode::EIGHT, KeyCode::NINE,																						// 10
	KeyCode::SEMICOLON, KeyCode::EQUAL,																														// 2
	KeyCode::A, KeyCode::B, KeyCode::C, KeyCode::D, KeyCode::E, KeyCode::F, KeyCode::G, KeyCode::H, KeyCode::I, KeyCode::J,KeyCode::K, KeyCode::L, KeyCode::M, KeyCode::N, KeyCode::O, KeyCode::P, KeyCode::Q, KeyCode::R, KeyCode::S, KeyCode::T, KeyCode::U, KeyCode::V, KeyCode::W, KeyCode::X, KeyCode::Y, KeyCode::Z,																// 26
	KeyCode::LEFT_BRACKET, KeyCode::BACKSLASH, KeyCode::RIGHT_BRACKET, KeyCode::GRAVE_ACCENT,																						// 4
	KeyCode::WORLD_1, KeyCode::WORLD_2,																															// 2
	KeyCode::ESCAPE, KeyCode::ENTER, KeyCode::TAB, KeyCode::BACKSPACE, KeyCode::INSERT, KeyCode::DELETE,																							// 6
	KeyCode::RIGHT_ARROW, KeyCode::LEFT_ARRORW, KeyCode::DOWN_ARROW, KeyCode::UP_ARROW,																									// 4
	KeyCode::PAGE_UP, KeyCode::PAGE_DOWN, KeyCode::HOME, KeyCode::END,																													// 4
	KeyCode::CAPS_LOCK, KeyCode::SCROLL_LOCK, KeyCode::NUM_LOCK, KeyCode::PRINT_SCREEN, KeyCode::PAUSE,																					// 5
	KeyCode::F1, KeyCode::F2, KeyCode::F3, KeyCode::F4, KeyCode::F5, KeyCode::F6, KeyCode::F7, KeyCode::F8, KeyCode::F9, KeyCode::F10, KeyCode::F11, KeyCode::F12, KeyCode::F13, KeyCode::F14, KeyCode::F15, KeyCode::F16, KeyCode::F17, KeyCode::F18, KeyCode::F19, KeyCode::F20, KeyCode::F21, KeyCode::F22, KeyCode::F23, KeyCode::F24, KeyCode::F25,						// 25
	KeyCode::KP_0, KeyCode::KP_1, KeyCode::KP_2, KeyCode::KP_3, KeyCode::KP_4, KeyCode::KP_5, KeyCode::KP_6, KeyCode::KP_7, KeyCode::KP_8, KeyCode::KP_9, KeyCode::KP_DECIMAL, KeyCode::KP_DIVIDE, KeyCode::KP_MULTIPLY, KeyCode::KP_SUBTRACT, KeyCode::KP_ADD, KeyCode::KP_ENTER, KeyCode::KP_EQUAL,	// 17
	KeyCode::LEFT_SHIFT, KeyCode::LEFT_CTRL, KeyCode::LEFT_ALT, KeyCode::LEFT_SUPER, KeyCode::RIGHT_SHIFT, KeyCode::RIGHT_CTRL, KeyCode::RIGHT_ALT, KeyCode::RIGHT_SUPER, KeyCode::MENU
};

std::unordered_map<KeyCode, bool> Input::keyDown;