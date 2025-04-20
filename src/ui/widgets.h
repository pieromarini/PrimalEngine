#pragma once

#include <glm/glm.hpp>
#include <limits>
#include <string>

#include "viewport.h"

/*
 * Planned Widgets:
 * - Checkbox
 * - Button
 * - Int, Float
 * + Slider (Int/Float) (drag: change value, double-click: enter text manually)
 *   + This should support scalars and vectors (2, 3 and 4 component vectors)
 * - Color picker
 * - Text input
 * - Text area
 * - Tree view (entity hierachy)
 * - Asset browser (folders & files with thumbnails)
 *   - This is not a single widget. Needs to be broken up into components.
 */

namespace pm::UI {

void checkbox(bool* value);
void button();

void sliderFloat(float* value, float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());
void sliderFloat2(glm::vec2* value, float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());
void sliderFloat3(glm::vec3* value, float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());
void sliderFloat4(glm::vec4* value, float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());

void sliderInt(int* value, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max());
void sliderInt2(glm::ivec2* value, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max());
void sliderInt3(glm::ivec3* value, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max());
void sliderInt4(glm::ivec4* value, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max());

void textInput(std::string* value);

void viewport(Viewport viewport);

}// namespace pm::UI
