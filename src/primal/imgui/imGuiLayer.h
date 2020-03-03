#pragma once

#include "../core/layer.h"

#include "../events/applicationEvent.h"
#include "../events/keyEvent.h"
#include "../events/mouseEvent.h"

namespace primal {

  class ImGuiLayer : public Layer {
	public:
	  ImGuiLayer();
	  ~ImGuiLayer() = default;

	  virtual void onAttach() override;
	  virtual void onDetach() override;

	  void begin();
	  void end();
	private:
	  float m_time = 0.0f;
  };

}