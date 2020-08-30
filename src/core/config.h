#ifndef CONFIG_H
#define CONFIG_H

#include "modules/graphics/graphics_api.h"

namespace primal {

  struct PrimalConfig {

	static constexpr RenderingAPI renderingAPI = RenderingAPI::OPENGL;

  };

}

#endif
