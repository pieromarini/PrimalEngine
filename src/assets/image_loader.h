#pragma once

#include "asset.h"
#include <string>

namespace pm {

ImageAsset loadPNG(std::string name, std::string filename);
ImageAsset loadPNG(std::string name, unsigned char* data, uint32_t size);
void destroyImageAsset(ImageAsset& asset);

}// namespace pm
