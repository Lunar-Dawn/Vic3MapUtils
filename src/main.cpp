#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>

#include "SplineNetwork/Diff.hpp"
#include "SplineNetwork/SplineNetwork.hpp"
#include "util.hpp"
#include "version.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

void handleApply(const argparse::ArgumentParser &arguments) {
	const fs::path baseNetworkPath = arguments.get("BaseNetwork");
	const fs::path diffPath = arguments.get("DiffFile");
	const fs::path outputPath = arguments.is_used("-o") ? fs::path(arguments.get("-o")) : baseNetworkPath;

	if (!checkFilesExist(baseNetworkPath, diffPath)) {
		std::exit(1);
	}

	SplineNetwork network(baseNetworkPath);
	network.applyDiff(json::parse(std::ifstream(diffPath)).get<Diff>());
	network.writeToFile(outputPath);
}
void handleGenerate(const argparse::ArgumentParser &arguments) {
	const fs::path originalNetworkPath = arguments.get("BaseNetwork");
	const fs::path editedNetworkPath = arguments.get("EditedNetwork");
	const fs::path outputPath = arguments.get("-o");

	if (!checkFilesExist(originalNetworkPath, editedNetworkPath)) {
		std::exit(1);
	}

	SplineNetwork originalNetwork(originalNetworkPath);
	SplineNetwork editedNetwork(editedNetworkPath);

	json jsonFile = originalNetwork.calculateDiff(editedNetwork);
	std::ofstream outputFile(outputPath);
	outputFile << jsonFile.dump(4) << '\n';
}
void handleFullMerge(const argparse::ArgumentParser &arguments) {
	const auto networkFilesStr = arguments.get<std::vector<std::string>>("Networks");
	const std::vector<fs::path> networkPaths(networkFilesStr.begin(), networkFilesStr.end());
	const fs::path outputPath = arguments.get("-o");

	if (!checkFilesExist(networkPaths)) {
		std::exit(1);
	}

	SplineNetwork emptyNetwork;
	Diff mergedDiff;
	for (const auto &path : networkPaths) {
		SplineNetwork toMerge(path);
		mergedDiff.mergeDiff(emptyNetwork.calculateDiff(toMerge));
	}

	emptyNetwork.applyDiff(mergedDiff);
	emptyNetwork.writeToFile(outputPath);
}
void handleMerge(const argparse::ArgumentParser &arguments) {
	fs::path basePath = arguments.get("BaseNetwork");
	const auto networkFilesStr = arguments.get<std::vector<std::string>>("EditedNetworks");
	const std::vector<fs::path> networkPaths(networkFilesStr.begin(), networkFilesStr.end());
	fs::path outputPath = arguments.get("-o");

	if (!checkFileExists(basePath)) {
		std::exit(1);
	}
	if (!checkFilesExist(networkPaths)) {
		std::exit(1);
	}

	SplineNetwork baseNetwork(basePath);
	Diff mergedDiff;
	for (const auto &path : networkPaths) {
		SplineNetwork toMerge(path);
		mergedDiff.mergeDiff(baseNetwork.calculateDiff(toMerge));
	}

	baseNetwork.applyDiff(mergedDiff);
	baseNetwork.writeToFile(outputPath);
}

int main(int argc, char *argv[]) {
	const auto reindexEpilog = "This will reindex Sub-Anchors and Route IDs, "
	                           "but it will refuse to merge if any duplicate hub anchors exist, "
	                           "these will be printed for you to delete/merge manually.";

	const auto program_name = fs::path(argv[0]).filename().string();
	argparse::ArgumentParser program(program_name, globals::programVersion);
	program.add_description("A utility to manipulate the .splnet files that describe the road network in Victoria 3.");

	argparse::ArgumentParser mergeParser("merge");
	mergeParser
	    .add_description("Merge the changes made in the edited networks as compared to the base network."
	                     " Useful for git-style merge conflict resolution.")
	    .add_epilog(reindexEpilog);
	mergeParser.add_argument("-o", "--output")
	    .help("The output file name. Optional, defaults to 'merged.splnet'.")
	    .metavar("FILE")
	    .default_value("merged.splnet");
	mergeParser.add_argument("BaseNetwork").help("The base network everything is compared to.");
	mergeParser.add_argument("EditedNetworks")
	    .help("The edited networks.")
	    .remaining()
	    .nargs(1, std::numeric_limits<size_t>::max());

	argparse::ArgumentParser generateParser("generate");
	generateParser.add_description("Generates a network diff file from the original (usually vanilla's) "
	                               "and the edited (usually your mod's) splnet files.");
	generateParser.add_argument("-o", "--output")
	    .help("The output file name. Optional, defaults to 'diff.json'.")
	    .default_value("diff.json");
	generateParser.add_argument("BaseNetwork").help("The base network file (Usually vanilla's).");
	generateParser.add_argument("EditedNetwork").help("The edited network file (Usually your mod's).");

	argparse::ArgumentParser applyParser("apply");
	applyParser.add_description("Applies network changes to a base spline network.");
	applyParser.add_argument("-o", "--output")
	    .help("The output file name. Optional, defaults to overriding BaseNetwork.")
	    .metavar("FILE");
	applyParser.add_argument("BaseNetwork").help("The base spline network file (Usually vanilla's).");
	applyParser.add_argument("DiffFile").help("The network change diff file.");

	argparse::ArgumentParser fullMergeParser("full-merge");
	fullMergeParser
	    .add_description("Merge two or more networks into one, containing all the anchors, strips, and routes.")
	    .add_epilog(reindexEpilog);
	fullMergeParser.add_argument("-o", "--output")
	    .help("The output file name. Optional, defaults to 'merged.splnet'.")
	    .default_value("merged.splnet")
	    .metavar("FILE");
	fullMergeParser.add_argument("Networks")
	    .help("Network files to merge.")
	    .remaining()
	    .nargs(2, std::numeric_limits<size_t>::max());

	program.add_subparser(mergeParser);
	program.add_subparser(generateParser);
	program.add_subparser(applyParser);
	program.add_subparser(fullMergeParser);

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		return 1;
	}

	if (program.is_subcommand_used(mergeParser)) {
		handleMerge(mergeParser);
		return 0;
	}
	if (program.is_subcommand_used(generateParser)) {
		handleGenerate(generateParser);
		return 0;
	}
	if (program.is_subcommand_used(applyParser)) {
		handleApply(applyParser);
		return 0;
	}
	if (program.is_subcommand_used(fullMergeParser)) {
		handleFullMerge(fullMergeParser);
		return 0;
	}
}
