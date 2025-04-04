#include "primitives.h"

namespace pm::UI {

UIElement box(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices) {
	auto element = UIElement{};

	element.firstIndex = indices.size();
	element.indexCount = 6;
	element.vertexOffset = static_cast<int32_t>(vertices.size());

	vertices.push_back({ .position = { 1.0f, 1.0f, 0.0f }, .uv_x = 1.0f, .color = { 1.0f, 0.0f, 0.0f }, .uv_y = 1.0f });
	vertices.push_back({ .position = { -1.0f, 1.0f, 0.0f }, .uv_x = -1.0f, .color = { 0.0f, 1.0f, 0.0f }, .uv_y = 1.0f });
	vertices.push_back({ .position = { -1.0f, -1.0f, 0.0f }, .uv_x = -1.0f, .color = { 1.0f, 0.0f, 1.0f }, .uv_y = -1.0f });
	vertices.push_back({ .position = { 1.0f, -1.0f, 0.0f }, .uv_x = 1.0f, .color = { 0.0f, 1.0f, 1.0f }, .uv_y = -1.0f });

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	return element;
}

UIElement triangle(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices) {
	auto element = UIElement{};

	element.firstIndex = indices.size();
	element.indexCount = 3;
	element.vertexOffset = static_cast<int32_t>(vertices.size());

	vertices.push_back({ .position = { 1.0f, 1.0f, 0.0f }, .uv_x = 1.0f, .color = { 1.0f, 0.0f, 0.0f }, .uv_y = 1.0f });
	vertices.push_back({ .position = { -1.0f, 1.0f, 0.0f }, .uv_x = -1.0f, .color = { 0.0f, 1.0f, 0.0f }, .uv_y = 1.0f });
	vertices.push_back({ .position = { -1.0f, -1.0f, 0.0f }, .uv_x = -1.0f, .color = { 1.0f, 0.0f, 1.0f }, .uv_y = -1.0f });

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	return element;
}

UIElement text(std::string_view text, float textureWidth, std::array<bmchar, 255>& fontChars, std::vector<UIVertex>* vertices, std::vector<uint32_t>* indices) {
	auto element = UIElement{};

	element.firstIndex = indices->size();
	element.indexCount = text.size() * 6; // 6 indices per generated quad
	element.vertexOffset = static_cast<int32_t>(vertices->size());

	generateTextFromFont(text, textureWidth, fontChars, vertices, indices);

	return element;
}

}// namespace pm::UI
