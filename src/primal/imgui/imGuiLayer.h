#pragma once

#include "primal/core/layer.h"

#include "primal/events/applicationEvent.h"
#include "primal/events/keyEvent.h"
#include "primal/events/mouseEvent.h"

namespace primal {

  class ImGuiLayer : public Layer {
	public:
	  ImGuiLayer();
	  ~ImGuiLayer() override = default;

	  void onAttach() override;
	  void onDetach() override;

	  void begin();
	  void end();
	private:
	  float m_time = 0.0f;
  };

}
