#include <iostream>
#include <cstdlib>
#include <vector>

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

#include <filesystem>


#define MAIN_WINDOW_WIDTH 800																						// Width of Main Window
#define MAIN_WINDOW_HEIGHT 600																						// Height of Main Window

#define LOPE_FAILURE 0
#define LOPE_SUCCESS 1

#define NUM_NOTES 10
#define NUM_UI_PANELS 5

#define Debug_Log(x) std::cout << x << std::endl

Shader* TextArea::textShader;
std::vector<float> TextArea::textTransformsFlattened;
std::vector<float> TextArea::texCoords;
std::vector<float> TextArea::isUI;
int TextArea::totalGlyphs;
int TextArea::totalTextAreas;

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

int main()
{
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

		memcpy(&ui_transformsFlattened[i * (int)16], head, 64);
		buttons[i].init(i);
		buttons[i].textArea.transform = buttons[i].transform;
		buttons[i].textArea.transform.position.x -= 0.1f;
		buttons[i].textArea.width = 0.1f;
		buttons[i].textArea.height = 0.1f;
		buttons[i].textArea.sampleString = "BUTTON";
		buttons[i].textArea.IsUI = 1.0f;
		buttons[i].textArea.FillGlobalTextArrays();
	}

	Shader notesShaderProgram("default.vert", "default.frag");

	std::vector<float> transformsFlattened(NUM_NOTES * 16);
	srand((unsigned int)time(0));
	std::vector<Note> notes(NUM_NOTES);
	for (int i = 0; i < NUM_NOTES; i++)
	{
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

		memcpy(&transformsFlattened[i * (int)16], head, 64);
		notes[i].textArea.transform = notes[i].transform;
		notes[i].textArea.transform.position -= glm::vec3(notes[i].width, -notes[i].height, 0.0f);
		notes[i].textArea.width = notes[i].width;
		notes[i].textArea.height = notes[i].height;
		notes[i].textArea.sampleString = "To dispriz'd coil, and be: to othe mind by a life, and love, and mome of somenterprises calamity opposing end swear, to dream:";
		notes[i].textArea.IsUI = 0.0f;
		notes[i].textArea.FillGlobalTextArrays();
	}

	//NOTES drawing VAOs and VBOs
	VAO notesVAO, uiVAO;
	notesVAO.Init();
	notesVAO.Bind();

	VBO notesVBO(vertices, sizeof(vertices), GL_STATIC_DRAW);
	//VBO notesTransformsVBO(offsets.data(), offsets.size());
	VBO notesTransformsVBO(transformsFlattened.data(), sizeof(float) * transformsFlattened.size(), GL_DYNAMIC_DRAW);
	//std::cout << transformsFlattened.size() / 16 << std::endl;
	notesTransformsVBO.Bind();
	
	EBO notesEBO(indices, sizeof(indices));
	notesVAO.LinkTransformAttrib(notesTransformsVBO, 1);
	notesVAO.LinkAttrib(notesVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

	notesVAO.Unbind();
	notesVBO.Unbind();
	notesTransformsVBO.Unbind();
	notesEBO.Unbind();

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

	TextArea::BindVAOsVBOsEBOs(vertices, indices);

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
	camera.trans.position = glm::vec3(0.0f, 0.0f, 10.0f);
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
		glm::mat4 perspectiveProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
		int persprojLoc = glGetUniformLocation(notesShaderProgram.ID, "perspectiveProj");
		glUniformMatrix4fv(persprojLoc, 1, GL_FALSE, glm::value_ptr(perspectiveProj));
		

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
			lastSelectedEntity = -1;
		}

		if (entityFromTexture != -1) {
			if (entityFromTexture < NUM_UI_PANELS) {
				lastSelectedUI = entityFromTexture;
				
				//std::cout << "FRAME: " << frame << std::endl;

				ButtonClickStruct bcs = { ui_visible, lastSelectedUI };
				MouseData mp = { Input::mouseX, Input::mouseY
								, Input::leftMouseButtonPressed, Input::rightMouseButtonPressed
								, Input::leftMouseButtonReleased, Input::rightMouseButtonReleased};
				ui_manager.ManageUI(mp, ButtonClickTest, &bcs, lastSelectedUI);

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

			glm::mat4 inverseCameraTrans = glm::inverse(*camera.trans.TransformMatrix());
			glm::mat4 inverseCameraView = glm::inverse(glm::lookAt(camera.trans.position, camera.trans.front + camera.trans.position, camera.trans.up));
			glm::mat4 inverseCameraProj = glm::inverse(perspectiveProj);

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

			int i = lastSelectedEntity - NUM_UI_PANELS;
			notes[i].transform.position = glm::vec3(point);
			float* curNewTrans = glm::value_ptr(*notes[i].transform.CalculateTransformMatr());
			memcpy(&transformsFlattened[i * (int)16], curNewTrans, 64);

			notesTransformsVBO.Bind();
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * transformsFlattened.size(), transformsFlattened.data());
			notesTransformsVBO.Unbind();

		}

		TextArea::textShader->Activate();
		int persprojLocUIT = glGetUniformLocation(notesShaderProgram.ID, "perspectiveProj");
		glUniformMatrix4fv(persprojLocUIT, 1, GL_FALSE, glm::value_ptr(perspectiveProj));
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