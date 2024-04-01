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