#pragma once

#include <vector>
#include <unordered_map>
#include <functional>

#include <iostream>

#include "Transform.h"
#include "TextArea.h"

enum class UI_TYPE {

	BUTTON,
	CHECKBOX,
	SCROLL_RECT,
	RECT,
	PANEL,
	TEXT_AREA,
	MANAGER
};

class UI {

public:

	int entityID = -1;

	UI_TYPE type;

	Transform transform;
	TextArea textArea;

	virtual void OnHover() = 0;
	virtual void OnClick(bool leftClick, bool rightClick, std::function<void(void*)> onClick_callback_func, void* args) = 0;
	virtual void OnRelease(bool leftRelease, bool rightRelease) = 0;

	static int numUIComp;
	static std::unordered_map<int, UI*> ui_created_components;
};


struct MouseData {
	float mouseXpos;
	float mouseYpos;
	bool leftClick;
	bool rightClick;
	bool leftRelease;
	bool rightRelease;
};

class UI_MANAGER {

public:

	void ManageUI(MouseData mp,
		std::function<void(void*)> onClick_callback_func,
		void* args,
		int uiLastSelected) {

		if (uiLastSelected != -1) {
			//std::cout << "CHECKING: " << uiLastSelected << std::endl;

			if (mp.leftClick || mp.rightClick)
				UI::ui_created_components[uiLastSelected]->OnClick(mp.leftClick, mp.rightClick, onClick_callback_func, args);
			else if (mp.leftRelease || mp.rightRelease)
				UI::ui_created_components[uiLastSelected]->OnRelease(mp.leftRelease, mp.rightRelease);
			else
				UI::ui_created_components[uiLastSelected]->OnHover();
		}

	}

};

class Button : public UI {

	UI* nextUIItem;
	std::vector<UI*> childrenItems;

public:

	void init(int _entityID) {
		entityID = _entityID;
		//std::cout << "REGISTERED : " << entityID << std::endl;
		ui_created_components.insert({ entityID, this });
		type = UI_TYPE::BUTTON;
	}

	virtual void OnHover() override {
		std::cout << "Hovering over : " << entityID << std::endl;
	}

	virtual void OnClick(bool leftClick, bool rightClick
		, std::function<void(void*)> onClick_callback_func, void* args) override {
		//std::cout << "Clicking : " << entityID << std::endl;
		onClick_callback_func(args);
	}

	virtual void OnRelease(bool leftRelease, bool rightRelease) override {
		std::cout << "Released click on : " << entityID << std::endl;
	}


};