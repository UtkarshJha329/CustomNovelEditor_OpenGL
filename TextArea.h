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

	float fontSize = 28.0f;

	static int totalTextAreas;

	static BMFontReader* reader;

	static Shader* textShader;
	static std::vector<float> textTransformsFlattened;	
	static std::vector<float> texCoords;
	static std::vector<float> isUI;
	
	static std::vector<float> textIsVisible;
	inline static std::vector<float> individualLengths;
	static int totalGlyphs;
	float textCursor = 0.0f;

	int flattenedTransformStartIndex = 0;
	int flattenedTransformEndIndex = 0;

	void FlipVisibility(int start) {

		float startLocation = 0.0f;
		for (int i = 0; i < start; i++)
		{
			startLocation += 400;
			//std::cout << "StartLocation: " << startLocation << ", IndividualLengths: " << individualLengths[i] << std::endl;
		}

		for (int i = 0; i < glyphTrans.size(); i++)
		{
			int curVisibility = textIsVisible[startLocation + i];

			if (curVisibility == 0.0f) {
				textIsVisible[startLocation + i] = 1.0f;
			}
			else {
				textIsVisible[startLocation + i] = 0.0f;
			}
		}
		//std::cout << "(START - NUMIP_PANELS)* 87 + NUM_UI_PANELS * 6: " << (start - 5) * 87 + 30 << std::endl;


		textIsVisibleVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * textIsVisible.size(), &textIsVisible[0]);
		textIsVisibleVBO.Unbind();
	}

	void SetVisibility(int start = 0,  float visibility = 1.0f) {
		
		float startLocation = 0.0f;
		for (int i = 0; i < start; i++)
		{
			startLocation += 400;
			//std::cout << "StartLocation: " << startLocation << ", IndividualLengths: " << individualLengths[i] << std::endl;
		}
		for (int i = 0; i < glyphTrans.size(); i++)
		{
			textIsVisible[startLocation + i] = visibility;
		}
		//std::cout << "(START - NUMIP_PANELS)* 126 + NUM_UI_PANELS * 6: " << (start - 5) * 126 + 30 << std::endl;
	}

	void AddToString(const std::string& stringToAdd) {


	}

	void FillGlobalTextArrays(std::vector<float>& values, glm::vec3 offset = glm::vec3(0.0f) , int start = -1, int end = -1) {

		if (start != -1 && end != -1) {

			//std::cout << "HERE!!!" << std::endl;

			for (int i = 0; i < glyphTrans.size(); i++)
			{
				glyphTrans[i].position += offset;

				float* head = glm::value_ptr(*glyphTrans[i].CalculateTransformMatr());
				//textIsVisible[start + i] = visibility;

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

			std::cout << "NEW SAMPLE STRING READING" << std::endl;

			for (int j = 0; j < 400; j++)
			{
				if (sampleString.length() != 400) {
					sampleString += " ";
				}
				else {
					break;
				}
			}

			for (int i = 0; i < sampleString.length(); i++)
			{
				textIsVisible.push_back(1.0f);

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

				if (!IsUI) {
					trans.scale.x *= fontSize / 91;
					trans.scale.y *= fontSize / 91;
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
			/*std::cout << " Individual Length: " << individualLengths[individualLengths.size() - 1] << std::endl;
			std::cout << textIsVisible.size() << std::endl;*/

			//individualLengths[individualLengths.size() - 1] = 400;

		}
	}

	void ChangeText(const std::string& newText, int start) {
		
		int currentIndex = (start + 5) * 400;

		sampleString = sampleString.substr(0, individualLengths[start + 5]);
		//std::cout << individualLengths[start + 5] << " : Truncated : " << sampleString << " : " << sampleString.length() << std::endl;
		sampleString += newText;
		
		//std::cout << newText << std::endl;
		//std::cout << individualLengths[start + 5] << " : " << sampleString << " : " << sampleString.length() << std::endl;

		individualLengths[start + 5] = sampleString.length();

		if (sampleString.length() < 400) {
			for (int i = 0; i < 400; i++)
			{
				if (sampleString.length() < 400) {
					sampleString += " ";
					//std::cout << "Padded." << std::endl;
				}
				else {
					break;
				}
			}
		}
		else {
			sampleString = sampleString.substr(0, 400);
		}

		//std::cout << sampleString << std::endl;

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
		
		std::cout << sampleString.length() << " : " << sampleString << std::endl;
		totalGlyphs -= 400;
		for (int i = 0; i < sampleString.length(); i++)
		{
			textIsVisible[currentIndex + i] = 1.0f;
			//std::cout << textIsVisible.size() << std::endl;

			totalGlyphs++;
			auto curGlyph = glyphsMap[sampleString[i]];
			//std::cout << (char)curGlyph.id << std::endl;
			//std::cout << currentIndex << " : " << i << " : " << curGlyph.width << ", " << curGlyph.height << std::endl;
			//std::cout << sampleString[i] << std::endl;
			//float yOffset = -(curGlyph.height - curGlyph.yOffset) / (2 * 512.0f);
			float yOffset = (0.0f - curGlyph.yOffset) / (512.0f);

			Transform trans;
			trans.position += transform.position + glm::vec3(-1.0f, 1.0f, 0.0f);

			trans.position += glm::vec3(xOffset + textCursor * 2, -yLine * lineHeight + yOffset * (curGlyph.height / 512.0f) * 2.0f, 0.0f);
			trans.scale = glm::vec3((curGlyph.width / 512.0f), (curGlyph.height / 512.0f), 1.0f);
			trans.scale *= IsUI ? 0.15f : 1.0f;

			if (curGlyph.width == 0.0f && curGlyph.height == 0.0f) {
				trans.scale = glm::vec3((curGlyph.xAdvance / 512.0f), 0.0f, 1.0f);
				trans.scale *= (IsUI) ? 0.15f : 0.35f;
			}

			curWidth = (trans.position.x - transform.position.x);
			if (curWidth + xOffset > width) {
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
				//std::cout << "HIDDEN VERTICALLY." << std::endl;
				trans.scale.y = 0.0f;
			}

			if (!IsUI) {
				trans.scale.x *= fontSize / 91;
				trans.scale.y *= fontSize / 91;
			}

			textCursor += trans.scale.x;
			if (IsUI) {
				trans.position.z -= 0.1f;
			}
			else {
				trans.position.z += 0.1f;
			}

			isUI[currentIndex + i] = IsUI;
			//std::cout << isUI.size() << " : " << isUI[isUI.size() - 1] << std::endl;

			float* head = glm::value_ptr(*trans.CalculateTransformMatr());

			//for (int j = 0; j < 16; j++)
			//{
			//	textTransformsFlattened[currentIndex + i * 16 + j] = (head[j]);
			//}

			memcpy(&textTransformsFlattened[currentIndex + i * 16], head, sizeof(float) * 16);


			glyphTrans.push_back(trans);
			//std::cout << currentIndex + i * 16 << std::endl;
			texCoords[currentIndex + i * 16 + 0] = (curGlyph.x / 512.0f);
			texCoords[currentIndex + i * 16 + 1] = ((512.0f - (curGlyph.y + curGlyph.height)) / 512.0f);
			texCoords[currentIndex + i * 16 + 2] = (0.0f);
			texCoords[currentIndex + i * 16 + 3] = (0.0f);
			texCoords[currentIndex + i * 16 + 4] = ((curGlyph.x + curGlyph.width) / 512.0f);
			texCoords[currentIndex + i * 16 + 5] = ((512.0f - (curGlyph.y + curGlyph.height)) / 512.0f);
			texCoords[currentIndex + i * 16 + 6] = (0.0f);
			texCoords[currentIndex + i * 16 + 7] = (0.0f);
			texCoords[currentIndex + i * 16 + 8] = ((curGlyph.x + curGlyph.width) / 512.0f);
			texCoords[currentIndex + i * 16 + 9] = ((512.0f - (curGlyph.y)) / 512.0f);
			texCoords[currentIndex + i * 16 + 10] = (0.0f);
			texCoords[currentIndex + i * 16 + 11] = (0.0f);
			texCoords[currentIndex + i * 16 + 12] = (curGlyph.x / 512.0f);
			texCoords[currentIndex + i * 16 + 13] = ((512.0f - curGlyph.y) / 512.0f);
			texCoords[currentIndex + i * 16 + 14] = (0.0f);
			texCoords[currentIndex + i * 16 + 15] = (0.0f);


		}

		std::cout << "TOTAL GLYPHS : " << totalGlyphs << std::endl;

		TextArea::textTransformsVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * TextArea::textTransformsFlattened.size(), TextArea::textTransformsFlattened.data());
		TextArea::textTransformsVBO.Unbind();

		TextArea::textTextureCoordsVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * TextArea::texCoords.size(), TextArea::texCoords.data());
		TextArea::textTextureCoordsVBO.Unbind();

		textIsVisibleVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * TextArea::textIsVisible.size(), TextArea::textIsVisible.data());
		textIsVisibleVBO.Unbind();
	}

	inline static VAO textVAO;
	inline static VBO textVBO;
	inline static VBO textIsUIVBO;
	inline static VBO textTextureCoordsVBO;
	inline static VBO textTransformsVBO;
	inline static VBO textIsVisibleVBO;
	inline static EBO textEBO;

	static void BindVAOsVBOsEBOs(float* textAreavertices, unsigned int* textAreaindices, unsigned int EXTRA_ALLOCATION) {

		textVAO.Init();

		//Text RENDERING UI
		textVAO.Bind();
		textVBO.Init(textAreavertices, sizeof(float) * 12, GL_STATIC_DRAW);
		textIsUIVBO.Init(TextArea::isUI.data(), sizeof(float) * (TextArea::isUI.size() + EXTRA_ALLOCATION), GL_STATIC_DRAW);
		std::cout << "WHILE BINDING SIZE OF TEXCOORDS: " << texCoords.size() << std::endl;
		textTextureCoordsVBO.Init(TextArea::texCoords.data(), sizeof(float) * (TextArea::texCoords.size() + EXTRA_ALLOCATION), GL_DYNAMIC_DRAW);
		std::cout << "WHILE BINDING SIZE OF flattnedTransforms: " << textTransformsFlattened.size() << std::endl;
		textTransformsVBO.Init(TextArea::textTransformsFlattened.data(), sizeof(float) * (TextArea::textTransformsFlattened.size() + EXTRA_ALLOCATION * 16), GL_DYNAMIC_DRAW);
		textIsVisibleVBO.Init(TextArea::textIsVisible.data(), sizeof(float) * (TextArea::textIsVisible.size() + EXTRA_ALLOCATION), GL_DYNAMIC_DRAW);
		textTransformsVBO.Bind();

		textEBO.Init(textAreaindices, sizeof(unsigned int) * 6);
		textVAO.LinkAttrib(textVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
		textVAO.LinkTransformAttrib(textTransformsVBO, 1);
		textVAO.LinkTransformAttrib(textTextureCoordsVBO, 5);
		textVAO.LinkAttrib(textIsUIVBO, 9, 1, GL_FLOAT, sizeof(float), (void*)0);
		textVAO.LinkAttrib(textIsVisibleVBO, 10, 1, GL_FLOAT, sizeof(float), (void*)0);
		glVertexAttribDivisor(9, 1);
		glVertexAttribDivisor(10, 1);


		textVAO.Unbind();
		textVBO.Unbind();
		textTransformsVBO.Unbind();
		textEBO.Unbind();
	}
};
