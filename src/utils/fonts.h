#pragma once


#include "assets/asset.h"
#include "ui/ui_types.h"
#include <cassert>
#include <cstdint>
#include <vector>


namespace pm {

std::pair<float, float> generateTextGeometry(PrimalString& text, float fontSize, FontAsset* font, std::vector<UI::UIVertex>* vertices = nullptr, std::vector<uint32_t>* indices = nullptr);

}// namespace pm
