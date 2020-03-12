#pragma once

#include "core.h"
#include "timestep.h"
#include "../events/event.h"

namespace primal { 

  class Layer {
	public:
	  Layer(const std::string& name = "Layer");
	  virtual ~Layer() = default;

	  virtual void onAttach() {}
	  virtual void onDetach() {}
	  virtual void onUpdate(Timestep ts) {}
	  virtual void onImGuiRender() {}
	  virtual void onEvent(Event& event) {}

	  inline const std::string& getName() const { return m_debugName; }
	protected:
	  std::string m_debugName;
  };

}
