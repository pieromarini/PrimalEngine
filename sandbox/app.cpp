#include "../src/primal.h"
#include "primal/core/entryPoint.h"

#include "sandbox2D.h"
#include "sandbox3D.h"

class Sandbox : public primal::Application {
  public:
	Sandbox() {
	  pushLayer(new Sandbox3D());
	}

	~Sandbox() override = default;
};

primal::Application* primal::createApplication() {
  return new Sandbox();
}
