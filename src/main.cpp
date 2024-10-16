#include <iostream>

#include "SplineNetwork/SplineNetwork.hpp"

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: Vic3MapUtils [vanilla spline_network.splnet] [your mod's spline_network.splnet]";
		return 1;
	}

	SplineNetwork vanillaNetwork(argv[1]);
	SplineNetwork modNetwork(argv[2]);

	return 0;
}
