#include <filesystem>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "SplineNetwork/SplineNetwork.hpp"
#include "util.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

void printUsage(std::string_view fileName) {
	std::cout << "Usage: " << fs::path(fileName).filename().string()
	          << " <generate | apply> [args]\n"
	             "\n"
	             "A utility to transfer your spline network edits across Victoria 3 versions.\n"
	             "\n"
	             "Commands:\n"
	             "    generate originalNetworkFile editedNetworkFile [output]\n"
	             "        Generates a network diff file from the original (usually vanilla's) "
	             "and the edited (usually your mod's) splnet files.\n"
	             "        The diff will be saved as diff.json unless output is specified.\n"

	             "    apply networkFile diff [output]\n"
	             "        Applies the network diff to the splnet file (usually vanilla's).\n"
	             "        The edited network will override the original unless output is specified.\n";
}

void handleGenerate(int argc, char *argv[]) {
	if (argc < 4 || argc > 6) {
		printUsage(argv[0]);
		std::exit(1);
	}

	fs::path originalNetworkFile(argv[2]);
	fs::path editedNetworkFile(argv[3]);

	if (!checkFilesExist(originalNetworkFile, editedNetworkFile)) {
		std::exit(1);
	};

	SplineNetwork originalNetwork(originalNetworkFile);
	SplineNetwork editedNetwork(editedNetworkFile);

	json jsonFile = originalNetwork.calculateDiff(editedNetwork);
	std::ofstream outputFile(argc == 5 ? argv[4] : "diff.json");
	outputFile << jsonFile.dump(4) << '\n';
}
void handleApply(int argc, char *argv[]) {
	if (argc < 4 || argc > 6) {
		printUsage(argv[0]);
		std::exit(1);
	}

	fs::path networkFile(argv[2]);
	fs::path diffFile(argv[3]);

	if (!checkFilesExist(networkFile, diffFile)) {
		std::exit(1);
	};

	SplineNetwork network(networkFile);
	network.applyDiff(json::parse(std::ifstream(diffFile)).get<SplineNetwork::Diff>());

	network.writeToFile(argc == 5 ? argv[4] : argv[2]);
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		printUsage(argv[0]);
		return 1;
	}
	std::string command(argv[1]);

	if (command == "help" || command == "-h" || command == "--help") {
		printUsage(argv[0]);
		return 0;
	}
	if (command == "generate") {
		handleGenerate(argc, argv);
		return 0;
	}
	if (command == "apply") {
		handleApply(argc, argv);
		return 0;
	}

	printUsage(argv[0]);
	return 1;
}
