#include "../primal/primal.h"
#include "../primal/primal/core/entryPoint.h"

#include "sandbox2D.h"
#include "sandbox3D.h"

class Sandbox : public primal::Application {
  public:
	Sandbox() {
	  // pushLayer(new Sandbox2D());
	  pushLayer(new Sandbox3D());
	}

	~Sandbox() { }
};

primal::Application* primal::createApplication() {
  return new Sandbox();
}
