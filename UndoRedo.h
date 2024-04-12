#pragma once

#include <functional>
#include <vector>

#include "Input.h"
#include "Transform.h"
#include "TextArea.h"
#include "Note.h"

enum class Action {

	ChangedNotePosition,
	UndoCreationOrDeletionOfNote,
	UndoCreationOrDeletionOfLinks
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

	static void AddAction(const ActionFunc actionFunc, const ActionArgs _args) {

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

struct Links {
	std::vector<int>& linesSelectedEntities;
	int entityA;
	int entityB;
};

void UndoLinesBetweenEntities(void* _links) {

	Links* links = (Links*)_links;

	for (int i = 0; i + 1 < links->linesSelectedEntities.size(); i++)
	{
		if ((links->linesSelectedEntities[i] == links->entityA && links->linesSelectedEntities[i + 1] == links->entityB)
			|| (links->linesSelectedEntities[i] == links->entityB && links->linesSelectedEntities[i + 1] == links->entityA))
		{
			links->linesSelectedEntities.erase(links->linesSelectedEntities.begin() + i + 1);
			links->linesSelectedEntities.erase(links->linesSelectedEntities.begin() + i);
		}
	}

}

void RedoLinesBetweenEntities(void* _links) {

	Links* links = (Links*)_links;

	links->linesSelectedEntities.push_back(links->entityA);
	links->linesSelectedEntities.push_back(links->entityB);
}