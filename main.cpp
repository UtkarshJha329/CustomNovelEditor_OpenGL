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

std::vector<int> linesSelectedEntities;

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
bool backspace = false;

void MoveNotes(std::vector<Note>& notes, std::vector<float>& notesTransformsFlattened
	, glm::vec3 point, VBO& notesTransformsVBO, VBO& notesVisibilityVBO, int i, UndoRedo& undoredo, bool allowRecording = true);

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

		//a->notes[i].textArea.BindVAOsVBOsEBOs(vertices, indices, EXTRA_BUFFER_ALLOCATION);

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


		a->notes[i].textArea.BindVAOsVBOsEBOs(vertices, indices, EXTRA_BUFFER_ALLOCATION);

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

int Factorial(int n) {
	if (n == 0) {
		return 1;
	}
	else {
		return n * Factorial(n - 1);
	}
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
	glfwSetCharModsCallback(window, Input::charmods_callback);
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

		notes[i].textArea.ID = i + NUM_UI_PANELS;
		notes[i].textArea.transform = notes[i].transform;
		notes[i].textArea.transform.position.z -= 0.2f;
		//notes[i].textArea.transform.position += glm::vec3(-notes[i].width, notes[i].height, 0.0f);
		notes[i].textArea.width = notes[i].width;
		notes[i].textArea.height = notes[i].height;
		notes[i].textArea.sampleString = "To dispriz'd coil, and be: to othe mind by a life, and love, and mome of somenterprises calamity opposing end swear, to dream:";
		//notes[i].textArea.sampleString = "To dispriz'd coil, and be: to othe mind by a life, and love, and mome of somenterprises";
		notes[i].textArea.individualLengths.push_back(notes[i].textArea.sampleString.length());
		notes[i].textArea.IsUI = 0.0f;
		notes[i].textArea.FillGlobalTextArrays(values);

		for (int i = 0; i < 3; i++)
		{
			int randNoteId = rand() % NUM_NOTES;
			notes[i].connectedToNotes.push_back(randNoteId);
		}


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
		MoveNotes(notes, notesTransformsFlattened, notes[i].transform.position, notesTransformsVBO, notesVisibilityVBO,  i, undoredo);
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
	camera.trans.position = glm::vec3(0.0f, 0.0f, 20.0f);
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

		if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE) {
			backspace = false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, camera.FBO);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		notesShaderProgram.Activate();
		int persprojLoc = glGetUniformLocation(notesShaderProgram.ID, "perspectiveProj");
		glUniformMatrix4fv(persprojLoc, 1, GL_FALSE, glm::value_ptr(camera.projection));
		

		
		if (Input::rightMouseButtonHeld) {

			const float cameraSpeed = 2.5f * (float)deltaTime;
			if (Input::KeyHeld(window, KeyCode::W))
				camera.trans.position += cameraSpeed * camera.trans.front;
			if (Input::KeyHeld(window, KeyCode::S))
				camera.trans.position -= cameraSpeed * camera.trans.front;
			if (Input::KeyHeld(window, KeyCode::A))
				camera.trans.position -= cameraSpeed * camera.trans.right;
			if (Input::KeyHeld(window, KeyCode::D))
				camera.trans.position += cameraSpeed * camera.trans.right;

			camera.trans.rotation.y -= (mouseDeltaX * mouseSensitivity * (float)deltaTime);
			camera.trans.rotation.x -= (mouseDeltaY * -mouseSensitivity * (float)deltaTime);


			if (camera.trans.rotation.x > 89.0f)
				camera.trans.rotation.x = 89.0f;
			if (camera.trans.rotation.x < -89.0f)
				camera.trans.rotation.x = -89.0f;
		}
		else {
			const float cameraSpeed = 2.5f * (float)deltaTime;
			if (Input::KeyHeld(window, KeyCode::UP_ARROW))
				camera.trans.position += cameraSpeed * camera.trans.up;
			if (Input::KeyHeld(window, KeyCode::DOWN_ARROW))
				camera.trans.position -= cameraSpeed * camera.trans.up;
			if (Input::KeyHeld(window, KeyCode::LEFT_ARRORW))
				camera.trans.position -= cameraSpeed * camera.trans.right;
			if (Input::KeyHeld(window, KeyCode::RIGHT_ARROW))
				camera.trans.position += cameraSpeed * camera.trans.right;

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
		int lastClickedEntityLoc = glGetUniformLocation(notesShaderProgram.ID, "lastClickedEntity");
		glUniform1i(lastClickedEntityLoc, lastSelectedEntityDelete);
		

		notesVAO.Bind();
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, NUM_NOTES);

		uiShaderProgram.Activate();
		int uiEntitiesCountLoc = glGetUniformLocation(uiShaderProgram.ID, "entitiesCount");
		glUniform1f(uiEntitiesCountLoc, NUM_UI_PANELS + NUM_NOTES);
		int UIlastSelectedEntityLoc = glGetUniformLocation(uiShaderProgram.ID, "lastSelectedUI");
		glUniform1i(UIlastSelectedEntityLoc, lastSelectedUI);

		uiVAO.Bind();

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, NUM_UI_PANELS);

		int entityFromTexture = -1;


		if (Input::doubleClicked && lastSelectedEntity == -1) {


			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glReadPixels(Input::mouseX, MAIN_WINDOW_HEIGHT - Input::mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &entityFromTexture);

			if (entityFromTexture >= NUM_NOTES + NUM_UI_PANELS) {
				entityFromTexture = -1;
				lastSelectedEntity = -1;
				lastSelectedUI = -1;
			}
		}
		else if (Input::doubleClicked && lastSelectedEntity != -1) {

			int i = lastSelectedEntity - NUM_UI_PANELS;
			MoveNotes(notes, notesTransformsFlattened,
				notes[i].transform.position - glm::vec3(0.0f, 0.0f, 0.4f),
				notesTransformsVBO, notesVisibilityVBO, i, undoredo, false);

			lastSelectedEntity = -1;
			recordedMovement = false;
		}

		if (Input::leftMouseButtonPressed) {
			
			int outValue = 0;
			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glReadPixels(Input::mouseX, MAIN_WINDOW_HEIGHT - Input::mouseY, 1, 1, GL_RED_INTEGER, GL_INT, &outValue);

			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {

				if (outValue > NUM_UI_PANELS - 1 && outValue < NUM_UI_PANELS + NUM_NOTES) {
					linesSelectedEntities.push_back(outValue - NUM_UI_PANELS);
					//std::cout << linesSelectedEntities.size() << std::endl;
				}
			}
			else {
				/*linesSelectedEntities.clear();
				linePoints.clear();*/

				if (outValue > NUM_UI_PANELS - 1 && outValue < NUM_UI_PANELS + NUM_NOTES) {
					lastSelectedEntityDelete = outValue;
				}
				else if(outValue >= NUM_NOTES + NUM_UI_PANELS) {
					lastSelectedEntityDelete = -1;
				}


			}

		}

		if (linesSelectedEntities.size() % 2 == 0) {

			linePoints.clear();
			for (int i = 0; i < linesSelectedEntities.size(); i++)
			{
				if (notesVisible[linesSelectedEntities[i]])
				{
					linePoints.push_back(notes[linesSelectedEntities[i]].transform.position.x);
					linePoints.push_back(notes[linesSelectedEntities[i]].transform.position.y);
					linePoints.push_back(notes[linesSelectedEntities[i]].transform.position.z);
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

		if (lastSelectedEntityDelete != -1) {

			int i = lastSelectedEntityDelete - NUM_UI_PANELS;

			if (Input::typed) {

				std::string curChar = "";
				curChar += Input::curTypedChar;
				notes[i].textArea.ChangeText(curChar, i);
				//MoveNotes(notes, notesTransformsFlattened, notes[i].transform.position, notesTransformsVBO, notesVisibilityVBO, i, undoredo, false);
				Input::typed = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS && !backspace) {

				//std::cout << "BACKSPACE!" << std::endl;
				notes[i].textArea.RemoveLastCharFromTextArea(i);
				backspace = true;

			}
		}
		else {
			Input::typed = false;
		}

		if (entityFromTexture != -1) {
			if (entityFromTexture < NUM_UI_PANELS) {
				lastSelectedUI = entityFromTexture;
				
				ButtonClickStruct bcs = { ui_visible, lastSelectedUI };
				MouseData mp = { Input::mouseX, Input::mouseY
								, Input::leftMouseButtonPressed, Input::rightMouseButtonPressed
								, Input::leftMouseButtonReleased, Input::rightMouseButtonReleased};
				
				if (lastSelectedUI > 0) {
					CreateNewNodeButtonStruct cnnbs = { notes, notesTransformsFlattened, notesVisible, notesVisibilityVBO, lastSelectedEntity };
					ui_manager.ManageUI(mp, CreateNewNote, &cnnbs, lastSelectedUI);
				}
				else if(lastSelectedEntityDelete > -1){
					//Debug_Log("Deleting..");
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

		glBindFramebuffer(GL_FRAMEBUFFER, camera.blurFBO);
		//glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		//glStencilMask(0x00); // disable writing to the stencil buffer
		//glDisable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_TEST);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		blurShader.Activate();
		persprojLoc = glGetUniformLocation(blurShader.ID, "perspectiveProj");
		glUniformMatrix4fv(persprojLoc, 1, GL_FALSE, glm::value_ptr(camera.projection));
		cameraTransLoc = glGetUniformLocation(blurShader.ID, "cameraTrans");
		glUniformMatrix4fv(cameraTransLoc, 1, GL_FALSE, glm::value_ptr(*camera.trans.CalculateTransformMatr()));

		/*for (int i = 0; i < NUM_NOTES; i++)
		{
			notes[i].transform.scale *= glm::vec3(1.0f);

			float* head = glm::value_ptr(*notes[i].transform.CalculateTransformMatr());
			memcpy(&notesTransformsFlattened[i * (int)16], head, 64);
		}
		notesTransformsVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * notesTransformsFlattened.size(), notesTransformsFlattened.data());
		notesTransformsVBO.Unbind();*/


		notesVAO.Bind();
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, NUM_NOTES);
		glActiveTexture(GL_TEXTURE0);
		/*glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);*/

		//for (int i = 0; i < NUM_NOTES; i++)
		//{
		//	notes[i].transform.scale /= glm::vec3(1.0f);

		//	float* head = glm::value_ptr(*notes[i].transform.CalculateTransformMatr());
		//	memcpy(&notesTransformsFlattened[i * (int)16], head, 64);
		//}
		//notesTransformsVBO.Bind();
		//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * notesTransformsFlattened.size(), notesTransformsFlattened.data());
		//notesTransformsVBO.Unbind();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		camera.fbShaderProgram.Activate();
		glBindVertexArray(rectVAO);

		glActiveTexture(GL_TEXTURE0);
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
			glBindTexture(GL_TEXTURE_2D, camera.entityColourTextureID);
		else
			glBindTexture(GL_TEXTURE_2D, camera.framebuffersTexture);
		glUniform1i(glGetUniformLocation(camera.fbShaderProgram.ID, "screenTexture"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, camera.blurTexture);
		glUniform1i(glGetUniformLocation(camera.fbShaderProgram.ID, "blurTexture"), 1);

		glDrawArrays(GL_TRIANGLES, 0, 6);


		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);

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
				int i, UndoRedo& undoredo, bool allowRecording) {

	glm::vec3 oldNotesPos = notes[i].transform.position;
	point += glm::vec3(0.0f, 0.0f, 0.2f);
	notes[i].transform.position = point;
	float* curNewTrans = glm::value_ptr(*notes[i].transform.CalculateTransformMatr());
	memcpy(&notesTransformsFlattened[i * (int)16], curNewTrans, 64);

	//Debug_Log("EARLIER VALUE VVVV");
	//Debug(notes[i].transform.position);

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