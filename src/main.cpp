#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#include "SplineNetwork/Diff.hpp"
#include "SplineNetwork/SplineNetwork.hpp"
#include "util.hpp"
#include "version.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

using ArgList = std::vector<std::string_view>;

// TODO: Revisit this, either using an argument parsing framework or making each command a class
void printUsage(const ArgList &arguments) {
	std::cout
	    << "Usage: " << fs::path(arguments[0]).filename().string()
	    << " <command> [options] <args>\n"
	       "\n"
	       "A utility to manipulate the .splnet files that describe the road network in Victoria 3.\n"
	       "\n"
	       "General Options:\n"
	       "    -o <file>, --output <file>: Sets the output file name. Optional, default behaviour varies by command.\n"
	       "Commands:\n"
	       "    help, -h, --help\n"
	       "        Print this help information\n"
	       "    version, --version\n"
	       "        Print version information\n"
	       "    generate [-o <file>] <originalNetworkFile> <editedNetworkFile>\n"
	       "        Generates a network diff file from the original (usually vanilla's) "
	       "and the edited (usually your mod's) splnet files.\n"
	       "        The diff will be saved as diff.json unless output is specified.\n"
	       "    apply [-o <file>] <networkFile> <diff>\n"
	       "        Applies the network diff to the splnet file (usually vanilla's).\n"
	       "        The edited network will override the original unless output is specified.\n"
	       "    merge [-o <file>] <baseNetworkFile> <editedNetwork>...\n"
	       "        Merge the changes made in the edited networks as compared to the base network. "
	       "Useful for git-style merge conflict resolution.\n"
	       "        This will reindex Sub-Anchors and Route IDs, "
	       "but it will refuse to merge if any duplicate hub anchors exist, "
	       "these will be printed for you to delete/merge manually.\n"
	       "        The merged network will be saved as merged.splnet unless output is specified.\n"
	       "    full-merge [-o <file>] <networkFile> <networkFile>...\n"
	       "        Merge two or more networks into one, containing all the anchors, strips, and routes.\n"
	       "        This will reindex Sub-Anchors and Route IDs, "
	       "but it will refuse to merge if any duplicate hub anchors exist, "
	       "these will be printed for you to delete/merge manually.\n"
	       "        The merged network will be saved as merged.splnet unless output is specified.\n";
}
void printVersion() {
	std::cout << globals::programVersion << '\n';
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
void handleFullMerge(const ArgList &arguments) {
	if (arguments.size() < 4) {
		printUsage(arguments);
		std::exit(1);
	}

	std::vector<fs::path> networkFiles;
	fs::path outputPath;

	auto it = arguments.begin() + 2;

	if (*it == "-o" || *it == "--output") {
		it++;
		outputPath = *it++;
		networkFiles.reserve(arguments.size() - 4);
	} else {
		outputPath = "merged.splnet";
		networkFiles.reserve(arguments.size() - 2);
	}

	std::copy(it, arguments.end(), std::back_inserter(networkFiles));

	if (networkFiles.size() < 2) {
		std::cerr << "Please provide at least 2 network files to merge.\n";
		printUsage(arguments);
		std::exit(1);
	}

	if (!checkFilesExist(networkFiles)) {
		std::exit(1);
	}

	SplineNetwork emptyNetwork;
	Diff mergedDiff;
	for (const auto &path : networkFiles) {
		SplineNetwork toMerge(path);
		mergedDiff.mergeDiff(emptyNetwork.calculateDiff(toMerge));
	}

	emptyNetwork.applyDiff(mergedDiff);
	emptyNetwork.writeToFile(outputPath);
}
void handleMerge(const ArgList &arguments) {
	if (arguments.size() < 5) {
		printUsage(arguments);
		std::exit(1);
	}

	fs::path basePath;
	std::vector<fs::path> networkFiles;
	fs::path outputPath;

	auto it = arguments.begin() + 2;

	if (*it == "-o" || *it == "--output") {
		it++;
		outputPath = *it++;
		networkFiles.reserve(arguments.size() - 4);
	} else {
		outputPath = "merged.splnet";
		networkFiles.reserve(arguments.size() - 2);
	}

	basePath = *it++;
	std::copy(it, arguments.end(), std::back_inserter(networkFiles));

	if (networkFiles.size() < 2) {
		std::cerr << "Please provide at least 2 network files to merge.\n";
		printUsage(arguments);
		std::exit(1);
	}

	if (!checkFileExists(basePath)) {
		std::exit(1);
	}
	if (!checkFilesExist(networkFiles)) {
		std::exit(1);
	}

	SplineNetwork baseNetwork(basePath);
	Diff mergedDiff;
	for (const auto &path : networkFiles) {
		SplineNetwork toMerge(path);
		mergedDiff.mergeDiff(baseNetwork.calculateDiff(toMerge));
	}

	baseNetwork.applyDiff(mergedDiff);
	baseNetwork.writeToFile(outputPath);
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
	if (command == "version" || command == "--version") {
		printVersion();
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
	if (command == "full-merge") {
		handleFullMerge(arguments);
		return 0;
	}
	if (command == "merge") {
		handleMerge(arguments);
		return 0;
	}

	printUsage(arguments);
	return 1;
}
