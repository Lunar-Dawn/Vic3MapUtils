#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#include "SplineNetwork/Diff.hpp"
#include "SplineNetwork/SplineNetwork.hpp"
#include "util.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

using ArgList = std::vector<std::string_view>;

// TODO: Revisit this, either using an argument parsing framework or making each command a class
void printUsage(const ArgList &arguments) {
	std::cout
	    << "Usage: " << fs::path(arguments[0]).filename().string()
	    << " <command> [options] <args>\n"
	       "\n"
	       "A utility to transfer your spline network edits across Victoria 3 versions.\n"
	       "\n"
	       "General Options:\n"
	       "    -o <file>, --output <file>: Sets the output file name. Optional, default behaviour varies by command.\n"
	       "Commands:\n"
	       "    help\n"
	       "        Print this help information\n"
	       "    generate [-o <file>] originalNetworkFile editedNetworkFile\n"
	       "        Generates a network diff file from the original (usually vanilla's) "
	       "and the edited (usually your mod's) splnet files.\n"
	       "        The diff will be saved as diff.json unless output is specified.\n"
	       "    apply [-o <file>] networkFile diff\n"
	       "        Applies the network diff to the splnet file (usually vanilla's).\n"
	       "        The edited network will override the original unless output is specified.\n";
}

void handleGenerate(const ArgList &arguments) {
	if (arguments.size() < 4 || arguments.size() > 6) {
		printUsage(arguments);
		std::exit(1);
	}

	fs::path originalNetworkPath;
	fs::path editedNetworkPath;
	fs::path outputPath;

	auto it = arguments.begin() + 2;

	if (*it == "-o" || *it == "--output") {
		it++;
		outputPath = *it++;
	} else {
		outputPath = "diff.json";
	}

	if (it == arguments.end()) {
		printUsage(arguments);
		std::exit(1);
	}
	originalNetworkPath = *it++;

	if (it == arguments.end()) {
		printUsage(arguments);
		std::exit(1);
	}
	editedNetworkPath = *it++;

	if (!checkFilesExist(originalNetworkPath, editedNetworkPath)) {
		std::exit(1);
	}

	SplineNetwork originalNetwork(originalNetworkPath);
	SplineNetwork editedNetwork(editedNetworkPath);

	json jsonFile = originalNetwork.calculateDiff(editedNetwork);
	std::ofstream outputFile(outputPath);
	outputFile << jsonFile.dump(4) << '\n';
}
void handleApply(const ArgList &arguments) {
	if (arguments.size() < 4 || arguments.size() > 6) {
		printUsage(arguments);
		std::exit(1);
	}

	fs::path networkPath;
	fs::path diffPath;
	fs::path outputPath;

	auto it = arguments.begin() + 2;

	if (*it == "-o" || *it == "--output") {
		it++;
		outputPath = *it++;
	} else {
		outputPath = "diff.json";
	}

	if (it == arguments.end()) {
		printUsage(arguments);
		std::exit(1);
	}
	networkPath = *it++;

	if (it == arguments.end()) {
		printUsage(arguments);
		std::exit(1);
	}
	diffPath = *it++;

	if (!checkFilesExist(networkPath, diffPath)) {
		std::exit(1);
	};

	SplineNetwork network(networkPath);
	network.applyDiff(json::parse(std::ifstream(diffPath)).get<Diff>());
	network.writeToFile(outputPath);
}

int main(int argc, char *argv[]) {
	ArgList arguments(argv, argv + argc);

	if (arguments.size() == 1) {
		printUsage(arguments);
		return 1;
	}

	const auto &command = arguments[1];
	if (command == "help" || command == "-h" || command == "--help") {
		printUsage(arguments);
		return 0;
	}
	if (command == "generate") {
		handleGenerate(arguments);
		return 0;
	}
	if (command == "apply") {
		handleApply(arguments);
		return 0;
	}

	printUsage(arguments);
	return 1;
}
