#pragma once

#include "TextArea.h"
#include "Transform.h"

#include "ShaderClass.h"

class Note {

public:

	Transform transform;
	TextArea textArea;

	static std::vector<Note> notes;
	static std::vector<float> flattenedTransforms;
	static std::vector<float> visible;

	inline static Shader* notesShaderProgram;

	inline static float width = 1.0f;
	inline static float height = 1.0f;

	inline static VAO notesVAO;
	inline static VBO notesQuadVBO;
	inline static VBO notesflattenedTransformsVBO;
	inline static VBO notesVisibileVBO;
	inline static EBO notesEBO;

	static void InitVAOsVBOsEBOs(float* vertices, unsigned int* indicies, int EXTRA_BUFFER_ALLOCATION) {
		
		notesVAO.Init();
		notesVAO.Bind();

		notesQuadVBO.Init(vertices, sizeof(float) * 12, GL_STATIC_DRAW);
		//VBO notesTransformsVBO(offsets.data(), offsets.size());
		notesflattenedTransformsVBO.Init(flattenedTransforms.data(), sizeof(float) * flattenedTransforms.size() + EXTRA_BUFFER_ALLOCATION * 16 * sizeof(float), GL_DYNAMIC_DRAW);
		notesVisibileVBO.Init(visible.data(), sizeof(float) * visible.size() + EXTRA_BUFFER_ALLOCATION * sizeof(float), GL_DYNAMIC_DRAW);
		//std::cout << notesTransformsFlattened.size() / 16 << std::endl;
		notesflattenedTransformsVBO.Bind();

		notesEBO.Init(indicies, sizeof(unsigned int) * 6);
		notesVAO.LinkTransformAttrib(notesflattenedTransformsVBO, 1);
		notesVAO.LinkAttrib(notesQuadVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
		notesVAO.LinkAttrib(notesVisibileVBO, 5, 1, GL_FLOAT, sizeof(float), (void*)0);
		glVertexAttribDivisor(5, 1);

		notesVAO.Unbind();
		notesQuadVBO.Unbind();
		notesflattenedTransformsVBO.Unbind();
		notesVisibileVBO.Unbind();
		notesEBO.Unbind();
	}
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

struct ChangeVisibility {
	std::vector<Note>& notes;
	std::vector<float>& notesVisible;
	std::vector<int>& deletedNotes;
	VBO& notesVisibilityVBO;
	int entityToChange;
	int num_ui_panels;
};

void ChangeNoteVisibility(void* _cnv) {

	ChangeVisibility* cnv = (ChangeVisibility*)_cnv;

	cnv->notes[cnv->entityToChange - cnv->num_ui_panels].textArea.FlipVisibility(cnv->entityToChange);

	float curValue = cnv->notesVisible[cnv->entityToChange - cnv->num_ui_panels];
	if (curValue == 1.0f)
	{
		cnv->notesVisible[cnv->entityToChange - cnv->num_ui_panels] = 0.0f;
	}
	else {
		cnv->notesVisible[cnv->entityToChange - cnv->num_ui_panels] = 1.0f;
		if (cnv->deletedNotes.size() > 0) {
			cnv->deletedNotes.pop_back();
		}
	}
	//Debug_Log("Deleted Note: " << i);

	cnv->notesVisibilityVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * cnv->notesVisible.size(), cnv->notesVisible.data());
	cnv->notesVisibilityVBO.Unbind();

}