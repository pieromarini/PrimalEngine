#include "../primal/primal.h"
#include "../primal/primal/core/entryPoint.h"

#include "sandbox2D.h"

class Sandbox : public primal::Application {
  public:
	Sandbox() {
	  pushLayer(new Sandbox2D());
	}

	~Sandbox() { }
};

primal::Application* primal::createApplication() {
  return new Sandbox();
}
