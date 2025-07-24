#pragma once

#include "assets/asset.h"
#include "core/core.h"

namespace pm {

using FontHash = u64;

MSDFFont FontCache_metricsFromFontSize(FontHash fontId, f32 fontSize);

};
