#pragma once


#include "assets/asset.h"
#include "ui/ui_types.h"
#include <cassert>
#include <cstdint>
#include <string_view>
#include <vector>

namespace pm {

std::pair<float, float> generateTextFromFont(std::string_view text, float fontSize, FontAsset* font, std::vector<UI::UIVertex>* vertices = nullptr, std::vector<uint32_t>* indices = nullptr);

}// namespace pm
