#include "primal.h"

int main() {
	pm::PrimalApp app;

	app.init();
	app.run();
	app.cleanup();

	return 0;
}
