#pragma once

#include "ShaderClass.h"
#include "Camera.h"

#include "Note.h"
#include "UI.h"

void DrawNotes(Camera& camera, unsigned int NUM_NOTES, unsigned int NUM_UI_PANELS, int lastSelectedEntity, int lastSelectedEntityDelete) {
	
	glBindFramebuffer(GL_FRAMEBUFFER, camera.FBO);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Note::notesShaderProgram->Activate();

	int persprojLoc = glGetUniformLocation(Note::notesShaderProgram->ID, "perspectiveProj");
	glUniformMatrix4fv(persprojLoc, 1, GL_FALSE, glm::value_ptr(camera.projection));

	int cameraTransLoc = glGetUniformLocation(Note::notesShaderProgram->ID, "cameraTrans");
	glUniformMatrix4fv(cameraTransLoc, 1, GL_FALSE, glm::value_ptr(*camera.trans.CalculateTransformMatr()));
	
	int entitiesCountLoc = glGetUniformLocation(Note::notesShaderProgram->ID, "entitiesCount");
	glUniform1f(entitiesCountLoc, NUM_NOTES + NUM_UI_PANELS);
	int uiEntitiesCountLocA = glGetUniformLocation(Note::notesShaderProgram->ID, "uiEntitiesCount");
	glUniform1f(uiEntitiesCountLocA, NUM_UI_PANELS);

	int lastSelectedEntityLoc = glGetUniformLocation(Note::notesShaderProgram->ID, "lastSelectedEntity");
	glUniform1i(lastSelectedEntityLoc, lastSelectedEntity);
	int lastClickedEntityLoc = glGetUniformLocation(Note::notesShaderProgram->ID, "lastClickedEntity");
	glUniform1i(lastClickedEntityLoc, lastSelectedEntityDelete);

	Note::notesVAO.Bind();
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, NUM_NOTES);

}

void DrawUI(unsigned int NUM_NOTES, unsigned int NUM_UI_PANELS, int lastSelectedUI) {

	UI_MANAGER::uiShaderProgram.Activate();
	int uiEntitiesCountLoc = glGetUniformLocation(UI_MANAGER::uiShaderProgram.ID, "entitiesCount");
	glUniform1f(uiEntitiesCountLoc, NUM_UI_PANELS + NUM_NOTES);
	int UIlastSelectedEntityLoc = glGetUniformLocation(UI_MANAGER::uiShaderProgram.ID, "lastSelectedUI");
	glUniform1i(UIlastSelectedEntityLoc, lastSelectedUI);

	UI_MANAGER::uiVAO.Bind();

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, NUM_UI_PANELS);


}

void DrawText(Camera& camera) {

	TextArea::textShader->Activate();
	int persprojLocUIT = glGetUniformLocation(Note::notesShaderProgram->ID, "perspectiveProj");
	glUniformMatrix4fv(persprojLocUIT, 1, GL_FALSE, glm::value_ptr(camera.projection));
	int cameraTransLocUIT = glGetUniformLocation(Note::notesShaderProgram->ID, "cameraTrans");
	glUniformMatrix4fv(cameraTransLocUIT, 1, GL_FALSE, glm::value_ptr(*camera.trans.CalculateTransformMatr()));

	glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, TextArea::fontAtlas);
	//textVAO.Bind();
	TextArea::textVAO.Bind();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, TextArea::totalGlyphs);
}

void DrawBackShadow(Camera& camera, Shader& blurShader, unsigned int NUM_NOTES) {

	glBindFramebuffer(GL_FRAMEBUFFER, camera.blurFBO);

	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	blurShader.Activate();
	int persprojLoc = glGetUniformLocation(blurShader.ID, "perspectiveProj");
	glUniformMatrix4fv(persprojLoc, 1, GL_FALSE, glm::value_ptr(camera.projection));
	int cameraTransLoc = glGetUniformLocation(blurShader.ID, "cameraTrans");
	glUniformMatrix4fv(cameraTransLoc, 1, GL_FALSE, glm::value_ptr(*camera.trans.CalculateTransformMatr()));

	Note::notesVAO.Bind();
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, NUM_NOTES);
	glActiveTexture(GL_TEXTURE0);
}

void DrawToScreen(Camera& camera, GLFWwindow* window) {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	camera.fbShaderProgram.Activate();
	glBindVertexArray(camera.screenRectVAO);

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
}