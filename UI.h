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

	inline static VAO uiVAO;
	inline static VBO uiQuadVBO;
	inline static VBO uiTransformsVBO;
	inline static VBO uiVisibleVBO;
	inline static EBO uiEBO;

	static std::vector<float> ui_transformsFlattened;
	static std::vector<float> ui_visible;

	static Shader uiShaderProgram;


	static void InitVAOsVBOsEBOs(float* vertices, unsigned int* indicies) {

		uiVAO.Init();
		uiVAO.Bind();

		uiQuadVBO.Init(vertices, sizeof(float) * 12, GL_STATIC_DRAW);
		uiTransformsVBO.Init(ui_transformsFlattened.data(), sizeof(float) * ui_transformsFlattened.size(), GL_DYNAMIC_DRAW);
		uiVisibleVBO.Init(ui_visible.data(), sizeof(float) * ui_visible.size(), GL_DYNAMIC_DRAW);
		uiTransformsVBO.Bind();
		uiVisibleVBO.Bind();

		uiEBO.Init(indicies, sizeof(unsigned int) * 6);
		uiVAO.LinkAttrib(uiQuadVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
		uiVAO.LinkTransformAttrib(uiTransformsVBO, 1);
		uiVAO.LinkAttrib(uiVisibleVBO, 5, 1, GL_FLOAT, sizeof(float), (void*)0);
		uiVisibleVBO.Bind();
		glVertexAttribDivisor(5, 1);
		uiVisibleVBO.Unbind();

		uiVAO.Unbind();
		uiQuadVBO.Unbind();
		uiTransformsVBO.Unbind();
		uiEBO.Unbind();
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