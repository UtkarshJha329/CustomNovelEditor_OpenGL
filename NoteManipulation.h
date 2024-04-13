#pragma once

#include <vector>
#include "Note.h"
#include "UndoRedo.h"

int NUM_NOTES = 5;
int NUM_UI_PANELS = 5;

std::vector<int> linesSelectedEntities;
std::vector<int> newLinesSelectedEntities;
std::vector<int> deleteLinesSelectedEntities;

int lastSelectedUI = -1;
int lastSelectedEntity = -1;
int lastSelectedEntityDelete = -1;

std::vector<int> deletedNotesEntities;
std::vector<bool> resetText;

int EXTRA_BUFFER_ALLOCATION = 100;

bool recordedMovement = false;

// Vertices coordinates
float vertices[] =
{
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f
};

// Indices for vertices order
unsigned int indicies[] =
{
	0, 1, 3,
	3, 1, 2
};

struct CreateNewNodeButtonStruct {
	std::vector<Note>& notes;
	std::vector<float>& notesTransformsFlattened;
	std::vector<float>& visible;
	VBO& notesVisibilityVBO;
	int& lastSelectedEntity;
};

void CreateNewNote(void* args) {
	CreateNewNodeButtonStruct* a = (CreateNewNodeButtonStruct*)args;


	int i = -1;
	bool newNote = deletedNotesEntities.size() == 0;

	if (newNote) {
		Note newNote;
		a->notes.push_back(newNote);
		NUM_NOTES++;
		i = NUM_NOTES - 1;

		//std::vector<float> values;

		//a->notes[i].textArea.individualLengths.push_back(a->notes[i].textArea.sampleString.length());
		//a->notes[i].textArea.IsUI = 0.0f;
		//Debug_Log(a->notes[i].textArea.texCoords.size());
		//a->notes[i].textArea.FillGlobalTextArrays(values);

		//resetText.push_back(true);
		//a->visible.push_back(1.0f);

		//a->notes[i].textArea.BindVAOsVBOsEBOs(vertices, indicies, EXTRA_BUFFER_ALLOCATION);

	}
	else {
		i = deletedNotesEntities[deletedNotesEntities.size() - 1];
		deletedNotesEntities.pop_back();

		//Debug_Log("Used old deleted node.");
		a->notes[i].textArea.SetVisibility(i + NUM_UI_PANELS, 1.0f);

		a->notes[i].textArea.textIsVisibleVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * a->notes[i].textArea.textIsVisible.size(), &a->notes[i].textArea.textIsVisible[0]);
		a->notes[i].textArea.textIsVisibleVBO.Unbind();
		a->visible[i] = 1.0f;

	}

	a->notes[i].width = 1.0f;
	a->notes[i].height = 1.0f;
	a->notes[i].transform.scale = glm::vec3(a->notes[i].width, a->notes[i].height, 1.0f);

	float* head = glm::value_ptr(*a->notes[i].transform.CalculateTransformMatr());
	for (int j = 0; j < 16; j++)
	{
		a->notesTransformsFlattened.push_back(head[j]);
	}

	a->lastSelectedEntity = i + NUM_UI_PANELS;


	a->notes[i].textArea.transform = a->notes[i].transform;
	a->notes[i].textArea.width = a->notes[i].width;
	a->notes[i].textArea.height = a->notes[i].height;
	a->notes[i].textArea.sampleString = "To dispriz'd coil, and be: to othe mind by a life, and love, and mome of somenterprises";

	//Debug(a->notes[i].textArea.transform.position);
	//Debug_Log(a->notes[i].textArea.texCoords.size());

	if (newNote) {

		std::vector<float> values;

		a->notes[i].textArea.individualLengths.push_back(a->notes[i].textArea.sampleString.length());
		a->notes[i].textArea.IsUI = 0.0f;
		//Debug_Log(a->notes[i].textArea.texCoords.size());
		a->notes[i].textArea.FillGlobalTextArrays(values);

		resetText.push_back(true);
		a->visible.push_back(1.0f);


		a->notes[i].textArea.BindVAOsVBOsEBOs(vertices, indicies, EXTRA_BUFFER_ALLOCATION);

	}

	ChangeVisibility* cnv = new ChangeVisibility{
		a->notes,
		a->visible,
		deletedNotesEntities,
		a->notesVisibilityVBO,
		i + NUM_UI_PANELS,
		NUM_UI_PANELS
	};

	ActionFunc af;
	af.actionType = Action::UndoCreationOrDeletionOfNote;
	af.redoFunction = ChangeNoteVisibility;
	af.undoFunction = ChangeNoteVisibility;

	ActionArgs aa = {
		cnv
	};

	UndoRedo::AddAction(af, aa);
}

struct DeleteNoteInfo {
	int& lastSelectedEntityDelete;
	std::vector<Note>& notes;
	std::vector<float>& notesVisible;
	VBO& notesVisibilityVBO;
};

void DeleteNote(void* args) {
	DeleteNoteInfo* a = (DeleteNoteInfo*)args;
	float* visibility = new float(0.0f);
	int i = a->lastSelectedEntityDelete - NUM_UI_PANELS;
	a->notesVisible[i] = 0.0f;
	//Debug_Log("Deleted Note: " << i);

	//Debug_Log("LastSelectedEntityDelete: " << lastSelectedEntityDelete << ", NumIndividualLengths: " << TextArea::individualLengths.size());
	a->notes[i].textArea.SetVisibility(lastSelectedEntityDelete, 0.0f);

	deletedNotesEntities.push_back(i);

	a->lastSelectedEntityDelete = -1.0f;


	a->notesVisibilityVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * a->notesVisible.size(), a->notesVisible.data());
	a->notesVisibilityVBO.Unbind();

	float bufferWriteOffset = 0.0f;
	for (int j = 0; j < i; j++)
	{
		bufferWriteOffset += a->notes[i].textArea.individualLengths[j];
	}
	//Debug_Log(bufferWriteOffset);

	a->notes[i].textArea.textIsVisibleVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * a->notes[i].textArea.textIsVisible.size(), &a->notes[i].textArea.textIsVisible[0]);
	a->notes[i].textArea.textIsVisibleVBO.Unbind();

	for (int k = 0; k < linesSelectedEntities.size(); k++)
	{
		if (linesSelectedEntities[k] == i) {
			if (k % 2 == 1) {
				linesSelectedEntities[k] = linesSelectedEntities[k - 1];
			}
			else {
				linesSelectedEntities[k] = linesSelectedEntities[k + 1];
			}
		}
	}

	ChangeVisibility* cnv = new ChangeVisibility{
		a->notes,
		a->notesVisible,
		deletedNotesEntities,
		a->notesVisibilityVBO,
		i + NUM_UI_PANELS,
		NUM_UI_PANELS
	};

	ActionFunc af;
	af.actionType = Action::UndoCreationOrDeletionOfNote;
	af.redoFunction = ChangeNoteVisibility;
	af.undoFunction = ChangeNoteVisibility;

	ActionArgs aa = {
		cnv
	};

	UndoRedo::AddAction(af, aa);

}


void MoveNotes(std::vector<Note>& notes, std::vector<float>& notesTransformsFlattened
	, glm::vec3 point, VBO& notesTransformsVBO, VBO& notesVisibilityVBO,
	int i, UndoRedo& undoredo, bool allowRecording = true) {

	glm::vec3 oldNotesPos = notes[i].transform.position;
	point += glm::vec3(0.0f, 0.0f, 0.2f);
	notes[i].transform.position = point;
	float* curNewTrans = glm::value_ptr(*notes[i].transform.CalculateTransformMatr());
	memcpy(&notesTransformsFlattened[i * (int)16], curNewTrans, 64);

	notesTransformsVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, i * 16 * sizeof(float), sizeof(float) * 16, curNewTrans);
	notesTransformsVBO.Unbind();

	float* visibiblity = new float(1.0f);
	notesVisibilityVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(float), sizeof(float), visibiblity);
	notesVisibilityVBO.Unbind();

	const int start = notes[i].textArea.flattenedTransformStartIndex;
	const int end = notes[i].textArea.flattenedTransformEndIndex;
	std::vector<float> values;

	glm::vec3 oldTextAreaPos = notes[i].textArea.transform.position;
	notes[i].textArea.transform.position = point;


	glm::vec3 offset = point - oldTextAreaPos;
	if (resetText[i] && glm::length(offset) > 0.1f) {
		offset -= glm::vec3(notes[i].textArea.width, -notes[i].textArea.height, -0.01f);
		resetText[i] = false;
	}

	notes[i].textArea.FillGlobalTextArrays(values, offset, start, end);

	const float arraySize = notes[i].textArea.glyphTrans.size() * 16;

	TextArea::textTransformsVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, start * sizeof(float), sizeof(float) * arraySize, &notes[i].textArea.textTransformsFlattened[start]);
	TextArea::textTransformsVBO.Unbind();

	if (!recordedMovement && allowRecording) {
		MousePickingMoving* mpm = new MousePickingMoving{
				oldNotesPos,
				oldTextAreaPos,
				point,
				point,
				lastSelectedEntity - NUM_UI_PANELS,
				notes,
				notesTransformsFlattened,
				notesTransformsVBO,
				TextArea::textTransformsVBO,
				offset,
		};

		ActionFunc notesMovingAF;
		notesMovingAF.actionType = Action::ChangedNotePosition;
		void* a = (void*)mpm;
		notesMovingAF.undoFunction = UndoMousePickingMoving;
		notesMovingAF.redoFunction = RedoMousePickingMoving;

		ActionArgs aa{
			a
		};

		undoredo.AddAction(notesMovingAF, aa);
		recordedMovement = true;
	}
}
