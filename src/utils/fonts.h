#include "vk_types.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>


namespace pm {

// AngelCode .fnt format structs and classes
struct bmchar {
	uint32_t x, y;
	uint32_t width;
	uint32_t height;
	int32_t xoffset;
	int32_t yoffset;
	int32_t xadvance;
	uint32_t page;
};

inline int32_t nextValuePair(std::stringstream* stream) {
	std::string pair;
	*stream >> pair;
	size_t spos = pair.find("=");
	std::string value = pair.substr(spos + 1);
	int32_t val = std::stoi(value);
	return val;
}

// Basic parser for AngelCode bitmap font format files
// See http://www.angelcode.com/products/bmfont/doc/file_format.html for details
inline std::array<bmchar, 255> parsebmFont(std::string_view fileName) {
	std::filebuf fileBuffer;
	fileBuffer.open(fileName.data(), std::ios::in);
	std::istream istream(&fileBuffer);

	std::array<bmchar, 255> fontChars{};

	assert(istream.good());

	while (!istream.eof()) {
		std::string line;
		std::stringstream lineStream;
		std::getline(istream, line);
		lineStream << line;

		std::string info;
		lineStream >> info;

		if (info == "char") {
			// char id
			uint32_t charid = nextValuePair(&lineStream);
			// Char properties
			fontChars[charid].x = nextValuePair(&lineStream);
			fontChars[charid].y = nextValuePair(&lineStream);
			fontChars[charid].width = nextValuePair(&lineStream);
			fontChars[charid].height = nextValuePair(&lineStream);
			fontChars[charid].xoffset = nextValuePair(&lineStream);
			fontChars[charid].yoffset = nextValuePair(&lineStream);
			fontChars[charid].xadvance = nextValuePair(&lineStream);
			fontChars[charid].page = nextValuePair(&lineStream);
		}
	}

	return fontChars;
}

inline void generateTextFromFont(std::string text, float textureWidth, std::array<bmchar, 255>& fontChars, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices, uint32_t& indexCount) {
	uint32_t indexOffset = 0;

	float posx = 0.0f;
	float posy = 0.0f;

	float SCALING_CONSTANT = 0.5f;

	for (uint32_t i = 0; i < text.size(); i++) {
		bmchar* charInfo = &fontChars[(int)text[i]];

		if (charInfo->width == 0)
			charInfo->width = 36;

		float charw = ((float)(charInfo->width) * SCALING_CONSTANT);
		float dimx = 1.0f * charw;
		float charh = ((float)(charInfo->height) * SCALING_CONSTANT);
		float dimy = 1.0f * charh;

		float us = charInfo->x / textureWidth;
		float ue = (charInfo->x + charInfo->width) / textureWidth;
		float ts = charInfo->y / textureWidth;
		float te = (charInfo->y + charInfo->height) / textureWidth;

		float xo = charInfo->xoffset * SCALING_CONSTANT;
		float yo = charInfo->yoffset * SCALING_CONSTANT;

		posy = yo;

    auto color = glm::vec3(1.0f, 0.0f, 0.0f);

		vertices.push_back({ { posx + dimx + xo, posy + dimy, 0.0f }, ue, color, te });
		vertices.push_back({ { posx + xo, posy + dimy, 0.0f }, us, color, te });
		vertices.push_back({ { posx + xo, posy, 0.0f }, us, color, ts });
		vertices.push_back({ { posx + dimx + xo, posy, 0.0f }, ue, color, ts });

		std::array<uint32_t, 6> letterIndices = { 0, 1, 2, 2, 3, 0 };
		for (auto& index : letterIndices) {
			indices.push_back(indexOffset + index);
		}
		indexOffset += 4;

		float advance = ((float)(charInfo->xadvance) * SCALING_CONSTANT);
		posx += advance;
	}
	indexCount = static_cast<uint32_t>(indices.size());

	/* NOTE: Not sure if we want this yet or not.
	// Center
	for (auto& v : vertices) {
		v.pos[0] -= posx / 2.0f;
		v.pos[1] -= 0.5f;
	}
	*/
}

}// namespace pm
