#pragma once

#include <functional>
#include <vector>

#include "Input.h"
#include "Transform.h"
#include "TextArea.h"

enum class Action {

	ChangedNotePosition,
	ChangedCameraPosition,
	SelectedEntity

};

class ActionArgs {
public:
	void* args;
};

class ActionFunc {

public:
	Action actionType;

	std::function<void(void*)> undoFunction;
	std::function<void(void*)> redoFunction;

};

class UndoRedo {

public:

	static void AddAction(const ActionFunc& actionFunc, const ActionArgs _args) {

		if (curAction + 1 < actions.size()) {

			while (actions.size() > curAction + 1) {
				actions.pop_back();
				actionArgs.pop_back();
			}

		}
		actions.push_back(actionFunc);
		actionArgs.push_back(_args);
		curAction++;
	}

	static void Undo() {

		if (curAction >= 0) {
			actions[curAction].undoFunction(actionArgs[curAction].args);
			curAction--;
			std::cout << curAction << std::endl;
		}
		//actions.pop_back();
		//actionArgs.pop_back();
	}

	static void Redo() {
		if (curAction < actions.size()) {
			actions[curAction + 1].redoFunction(actionArgs[curAction + 1].args);
			curAction++;
		}
	}

private:
	inline static float curAction = -1.0f;
	inline static std::vector<ActionFunc> actions;
	inline static std::vector<ActionArgs> actionArgs;
};

struct MousePickingMoving {
	glm::vec3 oldNotesPos;
	glm::vec3 oldTextAreaPos;
	int entity;
	std::vector<Note>& notes;
	std::vector<float>& notesTransformsFlattened;
	VBO& notesTransformsVBO;
	VBO& textAreaTransformsVBO;
	glm::vec3 offset;

};

void UndoMousePickingMoving(void* _mpm) {

	MousePickingMoving* mpm = (MousePickingMoving*)_mpm;

	int i = mpm->entity;
	mpm->notes[i].transform.position = mpm->oldNotesPos;
	float* curNewTrans = glm::value_ptr(*mpm->notes[i].transform.CalculateTransformMatr());
	memcpy(&mpm->notesTransformsFlattened[i * (int)16], curNewTrans, 64);

	/*float newTrans[16];
	memcpy(&newTrans[0], curNewTrans, 64);*/

	mpm->notesTransformsVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, i * 16 * sizeof(float), sizeof(float) * 16, curNewTrans);
	mpm->notesTransformsVBO.Unbind();

	
	const int start = mpm->notes[i].textArea.flattenedTransformStartIndex;
	const int end = mpm->notes[i].textArea.flattenedTransformEndIndex;

	glm::vec3 offset = mpm->oldTextAreaPos - mpm->notes[i].textArea.transform.position;
	mpm->notes[i].textArea.transform.position = mpm->oldTextAreaPos;

	std::vector<float> values;
	mpm->notes[i].textArea.FillGlobalTextArrays(values, offset, start, end);

	const float arraySize = mpm->notes[i].textArea.glyphTrans.size() * 16;

	mpm->textAreaTransformsVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, start * sizeof(float), sizeof(float) * arraySize, &mpm->notes[i].textArea.textTransformsFlattened[start]);
	mpm->textAreaTransformsVBO.Unbind();

}