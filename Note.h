#pragma once

#include "TextArea.h"
#include "Transform.h"

class Note {

public:

	Transform transform;
	TextArea textArea;

<<<<<<< Updated upstream
	inline static float width = 2.0f;
	inline static float height = 2.0f;
=======
	inline static float width = 1.0f;
	inline static float height = 1.0f;

	static std::vector<float> flattenedTransforms;
	static std::vector<float> visible;
>>>>>>> Stashed changes

	inline static VAO notesVAO;
	inline static VBO notesQuadVBO;
	inline static VBO notesTransformsVBO;
	inline static VBO notesVisibilityVBO;

	inline static EBO notesEBO;

	static void InitVAOsVBOsEBOs(float* vertices, unsigned int* indicies, unsigned int EXTRA_BUFFER_ALLOCATION) {
		
		notesVAO.Init();
		notesVAO.Bind();

		notesQuadVBO.Init(vertices, sizeof(float) * 12, GL_STATIC_DRAW);
		//VBO notesTransformsVBO(offsets.data(), offsets.size());
		notesTransformsVBO.Init(flattenedTransforms.data(), sizeof(float) * flattenedTransforms.size() + EXTRA_BUFFER_ALLOCATION * 16 * sizeof(float), GL_DYNAMIC_DRAW);
		notesVisibilityVBO.Init(visible.data(), sizeof(float) * visible.size() + EXTRA_BUFFER_ALLOCATION * sizeof(float), GL_DYNAMIC_DRAW);
		//std::cout << notesTransformsFlattened.size() / 16 << std::endl;
		notesTransformsVBO.Bind();

		notesEBO.Init(indicies, sizeof(unsigned int) * 6);
		notesVAO.LinkTransformAttrib(notesTransformsVBO, 1);
		notesVAO.LinkAttrib(notesQuadVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
		notesVAO.LinkAttrib(notesVisibilityVBO, 5, 1, GL_FLOAT, sizeof(float), (void*)0);
		glVertexAttribDivisor(5, 1);

		notesVAO.Unbind();
		notesQuadVBO.Unbind();
		notesTransformsVBO.Unbind();
		notesVisibilityVBO.Unbind();
		notesEBO.Unbind();
	}
};