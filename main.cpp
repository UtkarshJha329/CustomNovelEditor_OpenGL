#include <iostream>
#include <cstdlib>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include "Transform.h"
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

int main()
{
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

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

#pragma region VBOs and Positions
	Shader uiShaderProgram("UIBasic.vert", "UIBasic.frag");

	std::vector<float> ui_transformsFlattened(NUM_UI_PANELS * 16);
	srand((unsigned int)time(0));
	std::vector<Transform> ui_transforms(NUM_UI_PANELS);
	std::vector<float> ui_visible(NUM_UI_PANELS);
	std::vector<Button> buttons(NUM_UI_PANELS);
	for (int i = 0; i < NUM_UI_PANELS; i++)
	{
		float x = (float)rand() / (RAND_MAX);
		float y = (float)rand() / (RAND_MAX);
		float z = (float)rand() / (RAND_MAX);
		ui_transforms[i].position = glm::vec3(x, y, z);
		ui_transforms[i].position = (ui_transforms[i].position - 0.5f) * 2.0f;
		ui_transforms[i].position *= 1.0f;

		float sx = (float)rand() / (RAND_MAX);
		float sy = (float)rand() / (RAND_MAX);
		float sz = (float)rand() / (RAND_MAX);
		ui_transforms[i].scale = glm::vec3(0.1f);
		ui_visible[i] = 1.0f;

		float* head = glm::value_ptr(*ui_transforms[i].CalculateTransformMatr());

		memcpy(&ui_transformsFlattened[i * (int)16], head, 64);
		buttons[i].init(i);
	}

	Shader notesShaderProgram("default.vert", "default.frag");

	std::vector<float> transformsFlattened(NUM_NOTES * 16);
	srand((unsigned int)time(0));
	std::vector<Transform> transforms(NUM_NOTES);
	for (int i = 0; i < NUM_NOTES; i++)
	{
		float x = (float)rand() / (RAND_MAX);
		float y = (float)rand() / (RAND_MAX);
		float z = (float)rand() / (RAND_MAX);
		transforms[i].position = glm::vec3(x, y, z);
		transforms[i].position = (transforms[i].position - 0.5f) * 2.0f;
		transforms[i].position *= 10.0f;

		transforms[i].scale = glm::vec3(1.0f);

		float* head = glm::value_ptr(*transforms[i].CalculateTransformMatr());

		memcpy(&transformsFlattened[i * (int)16], head, 64);
	}

	//NOTES drawing VAOs and VBOs
	VAO notesVAO, uiVAO;
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


	std::string sampleString = "This is sample text.";

	BMFontReader reader("./fonts/TestFontDF.fnt");
	reader.read();

	Shader uiTextShader("UITextBasic.vert", "UITextBasic.frag");
	std::vector<float> textTransformsFlattened(16 * sampleString.length());
	std::vector<Transform> textTransforms(sampleString.length());
	std::unordered_map<char, Glyph> glyphsMap = reader.getGlyphs();
	std::vector<float> texCoords;
	std::vector<float> texCoordsB;
	float textCursor = 0.0f;
	float padding = 0.1f;
	glm::vec2 line = glm::vec2(0.0f);
	for (int i = 0; i < sampleString.length(); i++)
	{
		auto curGlyph = glyphsMap[sampleString[i]];
		//float yOffset = -(curGlyph.height - curGlyph.yOffset) / (2 * 512.0f);
		float yOffset = (0.0f - curGlyph.yOffset) / (512.0f);

		textTransforms[i].position = glm::vec3(textCursor * 2, yOffset * (curGlyph.height / 512.0f), 0.0f);
		textTransforms[i].scale = glm::vec3((curGlyph.width / 512.0f), (curGlyph.height / 512.0f), 1.0f);
		textTransforms[i].scale *= 0.25f;

		if (curGlyph.width == 0.0f && curGlyph.height == 0.0f) {
			textTransforms[i].scale = glm::vec3((curGlyph.xAdvance / 512.0f), 0.0f, 1.0f);
			textTransforms[i].scale *= 0.15f;
		}

		textTransforms[i].position -= glm::vec3(0.49f, 0.f, 0.0);
		textTransforms[i].position *= 2.0f;
		//std::cout << lastXpos << std::endl;
		//std::cout << textCursor * 2 << std::endl;
		textCursor += textTransforms[i].scale.x;


		float* head = glm::value_ptr(*textTransforms[i].CalculateTransformMatr());

		memcpy(&textTransformsFlattened[i * (int)16], head, 64);

		texCoords.push_back(curGlyph.x / 512.0f);
		texCoords.push_back((512.0f - (curGlyph.y + curGlyph.height)) / 512.0f);
		texCoords.push_back(0.0f);
		texCoords.push_back(0.0f);
		texCoords.push_back((curGlyph.x + curGlyph.width) / 512.0f);
		texCoords.push_back((512.0f - (curGlyph.y + curGlyph.height)) / 512.0f);
		texCoords.push_back(0.0f);
		texCoords.push_back(0.0f);
		texCoords.push_back((curGlyph.x + curGlyph.width) / 512.0f);
		texCoords.push_back((512.0f - (curGlyph.y)) / 512.0f);
		texCoords.push_back(0.0f);
		texCoords.push_back(0.0f);
		texCoords.push_back(curGlyph.x / 512.0f);
		texCoords.push_back((512.0f - curGlyph.y) / 512.0f);
		texCoords.push_back(0.0f);
		texCoords.push_back(0.0f);
	}

	VAO textVAO;

	//Text RENDERING UI
	textVAO.Bind();
	VBO textVBO(vertices, sizeof(vertices), GL_STATIC_DRAW);
	VBO textTextureCoordsVBO(texCoords.data(), sizeof(float) * texCoords.size(), GL_DYNAMIC_DRAW);
	VBO textTransformsVBO(textTransformsFlattened.data(), sizeof(float) * textTransformsFlattened.size(), GL_DYNAMIC_DRAW);
	textTransformsVBO.Bind();

	EBO textEBO(indices, sizeof(indices));
	textVAO.LinkAttrib(textVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	textVAO.LinkTransformAttrib(textTransformsVBO, 1);
	textVAO.LinkTransformAttrib(textTextureCoordsVBO, 5);
	glVertexAttribDivisor(9, 1);


	textVAO.Unbind();
	textVBO.Unbind();
	textTransformsVBO.Unbind();
	textEBO.Unbind();
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

	uiTextShader.Activate(); // don't forget to activate/use the shader before setting uniforms!
	// either set it manually like so:
	glUniform1i(glGetUniformLocation(uiTextShader.ID, "textureAtlas"), 0);

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

	while (!glfwWindowShouldClose(window))
	{

		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		Input::SetMouseInput(window);

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
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.trans.position += cameraSpeed * camera.trans.front;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.trans.position -= cameraSpeed * camera.trans.front;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.trans.position -= cameraSpeed * camera.trans.right;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.trans.position += cameraSpeed * camera.trans.right;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
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


		if (Input::leftMouseButtonPressed) {
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
				//std::cout << "ENTITY SELECTED: " << lastSelectedEntity << std::endl;
			}
		}

		uiTextShader.Activate();
		glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
		glBindTexture(GL_TEXTURE_2D, fontAtlas);
		textVAO.Bind();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, sampleString.length());
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
	textVAO.Delete();
	uiTextShader.Delete();
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