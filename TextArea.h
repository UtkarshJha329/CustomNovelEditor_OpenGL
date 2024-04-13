#pragma once

#include <vector>
#include <string>

#include "BMFontReader.h"
#include "Transform.h"
#include "ShaderClass.h"

#include "stb_image.h"

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
	static std::vector<float> isUIArray;
	
	static std::vector<float> textIsVisible;
	inline static std::vector<float> individualLengths;
	static int totalGlyphs;
	float textCursor = 0.0f;

	inline static int NUM_CHARS_IN_TEXTAREA = 400;

	int flattenedTransformStartIndex = 0;
	int flattenedTransformEndIndex = 0;

	inline static unsigned int fontAtlas = 0;
	
	//Use only after creating TextArea::textShader;
	static void ReadAndBindTexture() {

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

	}

	void RemoveLastCharFromTextArea(int noteIndex) {

		int currentLastGlyphIndex = (noteIndex + 5) * NUM_CHARS_IN_TEXTAREA + individualLengths[noteIndex + 5] - 1;

		//std::cout << noteIndex << " : " << currentLastGlyphIndex << std::endl;

		auto glyphsMap = reader->getGlyphs();

		textIsVisible[currentLastGlyphIndex] = 1.0f;

		auto curGlyph = glyphsMap[' '];

		isUIArray[currentLastGlyphIndex] = IsUI;

		texCoords[currentLastGlyphIndex * 16 + 0] = (curGlyph.x / 512.0f);
		texCoords[currentLastGlyphIndex * 16 + 1] = ((512.0f - (curGlyph.y + curGlyph.height)) / 512.0f);
		texCoords[currentLastGlyphIndex * 16 + 2] = (0.0f);
		texCoords[currentLastGlyphIndex * 16 + 3] = (0.0f);
		texCoords[currentLastGlyphIndex * 16 + 4] = ((curGlyph.x + curGlyph.width) / 512.0f);
		texCoords[currentLastGlyphIndex * 16 + 5] = ((512.0f - (curGlyph.y + curGlyph.height)) / 512.0f);
		texCoords[currentLastGlyphIndex * 16 + 6] = (0.0f);
		texCoords[currentLastGlyphIndex * 16 + 7] = (0.0f);
		texCoords[currentLastGlyphIndex * 16 + 8] = ((curGlyph.x + curGlyph.width) / 512.0f);
		texCoords[currentLastGlyphIndex * 16 + 9] = ((512.0f - (curGlyph.y)) / 512.0f);
		texCoords[currentLastGlyphIndex * 16 + 10] = (0.0f);
		texCoords[currentLastGlyphIndex * 16 + 11] = (0.0f);
		texCoords[currentLastGlyphIndex * 16 + 12] = (curGlyph.x / 512.0f);
		texCoords[currentLastGlyphIndex * 16 + 13] = ((512.0f - curGlyph.y) / 512.0f);
		texCoords[currentLastGlyphIndex * 16 + 14] = (0.0f);
		texCoords[currentLastGlyphIndex * 16 + 15] = (0.0f);

		sampleString[individualLengths[noteIndex + 5] - 1] = ' ';
		individualLengths[noteIndex + 5]--;

		//std::cout << sampleString.substr(0, individualLengths[noteIndex + 5]) << std::endl;

		TextArea::textTextureCoordsVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * currentLastGlyphIndex * 16, sizeof(float) * 16, &TextArea::texCoords[currentLastGlyphIndex * 16]);
		TextArea::textTextureCoordsVBO.Unbind();
	}

	void FlipVisibility(int start) {

		float startLocation = 0.0f;
		for (int i = 0; i < start; i++)
		{
			startLocation += NUM_CHARS_IN_TEXTAREA;
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
			startLocation += NUM_CHARS_IN_TEXTAREA;
			//std::cout << "StartLocation: " << startLocation << ", IndividualLengths: " << individualLengths[i] << std::endl;
		}
		for (int i = 0; i < glyphTrans.size(); i++)
		{
			textIsVisible[startLocation + i] = visibility;
		}
		//std::cout << "(START - NUMIP_PANELS)* 126 + NUM_UI_PANELS * 6: " << (start - 5) * 126 + 30 << std::endl;
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
			//std::cout << "COPIED TRANS : " << ID  << " : " << textAreaTransformsFlattened.size() << std::endl;

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

			//std::cout << "NEW SAMPLE STRING READING" << std::endl;

			for (int j = 0; j < NUM_CHARS_IN_TEXTAREA; j++)
			{
				if (sampleString.length() != NUM_CHARS_IN_TEXTAREA) {
					sampleString += " ";
				}
				else {
					break;
				}
			}

			for (int i = 0; i < NUM_CHARS_IN_TEXTAREA; i++)
			{
				textIsVisible.push_back(1.0f);
				//std::cout << textIsVisible.size() << std::endl;
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

				isUIArray.push_back(IsUI);
				//std::cout << isUIArray.size() << " : " << isUIArray[isUIArray.size() - 1] << std::endl;

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
		}
	}

	void ChangeText(const std::string& newText, int start) {

		int currentIndex = (start + 5) * NUM_CHARS_IN_TEXTAREA;

		sampleString = sampleString.substr(0, individualLengths[start + 5]);
		sampleString += newText;

		individualLengths[start + 5] = sampleString.length();


		if (sampleString.length() < NUM_CHARS_IN_TEXTAREA) {
			for (int i = 0; i < NUM_CHARS_IN_TEXTAREA; i++)
			{
				if (sampleString.length() < NUM_CHARS_IN_TEXTAREA) {
					sampleString += " ";
				}
				else {
					break;
				}
			}
		}
		else {
			sampleString = sampleString.substr(0, NUM_CHARS_IN_TEXTAREA);
		}

		glyphTrans.clear();

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

		textCursor = 0.0f;
		totalGlyphs -= NUM_CHARS_IN_TEXTAREA;
		for (int i = 0; i < sampleString.length(); i++)
		{
			textIsVisible[currentIndex + i] = 1.0f;

			totalGlyphs++;
			auto curGlyph = glyphsMap[sampleString[i]];
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
			if (curHeight > height) {
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

			isUIArray[currentIndex + i] = IsUI;

			float* head = glm::value_ptr(*trans.CalculateTransformMatr());

			memcpy(&textTransformsFlattened[currentIndex * 16 + i * 16], head, sizeof(float) * 16);


			glyphTrans.push_back(trans);
			texCoords[currentIndex * 16 + i * 16 + 0] = (curGlyph.x / 512.0f);
			texCoords[currentIndex * 16 + i * 16 + 1] = ((512.0f - (curGlyph.y + curGlyph.height)) / 512.0f);
			texCoords[currentIndex * 16 + i * 16 + 2] = (0.0f);
			texCoords[currentIndex * 16 + i * 16 + 3] = (0.0f);
			texCoords[currentIndex * 16 + i * 16 + 4] = ((curGlyph.x + curGlyph.width) / 512.0f);
			texCoords[currentIndex * 16 + i * 16 + 5] = ((512.0f - (curGlyph.y + curGlyph.height)) / 512.0f);
			texCoords[currentIndex * 16 + i * 16 + 6] = (0.0f);
			texCoords[currentIndex * 16 + i * 16 + 7] = (0.0f);
			texCoords[currentIndex * 16 + i * 16 + 8] = ((curGlyph.x + curGlyph.width) / 512.0f);
			texCoords[currentIndex * 16 + i * 16 + 9] = ((512.0f - (curGlyph.y)) / 512.0f);
			texCoords[currentIndex * 16 + i * 16 + 10] = (0.0f);
			texCoords[currentIndex * 16 + i * 16 + 11] = (0.0f);
			texCoords[currentIndex * 16 + i * 16 + 12] = (curGlyph.x / 512.0f);
			texCoords[currentIndex * 16 + i * 16 + 13] = ((512.0f - curGlyph.y) / 512.0f);
			texCoords[currentIndex * 16 + i * 16 + 14] = (0.0f);
			texCoords[currentIndex * 16 + i * 16 + 15] = (0.0f);


		}

		//std::cout << start << ", " << currentIndex * 16 << std::endl;

		TextArea::textTransformsVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * currentIndex * 16, sizeof(float) * sampleString.length() * 16, &TextArea::textTransformsFlattened[currentIndex * 16]);
		TextArea::textTransformsVBO.Unbind();

		TextArea::textTextureCoordsVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * currentIndex * 16, sizeof(float) * sampleString.length() * 16, &TextArea::texCoords[currentIndex * 16]);
		TextArea::textTextureCoordsVBO.Unbind();

		textIsVisibleVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * currentIndex, sizeof(float) * sampleString.length(), &TextArea::textIsVisible[currentIndex]);
		textIsVisibleVBO.Unbind();
	}

	inline static VAO textVAO;
	inline static VBO textVBO;
	inline static VBO textIsUIVBO;
	inline static VBO textTextureCoordsVBO;
	inline static VBO textTransformsVBO;
	inline static VBO textAreaTransformsVBO;
	inline static VBO textIsVisibleVBO;
	inline static EBO textEBO;

	static void BindVAOsVBOsEBOs(float* textAreavertices, unsigned int* textAreaindices, unsigned int EXTRA_ALLOCATION) {

		textVAO.Init();

		//Text RENDERING UI
		textVAO.Bind();
		textVBO.Init(textAreavertices, sizeof(float) * 12, GL_STATIC_DRAW);
		textIsUIVBO.Init(TextArea::isUIArray.data(), sizeof(float) * (TextArea::isUIArray.size() + EXTRA_ALLOCATION), GL_STATIC_DRAW);
		//std::cout << "WHILE BINDING SIZE OF TEXCOORDS: " << texCoords.size() << std::endl;
		textTextureCoordsVBO.Init(TextArea::texCoords.data(), sizeof(float) * (TextArea::texCoords.size() + EXTRA_ALLOCATION * 16), GL_DYNAMIC_DRAW);
		//std::cout << "WHILE BINDING SIZE OF flattnedTransforms: " << textTransformsFlattened.size() << std::endl;
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
