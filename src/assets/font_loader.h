#pragma once

#include <string>

#include "asset.h"

namespace pm {

MSDFFont loadFontMetadata(std::string_view metadataPath);

FontAsset loadFontSDF(std::string assetName, std::string texturePath, std::string metadataPath);
void destroyFontSDF(FontAsset& asset);

}// namespace pm
