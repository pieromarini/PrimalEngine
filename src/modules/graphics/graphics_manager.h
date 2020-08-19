#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "graphics_api.h"
#include "core/config.h"

namespace primal {

  class GraphicsManager {
	public:
	  static Unique<GraphicsManager> create() {
		s_graphicsAPI = GraphicsAPI::create(PrimalConfig::renderingAPI);
		return createUnique<GraphicsManager>();
	  }

	private:
	  static Unique<GraphicsAPI> s_graphicsAPI;
  };

}

#endif
