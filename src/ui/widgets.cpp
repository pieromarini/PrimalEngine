#include "widgets.h"
#include "ui_manager.h"
#include <format>

namespace pm::UI {

void checkbox(bool* value) {
	glm::vec4 bg = *value ? glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f } : glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
	openElement();
		pushBox({ .width = { .size = 40.0f, .sizingMode = UI::UISizingMode::STATIC },
			.height = { .size = 40.0f, .sizingMode = UI::UISizingMode::STATIC },
			.backgroundColor = bg,
			.padding = 5.0f,
			.border = { 4.0f, 4.0f },
			.data = {
				.dataType = UIDataType::BOOL,
				.valueBool = value } });

	closeElement();
}

void button() {
}

void sliderFloat(float* value, float min, float max) {
	auto textValue = std::format("{:.2f}", *value);
	openElement();
		pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.backgroundColor = { 0.43f, 0.43f, 0.43f, 1.0f },
			.padding = 5.0f });

		openTextElement();
			pushText({ .text = textValue,
					.data = {
						.dataType = UIDataType::FLOAT,
						.valueFloat = value,
						.minFloat = min,
						.maxFloat = max } });
		closeTextElement();
	closeElement();
}
void sliderFloat2(glm::vec2* value, float min, float max) {
	openElement();
		pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f },
			.padding = 8.0f,
			.childGap = 8.0f });

	sliderFloat(&value->x, min, max);
	sliderFloat(&value->y, min, max);

	closeElement();
}
void sliderFloat3(glm::vec3* value, float min, float max) {
	openElement();
		pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f },
			.padding = 8.0f,
			.childGap = 8.0f });

	sliderFloat(&value->x, min, max);
	sliderFloat(&value->y, min, max);
	sliderFloat(&value->z, min, max);

	closeElement();
}
void sliderFloat4(glm::vec4* value, float min, float max) {
	openElement();
		pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f },
			.padding = 8.0f,
			.childGap = 8.0f });

	sliderFloat(&value->x, min, max);
	sliderFloat(&value->y, min, max);
	sliderFloat(&value->z, min, max);
	sliderFloat(&value->w, min, max);

	closeElement();
}

void sliderInt(int* value, int min, int max) {
	auto textValue = std::format("{}", *value);
	openElement();
		pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.backgroundColor = { 0.43f, 0.43f, 0.43f, 1.0f },
			.padding = 5.0f });

		openTextElement();
			pushText({ .text = textValue,
					.data = {
						.dataType = UIDataType::INT,
						.valueInt = value,
						.minInt = min,
						.maxInt = max } });
		closeTextElement();
	closeElement();
}
void sliderInt2(glm::ivec2* value, int min, int max) {
	openElement();
		pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f },
			.padding = 8.0f,
			.childGap = 8.0f });

	sliderInt(&value->x, min, max);
	sliderInt(&value->y, min, max);

	closeElement();
}
void sliderInt3(glm::ivec3* value, int min, int max) {
	openElement();
		pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f },
			.padding = 8.0f,
			.childGap = 8.0f });

	sliderInt(&value->x, min, max);
	sliderInt(&value->y, min, max);
	sliderInt(&value->z, min, max);

	closeElement();
}
void sliderInt4(glm::ivec4* value, int min, int max) {
	openElement();
		pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f },
			.padding = 8.0f,
			.childGap = 8.0f });

	sliderInt(&value->x, min, max);
	sliderInt(&value->y, min, max);
	sliderInt(&value->z, min, max);
	sliderInt(&value->w, min, max);

	closeElement();
}

void textInput(std::string* value) {
}

void viewport(Viewport viewport) {
	auto width = viewport.width;
	auto height = viewport.height;

	openElement();
		pushBox({ .width = { .size = width, .sizingMode = UI::UISizingMode::STATIC },
			.height = { .size = height, .sizingMode = UI::UISizingMode::STATIC },
			.backgroundColor = { 0.478f, 0.478f, 0.478f, 0.0f },
			.textureId = viewport.textureId });

	closeViewportElement();
}

};// namespace pm::UI
