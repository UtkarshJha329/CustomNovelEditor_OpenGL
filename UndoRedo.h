#pragma once

#include <functional>
#include <vector>

#include "Input.h"
#include "Transform.h"
#include "TextArea.h"

bool once = true;

enum class Action {

	ChangedNotePosition,
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
		//std::cout << "ADDED NEW UNDO MOVE." << std::endl;
		actions.push_back(actionFunc);
		actionArgs.push_back(_args);
		curAction++;
	}

	static void Undo() {

		//std::cout << "Outside UNDO: " << curAction + 1 << ", " << actions.size() << std::endl;
		if (curAction >= 0) {
			if (curAction < actions.size()) {
				actions[curAction].undoFunction(actionArgs[curAction].args);
				//std::cout << "Undo: " << curAction << ", " << actions.size() << std::endl;
				curAction--;
			}
		}
		//actions.pop_back();
		//actionArgs.pop_back();
	}

	static void Redo() {
		//std::cout << "Outside REDO: " << curAction + 1 << ", " << actions.size() << std::endl;
		if (curAction + 1 < actions.size()) {
			actions[curAction + 1].redoFunction(actionArgs[curAction + 1].args);
			//std::cout << "Redo: " << curAction << ", " << actions.size() << std::endl;
			curAction++;
		}
	}

	void Flush() {
		actions.clear();
		actionArgs.clear();
		curAction = -1;
	}

private:
	inline static float curAction = -1.0f;
	inline static std::vector<ActionFunc> actions;
	inline static std::vector<ActionArgs> actionArgs;
};

struct MousePickingMoving {
	glm::vec3 oldNotesPos;
	glm::vec3 oldTextAreaPos;
	glm::vec3 newNotesPos;
	glm::vec3 newTextAreaPos;
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
	
	mpm->newTextAreaPos = mpm->notes[i].textArea.transform.position;
	mpm->newNotesPos = mpm->notes[i].transform.position;

	mpm->notes[i].transform.position = mpm->oldNotesPos;
	float* curNewTrans = glm::value_ptr(*mpm->notes[i].transform.CalculateTransformMatr());
	memcpy(&mpm->notesTransformsFlattened[i * (int)16], curNewTrans, 64);

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

void RedoMousePickingMoving(void* _mpm) {

	MousePickingMoving* mpm = (MousePickingMoving*)_mpm;

	int i = mpm->entity;
	mpm->notes[i].transform.position = mpm->newNotesPos;
	float* curNewTrans = glm::value_ptr(*mpm->notes[i].transform.CalculateTransformMatr());
	memcpy(&mpm->notesTransformsFlattened[i * (int)16], curNewTrans, 64);

	/*float newTrans[16];
	memcpy(&newTrans[0], curNewTrans, 64);*/

	mpm->notesTransformsVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, i * 16 * sizeof(float), sizeof(float) * 16, curNewTrans);
	mpm->notesTransformsVBO.Unbind();

	const int start = mpm->notes[i].textArea.flattenedTransformStartIndex;
	const int end = mpm->notes[i].textArea.flattenedTransformEndIndex;

	glm::vec3 offset = mpm->newTextAreaPos - mpm->notes[i].textArea.transform.position;
	mpm->notes[i].textArea.transform.position = mpm->newTextAreaPos;

	std::vector<float> values;
	mpm->notes[i].textArea.FillGlobalTextArrays(values, offset, start, end);

	const float arraySize = mpm->notes[i].textArea.glyphTrans.size() * 16;

	mpm->textAreaTransformsVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, start * sizeof(float), sizeof(float) * arraySize, &mpm->notes[i].textArea.textTransformsFlattened[start]);
	mpm->textAreaTransformsVBO.Unbind();

}