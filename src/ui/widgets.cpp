#include "widgets.h"
#include "ui_manager.h"
#include <format>

namespace pm::UI {

void checkbox(bool* value) {
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

};// namespace pm::UI
