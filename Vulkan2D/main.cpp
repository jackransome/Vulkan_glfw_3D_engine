#include "Graphics.h"

int main() {
	Graphics gfx;

	try {
		gfx.run();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}