#pragma once

#include <string_view>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "ui_types.h"
#include "utils/fonts.h"

namespace pm::UI {

UIElement box(float width, float height, glm::vec2 position, glm::vec2 scale, float rotationInRadians);
UIElement triangle(float width, float height, glm::vec2 position, glm::vec2 scale, float rotationInRadians);
UIElement text(std::string_view text, float width, float height, glm::vec2 position, glm::vec2 scale, float rotationInRadians, float textureWidth, std::array<bmchar, 255>& fontChars);


}// namespace pm::UI
