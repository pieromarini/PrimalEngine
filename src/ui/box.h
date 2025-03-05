#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "ui_types.h"

namespace pm::UI {

UIElement box(float width, float height, glm::vec2 position, glm::vec2 scale, float rotationInRadians);
UIElement triangle(float width, float height, glm::vec2 position, glm::vec2 scale, float rotationInRadians);

}// namespace pm::UI
