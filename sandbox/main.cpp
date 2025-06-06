#include "primal_engine.h"

int main() {
	pm::PrimalEngine app;

	app.run();
	app.cleanup();

	return 0;
}
