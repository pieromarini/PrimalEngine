#include "primitives.h"

namespace pm::UI {

UIElement box(float width, float height, glm::vec2 position, glm::vec2 scale, float rotationInRadians) {
	auto element = UIElement{
		.position = position,
		.scale = scale,
		.rotation = rotationInRadians,
		.width = width,
		.height = height
	};

	element.vertices.push_back({ { 1.0f, 1.0f, 0.0f }, 1.0f, { 1.0f, 0.0f, 0.0f }, 1.0f });
	element.vertices.push_back({ { -1.0f, 1.0f, 0.0f }, -1.0f, { 0.0f, 1.0f, 0.0f }, 1.0f });
	element.vertices.push_back({ { -1.0f, -1.0f, 0.0f }, -1.0f, { 1.0f, 0.0f, 1.0f }, -1.0f });
	element.vertices.push_back({ { 1.0f, -1.0f, 0.0f }, 1.0f, { 0.0f, 1.0f, 1.0f }, -1.0f });

	element.indices.push_back(0);
	element.indices.push_back(1);
	element.indices.push_back(2);
	element.indices.push_back(2);
	element.indices.push_back(3);
	element.indices.push_back(0);

	return element;
}

UIElement triangle(float width, float height, glm::vec2 position, glm::vec2 scale, float rotationInRadians) {
	auto element = UIElement{
		.position = position,
		.scale = scale,
		.rotation = rotationInRadians,
		.width = width,
		.height = height
	};

	element.vertices.push_back({ { 1.0f, 1.0f, 0.0f }, 1.0f, { 1.0f, 0.0f, 0.0f }, 1.0f });
	element.vertices.push_back({ { -1.0f, 1.0f, 0.0f }, -1.0f, { 0.0f, 1.0f, 0.0f }, 1.0f });
	element.vertices.push_back({ { -1.0f, -1.0f, 0.0f }, -1.0f, { 1.0f, 0.0f, 1.0f }, -1.0f });

	element.indices.push_back(0);
	element.indices.push_back(1);
	element.indices.push_back(2);

	return element;
}

UIElement text(std::string_view text, float width, float height, glm::vec2 position, glm::vec2 scale, float rotationInRadians, float textureWidth, std::array<bmchar, 255>& fontChars) {
	auto element = UIElement{
		.position = position,
		.scale = scale,
		.rotation = rotationInRadians,
		.width = width,
		.height = height
	};

	generateTextFromFont(text, textureWidth, fontChars, element.vertices, element.indices);

	return element;
}

}// namespace pm::UI
