#include "primitives.h"
#include "utils/fonts.h"
#include "utils/geometry.h"

namespace pm::UI {

UIElement box(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices) {
	auto element = UIElement{};

	element.firstIndex = indices.size();
	element.indexCount = 6;
	element.vertexOffset = static_cast<int32_t>(vertices.size());

	vertices.push_back({ .position = { 1.0f, 1.0f, 0.0f }, .uv_x = 1.0f, .color = { 1.0f, 0.0f, 0.0f }, .uv_y = 1.0f });
	vertices.push_back({ .position = { -1.0f, 1.0f, 0.0f }, .uv_x = 0.0f, .color = { 0.0f, 1.0f, 0.0f }, .uv_y = 1.0f });
	vertices.push_back({ .position = { -1.0f, -1.0f, 0.0f }, .uv_x = 0.0f, .color = { 1.0f, 0.0f, 1.0f }, .uv_y = 0.0f });
	vertices.push_back({ .position = { 1.0f, -1.0f, 0.0f }, .uv_x = 1.0f, .color = { 0.0f, 1.0f, 1.0f }, .uv_y = 0.0f });

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
	vertices.push_back({ .position = { -1.0f, 1.0f, 0.0f }, .uv_x = 0.0f, .color = { 0.0f, 1.0f, 0.0f }, .uv_y = 1.0f });
	vertices.push_back({ .position = { -1.0f, -1.0f, 0.0f }, .uv_x = 0.0f, .color = { 1.0f, 0.0f, 1.0f }, .uv_y = 0.0f });

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	return element;
}

UIElement circle(float radius, uint32_t segments, float thickness, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices) {
	auto element = UIElement{};

	element.firstIndex = indices.size();
	element.vertexOffset = static_cast<int32_t>(vertices.size());

	auto info = generateCircleGeometry(radius, segments, vertices, indices, CircleType::OUTLINE, thickness);

	element.indexCount = info.indexCount;

	return element;
}

UIElement circleFilled(float radius, uint32_t segments, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices) {
	auto element = UIElement{};

	element.firstIndex = indices.size();
	element.vertexOffset = static_cast<int32_t>(vertices.size());

	auto info = generateCircleGeometry(radius, segments, vertices, indices, CircleType::FILLED);

	element.indexCount = info.indexCount;

	return element;
}

UIElement text(PrimalString& text, float fontSize, FontAsset* font, std::vector<UIVertex>* vertices, std::vector<uint32_t>* indices) {
	auto element = UIElement{};

	element.firstIndex = indices->size();
	element.indexCount = text.length * 6; // 6 indices per generated quad
	element.vertexOffset = static_cast<int32_t>(vertices->size());

	generateTextFromFont(text, fontSize, font, vertices, indices);

	return element;
}

}// namespace pm::UI
