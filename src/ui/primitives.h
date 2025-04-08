#pragma once

#include <string_view>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "ui_types.h"
#include "utils/fonts.h"

namespace pm::UI {

UIElement box(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices);
UIElement triangle(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices);
UIElement circle(float radius, uint32_t segments, float thickness, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices);
UIElement circleFilled(float radius, uint32_t segments, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices);

UIElement text(std::string_view text, float textureWidth, std::array<bmchar, 255>& fontChars, std::vector<UIVertex>* vertices, std::vector<uint32_t>* indices);

}// namespace pm::UI
