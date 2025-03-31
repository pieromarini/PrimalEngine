#pragma once

#include <string_view>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "ui_types.h"
#include "utils/fonts.h"

namespace pm::UI {

UIElement box(UIElementOptions& options, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices);
UIElement triangle(UIElementOptions& options, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices);

UITextElement text(std::string_view text, float textureWidth, std::array<bmchar, 255>& fontChars, UIElementOptions& options, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices);

}// namespace pm::UI
