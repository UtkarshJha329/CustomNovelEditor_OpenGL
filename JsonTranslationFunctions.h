#pragma once

#include "Transform.h"
#include "TextArea.h"
#include "Note.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

void to_json(json& j, const Transform& trans) {
	j = json{ {"position.x", trans.position.x}, {"position.y", trans.position.y}, {"position.z", trans.position.z}
	, {"rotation.x", trans.rotation.x}, {"rotation.y", trans.rotation.y}, {"rotation.z", trans.rotation.z}
	, {"scale.x", trans.scale.x}, {"scale.y", trans.scale.y}, {"scale.z", trans.scale.z} };
}

void from_json(const json& j, Transform& trans) {
	j.at("position.x").get_to(trans.position.x);
	j.at("position.y").get_to(trans.position.y);
	j.at("position.z").get_to(trans.position.z);
	j.at("rotation.x").get_to(trans.rotation.x);
	j.at("rotation.y").get_to(trans.rotation.y);
	j.at("rotation.z").get_to(trans.rotation.z);
	j.at("scale.x").get_to(trans.scale.x);
	j.at("scale.y").get_to(trans.scale.y);
	j.at("scale.z").get_to(trans.scale.z);
}

void to_json(json& j, const TextArea& textArea) {
	j = json{ {"sampleString", textArea.sampleString}
			, {"trans", textArea.transform}
			, {"IsUI", textArea.IsUI}
			, {"ID", textArea.ID} };
}

void from_json(const json& j, TextArea& textArea) {
	j.at("sampleString").get_to(textArea.sampleString);
	j.at("trans").get_to(textArea.transform);
	j.at("IsUI").get_to(textArea.IsUI);
	j.at("ID").get_to(textArea.ID);
}

void to_json(json& j, const Note& note) {
	j = json{ {"noteTextArea", note.textArea}
		, {"noteTransform", note.transform} };
}

void from_json(const json& j, Note& note) {
	j.at("noteTextArea").get_to(note.textArea);
	j.at("noteTransform").get_to(note.transform);
}
