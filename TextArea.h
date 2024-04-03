#pragma once

#include <vector>
#include <string>

#include "BMFontReader.h"
#include "Transform.h"
#include "ShaderClass.h"

#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

class TextArea {

public:

	TextArea() {
		ID = totalTextAreas + 1;
		totalTextAreas++;
	}

	int ID = 0;

	float width = 0.0f;
	float height = 0.0f;

	float lineHeight = 0.25f;

	std::string text = "";
	float gapBetweenLines = 0.0f;
	std::vector<std::string> words;

	std::string sampleString = "This is sample text.";

	Transform transform;
	std::vector<Transform> glyphTrans;
	float IsUI = 0.0f;

	static int totalTextAreas;

	static BMFontReader* reader;

	static Shader* textShader;
	static std::vector<float> textTransformsFlattened;	
	static std::vector<float> texCoords;
	static std::vector<float> isUI;
	static int totalGlyphs;
	float textCursor = 0.0f;

	int flattenedTransformStartIndex = 0;
	int flattenedTransformEndIndex = 0;

	void FillGlobalTextArrays(std::vector<float>& values, glm::vec3 offset = glm::vec3(0.0f) , int start = -1, int end = -1) {

		if (start != -1 && end != -1) {

			//std::cout << "HERE!!!" << std::endl;

			for (int i = 0; i < glyphTrans.size(); i++)
			{
				glyphTrans[i].position += offset;

				float* head = glm::value_ptr(*glyphTrans[i].CalculateTransformMatr());

				for (int j = 0; j < 16; j++)
				{
					textTransformsFlattened[start + (i) * 16 + j] = head[j];
					values.push_back(head[j]);
				}
			}


		}
		else {
			glyphTrans.clear();
			//std::cout << "HERE INSTEAD" << std::endl;
			auto glyphsMap = reader->getGlyphs();
			float curWidth = 0;
			float curHeight = 0;
			float yLine = 0.0f;
			float xOffset = 0.0f;

			if (!IsUI) {
				yLine = 0.5f;
				xOffset = 0.15f;
			}
			else {
				xOffset = 0.02f;
			}

			for (int i = 0; i < sampleString.length(); i++)
			{
				totalGlyphs++;
				auto curGlyph = glyphsMap[sampleString[i]];
				//float yOffset = -(curGlyph.height - curGlyph.yOffset) / (2 * 512.0f);
				float yOffset = (0.0f - curGlyph.yOffset) / (512.0f);

				Transform trans;
				trans.position += transform.position;

				trans.position += glm::vec3(xOffset + textCursor * 2, -yLine * lineHeight + yOffset * (curGlyph.height / 512.0f) * 2.0f, 0.0f);
				trans.scale = glm::vec3((curGlyph.width / 512.0f), (curGlyph.height / 512.0f), 1.0f);
				trans.scale *= IsUI ? 0.15f : 1.0f;

				if (curGlyph.width == 0.0f && curGlyph.height == 0.0f) {
					trans.scale = glm::vec3((curGlyph.xAdvance / 512.0f), 0.0f, 1.0f);
					trans.scale *= (IsUI) ? 0.15f : 0.35f;
				}

				//trans.position -= glm::vec3(0.49f, 0.f, 0.0);
				//trans.position *= 2.0f;
				//std::cout << lastXpos << std::endl;
				//std::cout << textCursor * 2 << std::endl;
				curWidth = (trans.position.x - transform.position.x);
				//std::cout << curWidth << " : " << width << std::endl;
				if (curWidth + xOffset > width * 2) {
					trans.position -= glm::vec3(textCursor * 2, -yLine * lineHeight + yOffset * (curGlyph.height / 512.0f) * 2.0f, 0.0f);

					if (!IsUI) {
						yLine++;
					}
					else {
						yLine += 0.25f;
					}

					textCursor = 0;

					trans.position += glm::vec3(textCursor * 2, -yLine * lineHeight + yOffset * (curGlyph.height / 512.0f) * 2.0f, 0.0f);

				}

				curHeight = transform.position.y - trans.position.y;
				if (curHeight > height * 2) {
					trans.scale.y = 0.0f;
				}

				textCursor += trans.scale.x;
				if (IsUI) {
					trans.position.z -= 0.1f;
				}
				else {
					trans.position.z += 0.1f;
				}

				isUI.push_back(IsUI);
				//std::cout << isUI.size() << " : " << isUI[isUI.size() - 1] << std::endl;

				float* head = glm::value_ptr(*trans.CalculateTransformMatr());

				if (i == 0) {
					flattenedTransformStartIndex = textTransformsFlattened.size();
				}
				if (i == sampleString.length() - 1) {
					flattenedTransformEndIndex = textTransformsFlattened.size();
				}

				for (int j = 0; j < 16; j++)
				{
					textTransformsFlattened.push_back(head[j]);
					values.push_back(head[j]);
				}
				

				glyphTrans.push_back(trans);

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

		}
	}

	inline static VAO textVAO;
	inline static VBO textVBO;
	inline static VBO textIsUIVBO;
	inline static VBO textTextureCoordsVBO;
	inline static VBO textTransformsVBO;
	inline static EBO textEBO;

	static void BindVAOsVBOsEBOs(float* textAreavertices, unsigned int* textAreaindices) {

		textVAO.Init();

		//Text RENDERING UI
		textVAO.Bind();
		textVBO.Init(textAreavertices, sizeof(float) * 12, GL_STATIC_DRAW);
		textIsUIVBO.Init(TextArea::isUI.data(), sizeof(float) * TextArea::isUI.size(), GL_STATIC_DRAW);
		textTextureCoordsVBO.Init(TextArea::texCoords.data(), sizeof(float) * TextArea::texCoords.size(), GL_DYNAMIC_DRAW);
		textTransformsVBO.Init(TextArea::textTransformsFlattened.data(), sizeof(float) * TextArea::textTransformsFlattened.size(), GL_DYNAMIC_DRAW);
		textTransformsVBO.Bind();

		textEBO.Init(textAreaindices, sizeof(unsigned int) * 6);
		textVAO.LinkAttrib(textVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
		textVAO.LinkTransformAttrib(textTransformsVBO, 1);
		textVAO.LinkTransformAttrib(textTextureCoordsVBO, 5);
		textVAO.LinkAttrib(textIsUIVBO, 9, 1, GL_FLOAT, sizeof(float), (void*)0);
		glVertexAttribDivisor(9, 1);


		textVAO.Unbind();
		textVBO.Unbind();
		textTransformsVBO.Unbind();
		textEBO.Unbind();
	}
};
