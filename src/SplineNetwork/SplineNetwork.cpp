#include "SplineNetwork.hpp"

#include <filesystem>

#include <fmt/format.h>

SplineNetwork::SplineNetwork(const std::filesystem::path &inPath) {
	fmt::print("Reading \"{}\"\n", inPath.string());
}
