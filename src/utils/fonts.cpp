#include "fonts.h"

#include <algorithm>
#include <array>
#include <iostream>

namespace pm {

std::pair<float, float> generateTextGeometry(String8 text, float fontSize, FontAsset* font, std::vector<UIVertex>* vertices, std::vector<u32>* indices, vec2 offset) {
	auto& metadata = font->metadata;

	uint32_t vertexIndex = 0;

	float scale{ metadata.metrics.lineHeight * fontSize / metadata.metrics.emSize };

	float cursorX{ 0.0f };

	// TODO: why do we need this magic number? I'm guessing we are not aligning to the baseline correctly.
	float baseline{ metadata.metrics.ascender * scale * 0.80f };

	cursorX += offset.x;
	baseline += offset.y;

	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();

	float texelWidth = 1.0f / (float)metadata.atlas.width;
	float texelHeight = 1.0f / (float)metadata.atlas.height;

	float textWidth = 0.0f;

	for (size_t i = 0; i < text.size; i++) {
		int unicode = static_cast<u8>(text.str[i]);

		if (unicode == '\r') {
			continue;
		}
		if (unicode == '\n') {
			textWidth = std::max(textWidth, cursorX);
			cursorX = 0;
			baseline += scale * metadata.metrics.lineHeight;
			continue;
		}

		auto glyphIt = std::ranges::find_if(metadata.glyphs, [&unicode](const Glyph& g) { return g.unicode == unicode; });

		// If we can't find a glyph, we skip it
		if (glyphIt == metadata.glyphs.end()) {
			std::cout << "Unknown glyph with code: " << unicode << '\n';
			continue;
		}
		auto& glyph = *glyphIt;

		// Apply kerning if there's a next character (and we have kerning)
		if (i < text.size - 1) {
			int nextUnicode = static_cast<u8>(text.str[i + 1]);
			auto kerningIt = std::ranges::find_if(metadata.kerning, [&unicode, &nextUnicode](const KerningPair& k) { return k.unicode1 == unicode && k.unicode2 == nextUnicode; });
			if (kerningIt != metadata.kerning.end()) {
				cursorX += kerningIt->advance * scale;
			}
		}

		// Skip characters with no geometry (like spaces)
		if (glyph.atlasBounds.right == glyph.atlasBounds.left || glyph.atlasBounds.top == glyph.atlasBounds.bottom || glyph.planeBounds.right == glyph.planeBounds.left || glyph.planeBounds.top == glyph.planeBounds.bottom) {
			cursorX += glyph.advance * scale;
			continue;
		}

		// Calculate quad dimensions
		float glyphWidth = (glyph.planeBounds.right - glyph.planeBounds.left) * scale;
		float glyphHeight = (glyph.planeBounds.top - glyph.planeBounds.bottom) * scale;

		// UVs
		float u0 = glyph.atlasBounds.left * texelWidth;
		float v0 = 1.0f - (glyph.atlasBounds.top * texelHeight);
		float u1 = glyph.atlasBounds.right * texelWidth;
		float v1 = 1.0f - (glyph.atlasBounds.bottom * texelHeight);

		// Quad vertices
		float x0 = cursorX + glyph.planeBounds.left * scale;
		float y0 = baseline - glyph.planeBounds.top * scale;
		float x1 = cursorX + glyph.planeBounds.right * scale;
		float y1 = baseline - glyph.planeBounds.bottom * scale;
		
		minY = std::min({ minY, y0, y1 });
		maxY = std::max({ maxY, y0, y1 });

		if (vertices) {
			vertices->push_back({ .position = { x0, y0, 0.0f }, .uv_x = u0, .color = { 1.0f, 0.0f, 0.0f }, .uv_y = v0 });// Bottom-left
			vertices->push_back({ .position = { x1, y0, 0.0f }, .uv_x = u1, .color = { 1.0f, 0.0f, 0.0f }, .uv_y = v0 });// Bottom-right
			vertices->push_back({ .position = { x1, y1, 0.0f }, .uv_x = u1, .color = { 1.0f, 0.0f, 0.0f }, .uv_y = v1 });// Top-right
			vertices->push_back({ .position = { x0, y1, 0.0f }, .uv_x = u0, .color = { 1.0f, 0.0f, 0.0f }, .uv_y = v1 });// Top-right
		}

		if (indices) {
			std::array<uint32_t, 6> letterIndices = { 0, 1, 2, 2, 3, 0 };
			for (auto& index : letterIndices) {
				indices->push_back(vertexIndex + index);
			}
		}

		// Advance cursor and vertex index
		cursorX += glyph.advance * scale;
		vertexIndex += 4;
	}
	textWidth = std::max(textWidth, cursorX);

	float textHeight = maxY - minY;

	return { textWidth, textHeight };
}

}// namespace pm
