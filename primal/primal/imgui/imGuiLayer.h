#pragma once

#include "../layer.h"

#include "../events/applicationEvent.h"
#include "../events/keyEvent.h"
#include "../events/mouseEvent.h"

namespace primal {

  class PRIMAL_API ImGuiLayer : public Layer {
	public:
	  ImGuiLayer();
	  ~ImGuiLayer() = default;

	  virtual void onAttach() override;
	  virtual void onDetach() override;
	  virtual void onImGuiRender() override;

	  void begin();
	  void end();
	private:
	  float m_Time = 0.0f;
  };

}
