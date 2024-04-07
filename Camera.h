#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Transform.h"
#include "ShaderClass.h"

class Camera {

public:

	Transform trans;

	unsigned int FBO;
	unsigned int framebuffersTexture;
	unsigned int entityColourTextureID;
	unsigned int entityIDTextureID;
	unsigned int RBO;

	unsigned int blurFBO;
	unsigned int blurTexture;
	unsigned int blurRBO;

	Shader fbShaderProgram;
	Shader blurShaderProgram;

	glm::mat4 projection;

	void InitCamera(float width, float height) {
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glGenTextures(1, &framebuffersTexture);
		glBindTexture(GL_TEXTURE_2D, framebuffersTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffersTexture, 0);

		glGenTextures(1, &entityColourTextureID);
		glBindTexture(GL_TEXTURE_2D, entityColourTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, entityColourTextureID, 0);

		glGenTextures(1, &entityIDTextureID);
		glBindTexture(GL_TEXTURE_2D, entityIDTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, entityIDTextureID, 0);

		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		//unsigned int attachments2[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(3, attachments);


		auto fbostatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (fbostatus != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		fbShaderProgram.Init("framebuffer.vert", "framebuffer.frag");
		glUniform1i(glGetUniformLocation(fbShaderProgram.ID, "screenTexture"), 0);


		glGenFramebuffers(1, &blurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);

		glGenTextures(1, &blurTexture);
		glBindTexture(GL_TEXTURE_2D, blurTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture, 0);

		//glGenRenderbuffers(1, &RBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		unsigned int attachmentsTwo[1] = { GL_COLOR_ATTACHMENT0 };
		//unsigned int attachments2[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachmentsTwo);

		fbostatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (fbostatus != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		blurShaderProgram.Init("BlurPass.vert", "BlurPass.frag");
	}

};