#pragma once

#include "TextArea.h"
#include "Transform.h"

class Note {

public:

	Transform transform;
	TextArea textArea;

	inline static float width = 2.0f;
	inline static float height = 2.0f;

};