#pragma once


#include "platform/vulkan/vulkan_renderer.h"
#include "assets/asset.h"
#include "ui/ui_types.h"
#include <cassert>
#include <cstdint>
#include <vector>


namespace pm {

vec2 generateTextGeometry(String8 text, float fontSize, FontAsset* font, Arena* arena = nullptr, UIVertexArray* vertices = nullptr, UIIndexArray* indices = nullptr, vec2 offset = { 0.0f, 0.0f });

}// namespace pm
