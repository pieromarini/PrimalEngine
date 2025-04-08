#pragma once

#include "ui/ui_types.h"
#include "utils/colors.h"
#include <cmath>
#include <numbers>
#include <vector>

namespace pm {

/**
 * Represents the geometry data for a circle
 */
struct CircleGeometryInfo {
	uint32_t indexCount{};
};

inline CircleGeometryInfo generateCircleGeometry(float radius, uint32_t segments, std::vector<UI::UIVertex>& vertices, std::vector<uint32_t>& indices, UI::CircleType type = UI::CircleType::FILLED, float thickness = 0.1f) {

	assert(segments > 3);
	assert((type == UI::CircleType::OUTLINE && thickness > 0) || type == UI::CircleType::FILLED);
	assert((type == UI::CircleType::OUTLINE && thickness < radius) || type == UI::CircleType::FILLED);

	CircleGeometryInfo info{};

	// NOTE(piero): Right now we always draw circles at (0,0,0).
	// 							Leaving the functionality here in case we need it in the future.
	float centerX = 0.0f;
	float centerY = 0.0f;
	float centerZ = 0.0f;

	if (type == UI::CircleType::FILLED) {
		vertices.push_back({ .position = { centerX, centerY, centerZ } });

		for (int i = 0; i < segments; i++) {
			float theta = (i / static_cast<float>(segments)) * std::numbers::pi_v<float> * 2;
			float x = centerX + radius * std::cos(theta);
			float y = centerY + radius * std::sin(theta);

			/*
			// Convert angle to HSV (angle -> hue, constant saturation and value)
			float hue = theta * 180.0f / std::numbers::pi_v<float>; // Hue in [0, 360]
			float saturation = 1.0f;
			float value = 1.0f;

			// Convert HSV to RGB
			glm::vec3 color = hsvToRgb(hue, saturation, value);
			*/

			glm::vec3 color = { 1.0f, 0.0f, 0.0f };

			vertices.push_back({ .position = { x, y, centerZ }, .uv_x = x, .color = color, .uv_y = y });
		}

		// Generate indices for triangles (center to edge)
		for (int i = 0; i < segments; i++) {
			unsigned int current = i + 1;
			unsigned int next = (i + 1) % segments + 1;

			// Create a triangle: center -> current -> next
			indices.push_back(0);
			indices.push_back(current);
			indices.push_back(next);
		}
		info.indexCount = segments * 3;
	} else {
		float innerRadius = radius - thickness;

		// Generate vertices for both inner and outer circles
		for (int i = 0; i < segments; i++) {
			float theta = (i / static_cast<float>(segments)) * std::numbers::pi_v<float> * 2;
			float cosTheta = std::cos(theta);
			float sinTheta = std::sin(theta);

			// Outer vertex
			float outerX = centerX + radius * cosTheta;
			float outerY = centerY + radius * sinTheta;
			vertices.push_back({ .position = { outerX, outerY, centerZ }, .uv_x = outerX, .color = { 1.0f, 0.0f, 0.0f }, .uv_y = outerY });

			// Inner vertex
			float innerX = centerX + innerRadius * cosTheta;
			float innerY = centerY + innerRadius * sinTheta;
			vertices.push_back({ .position = { innerX, innerY, centerZ }, .uv_x = innerX, .color = { 0.0f, 1.0f, 0.0f }, .uv_y = innerY });
		}

		// Generate indices for the triangles forming the outline
		for (int segmentNum = 0; segmentNum < segments; segmentNum++) {
			unsigned int o1 = segmentNum * 2;
			unsigned int i1 = segmentNum * 2 + 1;
			unsigned int o2 = ((segmentNum + 1) % segments) * 2;
			unsigned int i2 = ((segmentNum + 1) % segments) * 2 + 1;

			indices.push_back(o1);
			indices.push_back(i1);
			indices.push_back(i2);

			indices.push_back(o1);
			indices.push_back(i2);
			indices.push_back(o2);
		}
		info.indexCount = segments * 6;
	}

	return info;
}

}// namespace pm
