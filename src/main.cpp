#include <iostream>

#include "SplineNetwork/SplineNetwork.hpp"
#include "util.hpp"

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: Vic3MapUtils [vanilla spline_network.splnet] [your mod's spline_network.splnet]";
		return 1;
	}

	bool vanillaFileExists = checkFileExists(argv[1]);
	bool modFileExists = checkFileExists(argv[2]);
	if (!vanillaFileExists || !modFileExists) {
		return 1;
	}

	SplineNetwork vanillaNetwork(argv[1]);
	SplineNetwork modNetwork(argv[2]);

	vanillaNetwork.writeToFile("./out.splnet");
	modNetwork.writeToFile("./out2.splnet");

	return 0;
}
