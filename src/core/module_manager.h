#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "core/memory.h"
#include "modules/graphics/graphics_manager.h"

namespace primal {

  class ModuleManager {
	public:
	  ModuleManager() {
		m_graphicsManager = GraphicsManager::create();
	  }

	private:
	  Unique<GraphicsManager> m_graphicsManager;
  };

}

#endif
