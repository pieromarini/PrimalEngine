#include "primal.h"

int main() {
	auto app = primal::Application();

	// DEBUG:
	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
