#include <filesystem>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <istream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include "Input.h"

#include "UndoRedo.h"
#include "Note.h"
#include "UI.h"
#include "Camera.h"

#include "BMFontReader.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "JsonTranslationFunctions.h"
#include "Drawing.h"

#include "NoteManipulation.h"

#define MAIN_WINDOW_WIDTH 800																						// Width of Main Window
#define MAIN_WINDOW_HEIGHT 600																						// Height of Main Window

#define LOPE_FAILURE 0
#define LOPE_SUCCESS 10

#define Debug_Log(x) std::cout << x << std::endl

Shader* TextArea::textShader;
std::vector<float> TextArea::textTransformsFlattened;
std::vector<float> TextArea::texCoords;
std::vector<float> TextArea::isUI;
std::vector<float> TextArea::textIsVisible;
int TextArea::totalGlyphs;
int TextArea::totalTextAreas;

std::vector<float> Note::flattenedTransforms(NUM_NOTES * 16);
std::vector<float> Note::visible(NUM_NOTES);
std::vector<Note> Note::notes(NUM_NOTES);

std::vector<float> UI_MANAGER::ui_transformsFlattened(NUM_UI_PANELS * 16);
std::vector<float> UI_MANAGER::ui_visible(NUM_UI_PANELS);
Shader UI_MANAGER::uiShaderProgram;


bool firstMouse = true;
float lastX = 0, lastY = 0;
float mouseDeltaX = 0;
float mouseDeltaY = 0;
float mouseSensitivity = 100.0f;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

int UI::numUIComp = NUM_UI_PANELS;

struct ButtonClickStruct {
	std::vector<float>& buttonsVisibility; int curButton;
};

void ButtonClickTest(void* a) {
	ButtonClickStruct* bcs = (ButtonClickStruct*)a;
	for (int i = 0; i < bcs->buttonsVisibility.size(); i++)
	{
		if (i != bcs->curButton) {
			//std::cout << "Flipped visibility" << std::endl;
			bcs->buttonsVisibility[i] = 1.0f - bcs->buttonsVisibility[i];
		}
	}
}

BMFontReader* TextArea::reader;
bool z = false;
bool y = false;
bool backspace = false;
bool s = false;

void Save() {
	json j;
	j["notes"] = Note::notes;
	j["notes.flattenedTrans"] = Note::flattenedTransforms;
	j["notes.visible"] = Note::visible;
	j["notes.resetText"] = resetText;

	j["textArea.flattenedTrans"] = TextArea::textTransformsFlattened;
	j["textArea.visible"] = TextArea::textIsVisible;
	std::ofstream out("pretty.json");

	out << std::setw(4) << j;
}

void Load(json& inJson) {
	std::ifstream openFile("pretty.json");
	openFile >> inJson;
}

int main()
{
	UndoRedo undoredo;

	BMFontReader reader("./fonts/TestFontDF.fnt");
	reader.read();
	TextArea::reader = &reader;

	json inJson;
	Load(inJson);

	glfwInit();

	UI_MANAGER ui_manager;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, "Little Opengl Engine", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	
	gladLoadGL();
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, Input::keyboard_callback);
	glfwSetCharModsCallback(window, Input::charmods_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

#pragma region VBOs and Positions
	UI_MANAGER::uiShaderProgram.Init("UIBasic.vert", "UIBasic.frag");

	srand((unsigned int)time(0));
	std::vector<Button> buttons(NUM_UI_PANELS);
	float y = -0.8f;
	for (int i = 0; i < NUM_UI_PANELS; i++)
	{
		buttons[i].transform.position.x = 5.0f;
		buttons[i].transform.position = glm::vec3(0.1f, y, 0.0f);
		buttons[i].transform.position.x = (buttons[i].transform.position.x - 0.5f) * 2.0f;
		buttons[i].transform.position *= 1.0f;
		y += 0.4f;
		float sx = (float)rand() / (RAND_MAX);
		float sy = (float)rand() / (RAND_MAX);
		float sz = (float)rand() / (RAND_MAX);
		buttons[i].transform.scale = glm::vec3(0.1f);
		UI_MANAGER::ui_visible[i] = 1.0f;

		float* head = glm::value_ptr(*buttons[i].transform.CalculateTransformMatr());

		std::vector<float> values;

		memcpy(&UI_MANAGER::ui_transformsFlattened[i * (int)16], head, 64);
		buttons[i].init(i);
		buttons[i].textArea.ID = i;
		buttons[i].textArea.transform = buttons[i].transform;
		buttons[i].textArea.transform.position.x -= 0.1f;
		buttons[i].textArea.width = 0.1f;
		buttons[i].textArea.height = 0.1f;
		buttons[i].textArea.sampleString = "BUTTON";
		buttons[i].textArea.individualLengths.push_back(buttons[i].textArea.sampleString.length());
		buttons[i].textArea.IsUI = 1.0f;
		buttons[i].textArea.FillGlobalTextArrays(values);
	}

	Note::notesShaderProgram = new Shader("default.vert", "default.frag");

	srand((unsigned int)time(0));
	
	Note::width = 1.0f;
	Note::height = 1.0f;

	Note::notes = inJson["notes"];
	Note::flattenedTransforms = inJson["notes.flattenedTrans"].get<std::vector<float>>();
	Note::visible = inJson["notes.visible"].get<std::vector<float>>();
	resetText = inJson["notes.resetText"].get<std::vector<bool>>();

	NUM_NOTES = Note::notes.size();

	//j["textArea.flattenedTrans"] = TextArea::textTransformsFlattened;
	//j["textArea.visible"] = TextArea::textIsVisible;

	for (int i = 0; i < NUM_NOTES; i++)
	{
		Note::visible[i] = 1.0f;

		//float x = (float)rand() / (RAND_MAX);
		//float y = (float)rand() / (RAND_MAX);
		//float z = 10.0f;
		//Note::notes[i].transform.position = glm::vec3(x, y, z);
		//Note::notes[i].transform.position.x = (Note::notes[i].transform.position.x - 0.5f) * 2.0f;
		//Note::notes[i].transform.position.y = (Note::notes[i].transform.position.y - 0.5f) * 2.0f;
		//Note::notes[i].transform.position.x *= 5.0f;
		//Note::notes[i].transform.position.y *= 5.0f;

		//Note::notes[i].transform.scale = glm::vec3(Note::width, Note::height, 1.0f);

		//float* head = glm::value_ptr(*Note::notes[i].transform.CalculateTransformMatr());

		//memcpy(&Note::flattenedTransforms[i * (int)16], head, 64);
		std::vector<float> values;


		//Note::notes[i].textArea.ID = i + NUM_UI_PANELS;
		//Note::notes[i].textArea.transform = Note::notes[i].transform;
		//Note::notes[i].textArea.transform.position.z -= 0.2f;
		//notes[i].textArea.transform.position += glm::vec3(-notes[i].width, notes[i].height, 0.0f);
		Note::notes[i].textArea.width = Note::width;
		Note::notes[i].textArea.height = Note::height;
		//Note::notes[i].textArea.sampleString = "To dispriz'd coil, and be: to othe mind by a life, and love, and mome of somenterprises calamity opposing end swear, to dream:";
		//notes[i].textArea.sampleString = "To dispriz'd coil, and be: to othe mind by a life, and love, and mome of somenterprises";
		TextArea::individualLengths.push_back(Note::notes[i].textArea.sampleString.length());
		Note::notes[i].textArea.IsUI = 0.0f;
		Note::notes[i].textArea.FillGlobalTextArrays(values);

		resetText[i] = true;
		//Note::visible[i] = 1.0f;
	}

	//NOTES drawing VAOs and VBOs
	Note::InitVAOsVBOsEBOs(vertices, indicies, EXTRA_BUFFER_ALLOCATION);
	

	for (int i = 0; i < NUM_NOTES; i++)
	{
		for (int j = 0; j < Note::notes[i].textArea.glyphTrans.size(); j++)
		{
			Note::notes[i].textArea.textIsVisible[i * TextArea::NUM_CHARS_IN_TEXTAREA + j] = 1.0f;
		}
		MoveNotes(Note::notes, Note::flattenedTransforms, Note::notes[i].transform.position, Note::notesflattenedTransformsVBO, Note::notesVisibileVBO,  i, undoredo);
		//notesVisible[i] = 0.0f;
	}
	Note::notesVisibileVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * Note::visible.size(), Note::visible.data());
	Note::notesVisibileVBO.Unbind();

	undoredo.Flush();
	recordedMovement = false;

	//UI DRAWING VAOs and VBOs
	UI_MANAGER::InitVAOsVBOsEBOs(vertices, indicies);

	TextArea::textShader = new Shader("TextBasic.vert", "TextBasic.frag");;
	TextArea::BindVAOsVBOsEBOs(vertices, indicies, EXTRA_BUFFER_ALLOCATION);
	TextArea::ReadAndBindTexture();

#pragma endregion

	//Note::notes = j["notes"];

	Camera camera;
	camera.trans.position = glm::vec3(0.0f, 0.0f, 20.0f);
	camera.trans.scale = glm::vec3(1.0f);

	camera.InitCamera(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);

	double deltaTime = 0.0f;	// Time between current frame and last frame
	double lastFrame = 0.0f;	// Time of last frame

	unsigned int frame = 0;

	Shader blurShader("Blur.vert", "Blur.frag");

	float timeToNextCharPress = 0.0f;
	float timeBetweenCharPress = 1.0f;

	Shader lineShader("LineRenderer.vert", "LineRenderer.frag", "LineRenderer.gs");
	lineShader.Activate();
	int lineWidthLoc = glGetUniformLocation(lineShader.ID, "lineWidth");
	glUniform1f(lineWidthLoc, 0.1f);

	std::vector<float> linePoints;

	VAO linesVAO;

	linesVAO.Init();
	linesVAO.Bind();

	VBO linesPointsVBO(linePoints.data(), sizeof(float) * linePoints.size(), GL_DYNAMIC_DRAW);
	linesPointsVBO.Bind();

	linesVAO.LinkAttrib(linesPointsVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

	linesVAO.Unbind();
	linesPointsVBO.Unbind();

	float timeToBackspaceReset = 0.0f;
	float backspaceResetTime = 0.1f;

	while (!glfwWindowShouldClose(window))
	{
		if (timeToBackspaceReset > 0.0f) {
			timeToBackspaceReset -= deltaTime;
		}
		
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Debug_Log(deltaTime);

		if (Input::doubleClicked && !Input::leftMouseButtonHeld) {
			Input::doubleClicked = false;
		}

		Input::SetMouseInput(window);

		if (Input::timeRemainingForDoubleClick > 0) {
			Input::timeRemainingForDoubleClick -= deltaTime;
		}


		if (Input::KeyHeld(window, KeyCode::LEFT_CTRL) && glfwGetKey(window, GLFW_KEY_Z) && !z) {
			//Debug_Log("DID UNDO.");
			undoredo.Undo();
			z = true;
		}
		else if (Input::KeyHeld(window, KeyCode::LEFT_CTRL) && glfwGetKey(window, GLFW_KEY_Y) && !y) {
			//Debug_Log("DID ReDO.");
			undoredo.Redo();
			y = true;
		}
		else if (Input::KeyHeld(window, KeyCode::LEFT_CTRL) && glfwGetKey(window, GLFW_KEY_S) && !s) {
			Save();
			s = true;
		}

		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE) {
			z = false;
		}

		if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE) {
			y = false;
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
			s = false;
		}

		if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE || timeToBackspaceReset <= 0.0f) {
			backspace = false;
		}

		Camera::CameraMovement(camera, window, deltaTime, mouseDeltaX, mouseDeltaY, mouseSensitivity);
		mouseDeltaX = 0.0f;
		mouseDeltaY = 0.0f;

		DrawNotes(camera, NUM_NOTES, NUM_UI_PANELS, lastSelectedEntity, lastSelectedEntityDelete);
		DrawUI(NUM_NOTES, NUM_UI_PANELS, lastSelectedUI);

		int entityFromTexture = -1;

		//Select entity if not selected.
		if (Input::doubleClicked && lastSelectedEntity == -1) {


			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glReadPixels(Input::mouseX, MAIN_WINDOW_HEIGHT - Input::mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &entityFromTexture);

			if (entityFromTexture >= NUM_NOTES + NUM_UI_PANELS) {
				entityFromTexture = -1;
				lastSelectedEntity = -1;
				lastSelectedUI = -1;
			}
		}
		//Drop moved note if entity already selected and being dragged.
		else if (Input::doubleClicked && lastSelectedEntity != -1) {

			int i = lastSelectedEntity - NUM_UI_PANELS;
			MoveNotes(Note::notes, Note::flattenedTransforms,
				Note::notes[i].transform.position - glm::vec3(0.0f, 0.0f, 0.4f),
				Note::notesflattenedTransformsVBO, Note::notesVisibileVBO, i, undoredo, false);

			lastSelectedEntity = -1;
			recordedMovement = false;
		}

		//Select Entities for New Lines or deleting lines or deleting entities.
		if (Input::leftMouseButtonPressed) {
			
			int outValue = 0;
			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glReadPixels(Input::mouseX, MAIN_WINDOW_HEIGHT - Input::mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &outValue);

			if (outValue > NUM_UI_PANELS - 1 && outValue < NUM_UI_PANELS + NUM_NOTES) {
				if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {

					newLinesSelectedEntities.push_back(outValue - NUM_UI_PANELS);
				}
				else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {

					deleteLinesSelectedEntities.push_back(outValue - NUM_UI_PANELS);
				}
				else {
					lastSelectedEntityDelete = outValue;
				}
			}			
			else  if (outValue >= NUM_NOTES + NUM_UI_PANELS) {
				lastSelectedEntityDelete = -1;
			}
		}
		//Draw new lines if selected two line entities.
		if (newLinesSelectedEntities.size() == 2) {
			linesSelectedEntities.push_back(newLinesSelectedEntities[0]);
			linesSelectedEntities.push_back(newLinesSelectedEntities[1]);

			Links* cl = new Links{
				linesSelectedEntities,
				newLinesSelectedEntities[0],
				newLinesSelectedEntities[1]
			};

			ActionFunc af;
			af.actionType = Action::UndoCreationOrDeletionOfLinks;
			af.redoFunction = RedoLinesBetweenEntities;
			af.undoFunction = UndoLinesBetweenEntities;

			ActionArgs aa = {
				cl
			};

			UndoRedo::AddAction(af, aa);

			newLinesSelectedEntities.clear();
		}

		//Delete lines if two delete line entities selected.
		if (deleteLinesSelectedEntities.size() == 2) {

			for (int i = 0; i + 1 < linesSelectedEntities.size(); i++)
			{
				if (linesSelectedEntities[i] == deleteLinesSelectedEntities[0] && linesSelectedEntities[i + 1] == deleteLinesSelectedEntities[1]) {
					linesSelectedEntities.erase(linesSelectedEntities.begin() + i + 1);
					linesSelectedEntities.erase(linesSelectedEntities.begin() + i);
				}
				else if (linesSelectedEntities[i] == deleteLinesSelectedEntities[1] && linesSelectedEntities[i + 1] == deleteLinesSelectedEntities[0]) {
					linesSelectedEntities.erase(linesSelectedEntities.begin() + i + 1);
					linesSelectedEntities.erase(linesSelectedEntities.begin() + i);
				}
			}

			Links* cl = new Links{
				linesSelectedEntities,
				deleteLinesSelectedEntities[0],
				deleteLinesSelectedEntities[1]
			};

			ActionFunc af;
			af.actionType = Action::UndoCreationOrDeletionOfLinks;
			af.redoFunction = UndoLinesBetweenEntities;
			af.undoFunction = RedoLinesBetweenEntities;

			ActionArgs aa = {
				cl
			};

			UndoRedo::AddAction(af, aa);

			deleteLinesSelectedEntities.clear();

		}

		//Create list of line origins if entities selected for drawing lines.
		if (linesSelectedEntities.size() % 2 == 0) {

			linePoints.clear();
			for (int i = 0; i < linesSelectedEntities.size(); i++)
			{
				if (Note::visible[linesSelectedEntities[i]])
				{
					linePoints.push_back(Note::notes[linesSelectedEntities[i]].transform.position.x);
					linePoints.push_back(Note::notes[linesSelectedEntities[i]].transform.position.y);
					linePoints.push_back(Note::notes[linesSelectedEntities[i]].transform.position.z);
				}
			}

			lineShader.Activate();
			int cameraTransLocGmtry = glGetUniformLocation(lineShader.ID, "cameraTrans");
			glUniformMatrix4fv(cameraTransLocGmtry, 1, GL_FALSE, glm::value_ptr(*camera.trans.CalculateTransformMatr()));
			int persprojLocGmtry = glGetUniformLocation(lineShader.ID, "perspectiveProj");
			glUniformMatrix4fv(persprojLocGmtry, 1, GL_FALSE, glm::value_ptr(camera.projection));
			int cameraPosLoc = glGetUniformLocation(lineShader.ID, "camPos");
			glUniform3f(cameraPosLoc, camera.trans.position.x, camera.trans.position.y, camera.trans.position.z);

			linesVAO.Bind();

			linesPointsVBO.Init(linePoints.data(), sizeof(float)* linePoints.size(), GL_DYNAMIC_DRAW);
			linesPointsVBO.Bind();

			linesVAO.LinkAttrib(linesPointsVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

			linesVAO.Unbind();
			linesPointsVBO.Unbind();

			linesVAO.Bind();
			glDrawArrays(GL_LINES, 0, linePoints.size());

		}

		//Delete the last selected entitiy or write to entity.
		if (lastSelectedEntityDelete != -1) {

			int i = lastSelectedEntityDelete - NUM_UI_PANELS;

			if (Input::typed) {

				std::string curChar = "";
				curChar += Input::curTypedChar;
				Note::notes[i].textArea.ChangeText(curChar, i);
				//MoveNotes(notes, notesTransformsFlattened, notes[i].transform.position, notesTransformsVBO, notesVisibilityVBO, i, undoredo, false);
				Input::typed = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS && !backspace) {

				//std::cout << "BACKSPACE!" << std::endl;
				Note::notes[i].textArea.RemoveLastCharFromTextArea(i);
				backspace = true;
				timeToBackspaceReset = backspaceResetTime;

			}
		}
		else {
			Input::typed = false;
		}

		//Create and delete selected Entity.
		if (entityFromTexture != -1) {
			if (entityFromTexture < NUM_UI_PANELS) {
				lastSelectedUI = entityFromTexture;
				
				ButtonClickStruct bcs = { UI_MANAGER::ui_visible, lastSelectedUI };
				MouseData mp = { Input::mouseX, Input::mouseY
								, Input::leftMouseButtonPressed, Input::rightMouseButtonPressed
								, Input::leftMouseButtonReleased, Input::rightMouseButtonReleased};
				
				if (lastSelectedUI > 0) {
					CreateNewNodeButtonStruct cnnbs = { Note::notes, Note::flattenedTransforms, Note::visible, Note::notesflattenedTransformsVBO, lastSelectedEntity };
					ui_manager.ManageUI(mp, CreateNewNote, &cnnbs, lastSelectedUI);
				}
				else if(lastSelectedEntityDelete > -1){
					//Debug_Log("Deleting..");
					int i = lastSelectedEntityDelete;
					DeleteNoteInfo dNI = { lastSelectedEntityDelete, Note::notes, Note::visible,  Note::notesVisibileVBO };
					ui_manager.ManageUI(mp, DeleteNote, &dNI, lastSelectedUI);

				}

				//std::cout << Input::leftMouseButtonPressed << std::endl;

				/*UI_MANAGER::uiVisibleVBO.Bind();
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * UI_MANAGER::ui_visible.size(), UI_MANAGER::ui_visible.data());
				UI_MANAGER::uiVisibleVBO.Unbind();*/

				//std::cout << "UI SELECTED : " << lastSelectedUI << std::endl;
				//ui_manager.ManageUI(mouseX, mouseY, leftClick, rightClick, leftRelease, rightRelease, lastSelectedUI);
			}
			else if (entityFromTexture < NUM_NOTES + NUM_UI_PANELS) {
				lastSelectedEntity = entityFromTexture;
			}
		}


		if (lastSelectedEntity != -1) {
			//std::cout << "X" << std::endl;

			camera.trans.TransformMatrix();
			glm::mat4 inverseCameraView = glm::inverse(glm::lookAt(camera.trans.position, camera.trans.front + camera.trans.position, camera.trans.up));
			glm::mat4 inverseCameraProj = glm::inverse(camera.projection);

			GLfloat x = ((2.0f * Input::mouseX) / MAIN_WINDOW_WIDTH) - 1.0f;
			GLfloat y = 1.0f - (2.0f * Input::mouseY) / MAIN_WINDOW_HEIGHT;

			glm::vec4 mouseCameraCoords = inverseCameraProj * glm::vec4(x, y, -1.0f, 1.0f);
			mouseCameraCoords = glm::vec4(mouseCameraCoords.x, mouseCameraCoords.y, -1.0f, 0.0f);
			
			glm::vec3 rayDirection = glm::normalize(glm::vec3(inverseCameraView * mouseCameraCoords));
			glm::vec3 rayOrigin = camera.trans.position;
			float t = 0;

			glm::vec3 planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
			glm::vec3 pointOnPlane = glm::vec3(0.0f, 0.0f, 10.0f);
			float d = 10.0f;


			float denom = glm::dot(rayDirection, planeNormal);
			if (glm::abs(denom) > 0.0001) {
				glm::vec3 diff = pointOnPlane - rayOrigin;
				t = (glm::dot(diff, planeNormal)) / denom;

			}

			glm::vec3 point = rayOrigin + rayDirection * t;
			point.z = pointOnPlane.z;

			MoveNotes(Note::notes, Note::flattenedTransforms, point, Note::notesflattenedTransformsVBO, Note::notesVisibileVBO, lastSelectedEntity - NUM_UI_PANELS,  undoredo);
		}

		DrawText(camera);

		DrawBackShadow(camera, blurShader, NUM_NOTES);

		DrawToScreen(camera, window);

		glfwSwapBuffers(window);
		glfwPollEvents();

		Input::ResetKeys();

		frame++;
	}

	Note::notesVAO.Delete();
	Note::notesShaderProgram->Delete();
	UI_MANAGER::uiVAO.Delete();
	UI_MANAGER::uiShaderProgram.Delete();
	TextArea::textVAO.Delete();
	TextArea::textShader->Delete();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	Input::mouseX = static_cast<float>(xposIn);
	Input::mouseY = static_cast<float>(yposIn);
	
	if (firstMouse)
	{
		lastX = Input::mouseX;
		lastY = Input::mouseY;
		firstMouse = false;
	}

	mouseDeltaX = Input::mouseX - lastX;
	mouseDeltaY = lastY - Input::mouseY; // reversed since y-coordinates go from bottom to top
	lastX = Input::mouseX;
	lastY = Input::mouseY;
}