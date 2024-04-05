#include <filesystem>
#include <cstdlib>
#include <vector>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include "TextArea.h"
#include "Note.h"
#include "Camera.h"

#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"

#include "UI.h"
#include "Input.h"

#include "BMFontReader.h"

#include "UndoRedo.h"


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

int NUM_NOTES = 5;
int NUM_UI_PANELS = 5;

int EXTRA_BUFFER_ALLOCATION = 100;

// Vertices coordinates
float vertices[] =
{
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f
};

float textureCoords[] =
{
	 449.0f / 512.0f, (512.0f - 387.0f - 46.0f) / 512.0f,		//bottom left		449.0f / 512.0f, (512.0f - 387.0f + 46.0f) / 512.0f
	 (449.0f + 49.0f) / 512.0f, (512.0f - 387.0f - 46.0f) / 512.0f,		//bottom right		(449.0f + 49.0f) / 512.0f, (512.0f - 387.0f + 46.0f) / 512.0f
	 (449.0f + 49.0f) / 512.0f, (512.0f - 387.0f) / 512.0f,		//top right			(449.0f + 49.0f) / 512.0f, (512.0f - 387.0f) / 512.0f
	 (449.0f) / 512.0f, (512.0f - 387.0f) / 512.0f,		//top left			(449.0f) / 512.0f, (512.0f - 387.0f) / 512.0f
};

// Indices for vertices order
unsigned int indices[] =
{
	0, 1, 3,
	3, 1, 2
};

float screenRenderQuadVertices[] = {
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

bool firstMouse = true;
float lastX = 0, lastY = 0;
float mouseDeltaX = 0;
float mouseDeltaY = 0;
float mouseSensitivity = 100.0f;

int lastSelectedUI = -1;
int lastSelectedEntity = -1;
int lastSelectedEntityDelete = -1;

bool recordedMovement = false;

std::vector<bool> resetText(NUM_NOTES);
std::vector<float> notesVisible(NUM_NOTES);
std::vector<int> deletedNotesEntities;

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

bool hide = false;

void MoveNotes(std::vector<Note>& notes, std::vector<float>& notesTransformsFlattened
	, glm::vec3 point, VBO& notesTransformsVBO, VBO& notesVisibilityVBO, int i, UndoRedo& undoredo);

struct CreateNewNodeButtonStruct {
	std::vector<Note>& notes;
	std::vector<float>& notesTransformsFlattened;
	std::vector<float>& visible;
	int& lastSelectedEntity;
};

void CreateNewNote(void* args) {
	CreateNewNodeButtonStruct* a = (CreateNewNodeButtonStruct*)args;

	Note newNote;
	a->notes.push_back(newNote);
	NUM_NOTES++;

	int i = NUM_NOTES - 1;
	

	a->notes[i].width = 1.0f;
	a->notes[i].height = 1.0f;
	a->notes[i].transform.scale = glm::vec3(a->notes[i].width, a->notes[i].height, 1.0f);

	float* head = glm::value_ptr(*a->notes[i].transform.CalculateTransformMatr());
	for (int j = 0; j < 16; j++)
	{
		a->notesTransformsFlattened.push_back(head[j]);
	}

	a->lastSelectedEntity = i + NUM_UI_PANELS;

	std::vector<float> values;

	a->notes[i].textArea.transform = a->notes[i].transform;
	a->notes[i].textArea.width = a->notes[i].width;
	a->notes[i].textArea.height = a->notes[i].height;
	a->notes[i].textArea.sampleString = "To dispriz'd coil, and be: to othe mind by a life, and love, and mome of somenterprises calamity opposing end swear, to dream:";
	a->notes[i].textArea.individualLengths.push_back(a->notes[i].textArea.sampleString.length());
	a->notes[i].textArea.IsUI = 0.0f;
	//Debug_Log(a->notes[i].textArea.texCoords.size());
	a->notes[i].textArea.FillGlobalTextArrays(values);
	
	resetText.push_back(true);
	a->visible.push_back(1.0f);


	a->notes[i].textArea.BindVAOsVBOsEBOs(vertices, indices, EXTRA_BUFFER_ALLOCATION);

	//Debug(a->notes[i].textArea.transform.position);
	//Debug_Log(a->notes[i].textArea.texCoords.size());
	

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

}

int main()
{
	UndoRedo undoredo;

	BMFontReader reader("./fonts/TestFontDF.fnt");
	reader.read();

	TextArea::reader = &reader;
	

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
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);


	TextArea::textShader = new Shader("UITextBasic.vert", "UITextBasic.frag");;

#pragma region VBOs and Positions
	Shader uiShaderProgram("UIBasic.vert", "UIBasic.frag");

	std::vector<float> ui_transformsFlattened(NUM_UI_PANELS * 16);
	srand((unsigned int)time(0));

	std::vector<float> ui_visible(NUM_UI_PANELS);
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
		ui_visible[i] = 1.0f;

		float* head = glm::value_ptr(*buttons[i].transform.CalculateTransformMatr());

		std::vector<float> values;

		memcpy(&ui_transformsFlattened[i * (int)16], head, 64);
		buttons[i].init(i);
		buttons[i].textArea.transform = buttons[i].transform;
		buttons[i].textArea.transform.position.x -= 0.1f;
		buttons[i].textArea.width = 0.1f;
		buttons[i].textArea.height = 0.1f;
		buttons[i].textArea.sampleString = "BUTTON";
		buttons[i].textArea.individualLengths.push_back(buttons[i].textArea.sampleString.length());
		buttons[i].textArea.IsUI = 1.0f;
		buttons[i].textArea.FillGlobalTextArrays(values);
	}

	Shader notesShaderProgram("default.vert", "default.frag");

	std::vector<float> visible(NUM_NOTES);
	std::vector<float> notesTransformsFlattened(NUM_NOTES * 16);
	srand((unsigned int)time(0));
	std::vector<Note> notes(NUM_NOTES);
	for (int i = 0; i < NUM_NOTES; i++)
	{
		visible[i] = 1.0f;
		notes[i].width = 1.0f;
		notes[i].height = 1.0f;
		float x = (float)rand() / (RAND_MAX);
		float y = (float)rand() / (RAND_MAX);
		float z = 10.0f;
		notes[i].transform.position = glm::vec3(x, y, z);
		notes[i].transform.position.x = (notes[i].transform.position.x - 0.5f) * 2.0f;
		notes[i].transform.position.y = (notes[i].transform.position.y - 0.5f) * 2.0f;
		notes[i].transform.position.x *= 5.0f;
		notes[i].transform.position.y *= 5.0f;

		notes[i].transform.scale = glm::vec3(notes[i].width, notes[i].height, 1.0f);

		float* head = glm::value_ptr(*notes[i].transform.CalculateTransformMatr());

		std::vector<float> values;

		memcpy(&notesTransformsFlattened[i * (int)16], head, 64);

		notes[i].textArea.ID = i;
		notes[i].textArea.transform = notes[i].transform;
		//notes[i].textArea.transform.position += glm::vec3(-notes[i].width, notes[i].height, 0.0f);
		notes[i].textArea.width = notes[i].width;
		notes[i].textArea.height = notes[i].height;
		notes[i].textArea.sampleString = "To dispriz'd coil, and be: to othe mind by a life, and love, and mome of somenterprises calamity opposing end swear, to dream:";
		notes[i].textArea.individualLengths.push_back(notes[i].textArea.sampleString.length());
		notes[i].textArea.IsUI = 0.0f;
		notes[i].textArea.FillGlobalTextArrays(values);
		resetText[i] = true;
		notesVisible[i] = 1.0f;
	}

	//NOTES drawing VAOs and VBOs
	VAO notesVAO, uiVAO;
	notesVAO.Init();
	notesVAO.Bind();

	VBO notesVBO(vertices, sizeof(vertices), GL_STATIC_DRAW);
	//VBO notesTransformsVBO(offsets.data(), offsets.size());
	VBO notesTransformsVBO(notesTransformsFlattened.data(), sizeof(float) * notesTransformsFlattened.size() + EXTRA_BUFFER_ALLOCATION * 16 * sizeof(float), GL_DYNAMIC_DRAW);
	VBO notesVisibilityVBO(notesVisible.data(), sizeof(float) * notesVisible.size() + EXTRA_BUFFER_ALLOCATION * sizeof(float), GL_DYNAMIC_DRAW);
	//std::cout << notesTransformsFlattened.size() / 16 << std::endl;
	notesTransformsVBO.Bind();
	
	EBO notesEBO(indices, sizeof(indices));
	notesVAO.LinkTransformAttrib(notesTransformsVBO, 1);
	notesVAO.LinkAttrib(notesVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	notesVAO.LinkAttrib(notesVisibilityVBO, 5, 1, GL_FLOAT, sizeof(float), (void*)0);
	glVertexAttribDivisor(5, 1);

	notesVAO.Unbind();
	notesVBO.Unbind();
	notesTransformsVBO.Unbind();
	notesVisibilityVBO.Unbind();
	notesEBO.Unbind();

	for (int i = 0; i < NUM_NOTES; i++)
	{
		for (int j = 0; j < notes[i].textArea.glyphTrans.size(); j++)
		{
			notes[i].textArea.textIsVisible[i + j] = 1.0f;
		}
		MoveNotes(notes, notesTransformsFlattened, notes[i].transform.position + glm::vec3(1.0f), notesTransformsVBO, notesVisibilityVBO,  i, undoredo);
		//notesVisible[i] = 0.0f;
	}
	notesVisibilityVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * notesVisible.size(), notesVisible.data());
	notesVisibilityVBO.Unbind();
	

	undoredo.Flush();
	recordedMovement = false;

	//UI DRAWING VAOs and VBOs
	uiVAO.Init();
	uiVAO.Bind();

	VBO uiVBO(vertices, sizeof(vertices), GL_STATIC_DRAW);
	VBO uiTransformsVBO(ui_transformsFlattened.data(), sizeof(float) * ui_transformsFlattened.size(), GL_DYNAMIC_DRAW);
	VBO uiVisibleVBO(ui_visible.data(), sizeof(float) * ui_visible.size(), GL_DYNAMIC_DRAW);
	uiTransformsVBO.Bind();
	uiVisibleVBO.Bind();

	EBO uiEBO(indices, sizeof(indices));
	uiVAO.LinkAttrib(uiVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	uiVAO.LinkTransformAttrib(uiTransformsVBO, 1);
	uiVAO.LinkAttrib(uiVisibleVBO, 5, 1, GL_FLOAT,  sizeof(float), (void*)0);
	uiVisibleVBO.Bind();
	glVertexAttribDivisor(5, 1);
	uiVisibleVBO.Unbind();

	uiVAO.Unbind();
	uiVBO.Unbind();
	uiTransformsVBO.Unbind();
	uiEBO.Unbind();
#pragma endregion

	TextArea::BindVAOsVBOsEBOs(vertices, indices, EXTRA_BUFFER_ALLOCATION);

#pragma endregion

#pragma region Texture Reading

	unsigned int fontAtlas;
	// texture 1
	// ---------
	glGenTextures(1, &fontAtlas);
	glBindTexture(GL_TEXTURE_2D, fontAtlas);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char* data = stbi_load("fonts/TestFontDF.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	TextArea::textShader->Activate(); // don't forget to activate/use the shader before setting uniforms!
	// either set it manually like so:
	glUniform1i(glGetUniformLocation(TextArea::textShader->ID, "textureAtlas"), 0);

#pragma endregion


	Camera camera;
	camera.trans.position = glm::vec3(0.0f, 0.0f, 30.0f);
	camera.trans.scale = glm::vec3(1.0f);

	double deltaTime = 0.0f;	// Time between current frame and last frame
	double lastFrame = 0.0f; // Time of last frame

	camera.InitCamera(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);

	unsigned int rectVAO, rectVBO;
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenRenderQuadVertices), &screenRenderQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	unsigned int frame = 0;
	int dc = 0;
	int lhd = 0;

	camera.projection = glm::perspective(glm::radians(45.0f), float(MAIN_WINDOW_WIDTH) / MAIN_WINDOW_HEIGHT, 0.1f, 1000.0f);

	while (!glfwWindowShouldClose(window))
	{
		
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

		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE) {
			z = false;
		}

		if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE) {
			y = false;
		}

		if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_RELEASE) {
			hide = false;
		}

		/*if(Input::leftMouseButtonHeld)
		{
			Debug_Log("LEFT MOUSE HELD DOWN." << lhd);
			lhd++;
		}

		if (Input::doubleClicked) {
			Debug_Log("DOUBLE CLICKED" << dc);
			dc++;
		}*/

		glBindFramebuffer(GL_FRAMEBUFFER, camera.FBO);
		glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
		glEnable(GL_BLEND);

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		notesShaderProgram.Activate();
		int persprojLoc = glGetUniformLocation(notesShaderProgram.ID, "perspectiveProj");
		glUniformMatrix4fv(persprojLoc, 1, GL_FALSE, glm::value_ptr(camera.projection));
		

		const float cameraSpeed = 2.5f * (float)deltaTime; // adjust accordingly
		if (Input::KeyHeld(window, KeyCode::W))
			camera.trans.position += cameraSpeed * camera.trans.front;
		if (Input::KeyHeld(window, KeyCode::S))
			camera.trans.position -= cameraSpeed * camera.trans.front;
		if (Input::KeyHeld(window, KeyCode::A))
			camera.trans.position -= cameraSpeed * camera.trans.right;
		if (Input::KeyHeld(window, KeyCode::D))
			camera.trans.position += cameraSpeed * camera.trans.right;

		if (Input::rightMouseButtonHeld) {
			camera.trans.rotation.y -= (mouseDeltaX * mouseSensitivity * (float)deltaTime);
			camera.trans.rotation.x -= (mouseDeltaY * -mouseSensitivity * (float)deltaTime);


			if (camera.trans.rotation.x > 89.0f)
				camera.trans.rotation.x = 89.0f;
			if (camera.trans.rotation.x < -89.0f)
				camera.trans.rotation.x = -89.0f;
		}

		mouseDeltaX = 0.0f;
		mouseDeltaY = 0.0f;

		int cameraTransLoc = glGetUniformLocation(notesShaderProgram.ID, "cameraTrans");
		glUniformMatrix4fv(cameraTransLoc, 1, GL_FALSE, glm::value_ptr(*camera.trans.CalculateTransformMatr()));
		int entitiesCountLoc = glGetUniformLocation(notesShaderProgram.ID, "entitiesCount");
		glUniform1f(entitiesCountLoc, NUM_NOTES + NUM_UI_PANELS);
		int uiEntitiesCountLocA = glGetUniformLocation(notesShaderProgram.ID, "uiEntitiesCount");
		glUniform1f(uiEntitiesCountLocA, NUM_UI_PANELS);
		int lastSelectedEntityLoc = glGetUniformLocation(notesShaderProgram.ID, "lastSelectedEntity");
		glUniform1i(lastSelectedEntityLoc, lastSelectedEntity);

		notesVAO.Bind();
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, NUM_NOTES);

		uiShaderProgram.Activate();
		int uiEntitiesCountLoc = glGetUniformLocation(uiShaderProgram.ID, "entitiesCount");
		glUniform1f(uiEntitiesCountLoc, NUM_UI_PANELS + NUM_NOTES);
		int UIlastSelectedEntityLoc = glGetUniformLocation(uiShaderProgram.ID, "lastSelectedUI");
		glUniform1i(UIlastSelectedEntityLoc, lastSelectedUI);

		uiVAO.Bind();/*
		glDisable(GL_DEPTH_TEST);*/

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, NUM_UI_PANELS);

		int entityFromTexture = -1;


		if (Input::doubleClicked && lastSelectedEntity == -1) {
			//Debug_Log("Double Clicked.");
			//glBindFramebuffer(GL_FRAMEBUFFER, camera.FBO);
			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glReadPixels(Input::mouseX, MAIN_WINDOW_HEIGHT - Input::mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &entityFromTexture);
			//glGetError();
			if (entityFromTexture >= NUM_NOTES + NUM_UI_PANELS) {
				entityFromTexture = -1;
				lastSelectedEntity = -1;
				lastSelectedUI = -1;
			}
			//std::cout << "Mouse over entity : " << entityFromTexture << std::endl;
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else if (Input::doubleClicked && lastSelectedEntity != -1) {

			//float* visibility = new float(1.0f);
			//int i = lastSelectedEntity - NUM_UI_PANELS;
			//notesVisible[i] = 1.0f;
			//notesVisibilityVBO.Bind();
			//glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(float), sizeof(float), visibility);
			//notesVisibilityVBO.Unbind();

			lastSelectedEntity = -1;
			recordedMovement = false;
			//resetText = true;
		}

		if (Input::leftMouseButtonPressed || (glfwGetKey(window, GLFW_KEY_DELETE) && !hide)) {
			int outValue = 0;
			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glReadPixels(Input::mouseX, MAIN_WINDOW_HEIGHT - Input::mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &outValue);
			//glGetError();
			if (outValue > NUM_UI_PANELS - 1 && outValue < NUM_UI_PANELS + NUM_NOTES) {
				lastSelectedEntityDelete = outValue;
				Debug_Log("Deleting: " << lastSelectedEntityDelete);
			}
			hide = true;
		}

		/*if (lastSelectedEntityDelete != -1) {
			
			Debug_Log("Attempting to delete: " << lastSelectedEntityDelete);
			MouseData mp = { Input::mouseX, Input::mouseY
								, Input::leftMouseButtonPressed, Input::rightMouseButtonPressed
								, Input::leftMouseButtonReleased, Input::rightMouseButtonReleased };

			int i = lastSelectedEntityDelete;
			DeleteNoteInfo dNI = { lastSelectedEntityDelete, notesVisible,  notesVisibilityVBO };
			void* a = (void*)&dNI;
			DeleteNote(a);
		}*/

		if (entityFromTexture != -1) {
			if (entityFromTexture < NUM_UI_PANELS) {
				lastSelectedUI = entityFromTexture;
				
				//std::cout << "FRAME: " << frame << std::endl;

				ButtonClickStruct bcs = { ui_visible, lastSelectedUI };
				MouseData mp = { Input::mouseX, Input::mouseY
								, Input::leftMouseButtonPressed, Input::rightMouseButtonPressed
								, Input::leftMouseButtonReleased, Input::rightMouseButtonReleased};
				
				//ui_manager.ManageUI(mp, ButtonClickTest, &bcs, lastSelectedUI);
				if (lastSelectedUI > 0) {
					CreateNewNodeButtonStruct cnnbs = { notes, notesTransformsFlattened, notesVisible, lastSelectedEntity };
					ui_manager.ManageUI(mp, CreateNewNote, &cnnbs, lastSelectedUI);
				}
				else if(lastSelectedEntityDelete > -1){
					Debug_Log("Deleting..");
					int i = lastSelectedEntityDelete;
					DeleteNoteInfo dNI = { lastSelectedEntityDelete, notes, notesVisible,  notesVisibilityVBO };
					ui_manager.ManageUI(mp, DeleteNote, &dNI, lastSelectedUI);

				}

				//std::cout << Input::leftMouseButtonPressed << std::endl;

				uiVisibleVBO.Bind();
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * ui_visible.size(), ui_visible.data());
				uiVisibleVBO.Unbind();

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

			MoveNotes(notes, notesTransformsFlattened, point, notesTransformsVBO, notesVisibilityVBO, lastSelectedEntity - NUM_UI_PANELS,  undoredo);
		}

		TextArea::textShader->Activate();
		int persprojLocUIT = glGetUniformLocation(notesShaderProgram.ID, "perspectiveProj");
		glUniformMatrix4fv(persprojLocUIT, 1, GL_FALSE, glm::value_ptr(camera.projection));
		int cameraTransLocUIT = glGetUniformLocation(notesShaderProgram.ID, "cameraTrans");
		glUniformMatrix4fv(cameraTransLocUIT, 1, GL_FALSE, glm::value_ptr(*camera.trans.CalculateTransformMatr()));

		glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
		glBindTexture(GL_TEXTURE_2D, fontAtlas);
		//textVAO.Bind();
		TextArea::textVAO.Bind();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, TextArea::totalGlyphs);
		//glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		camera.fbShaderProgram.Activate();
		glBindVertexArray(rectVAO);
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
			glBindTexture(GL_TEXTURE_2D, camera.entityColourTextureID);
		else
			glBindTexture(GL_TEXTURE_2D, camera.framebuffersTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();

		Input::ResetKeys();

		frame++;

	}

	notesVAO.Delete();
	notesShaderProgram.Delete();
	uiVAO.Delete();
	uiShaderProgram.Delete();
	//textVAO.Delete();
	TextArea::textVAO.Delete();
	TextArea::textShader->Delete();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void MoveNotes(std::vector<Note>& notes, std::vector<float>& notesTransformsFlattened
				, glm::vec3 point, VBO& notesTransformsVBO,VBO& notesVisibilityVBO, 
				int i, UndoRedo& undoredo) {
	glm::vec3 oldNotesPos = notes[i].transform.position;
	notes[i].transform.position = glm::vec3(point);
	float* curNewTrans = glm::value_ptr(*notes[i].transform.CalculateTransformMatr());
	memcpy(&notesTransformsFlattened[i * (int)16], curNewTrans, 64);

	/*float newTrans[16];
	memcpy(&newTrans[0], curNewTrans, 64);*/

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
		offset -= glm::vec3(notes[i].textArea.width, -notes[i].textArea.height, 0.0f);
		resetText[i] = false;
	}

	notes[i].textArea.FillGlobalTextArrays(values, offset, start, end);

	const float arraySize = notes[i].textArea.glyphTrans.size() * 16;

	TextArea::textTransformsVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, start * sizeof(float), sizeof(float) * arraySize, &notes[i].textArea.textTransformsFlattened[start]);
	TextArea::textTransformsVBO.Unbind();

	if (!recordedMovement) {
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

		ActionFunc notesMovindAF;
		notesMovindAF.actionType = Action::ChangedNotePosition;
		void* a = (void*)mpm;
		notesMovindAF.undoFunction = UndoMousePickingMoving;
		notesMovindAF.redoFunction = RedoMousePickingMoving;

		ActionArgs aa{
			a
		};

		undoredo.AddAction(notesMovindAF, aa);
		recordedMovement = true;
	}
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