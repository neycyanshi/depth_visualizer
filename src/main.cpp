#include <iostream>
#include "application.h"

int main(int argc, char *argv[])
{
	try {
		Application app;
		app.run(argc, argv);
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}