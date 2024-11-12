#include <iostream>

#include <fmt/ostream.h>

#include "util.hpp"

namespace fs = std::filesystem;

bool checkFileExists(const fs::path &path) {
	if (exists(path))
		return true;

	fmt::print(std::cerr, "{}: File not found\n", path.string());
	return false;
}
bool checkFilesExist(std::span<std::filesystem::path> files) {
	bool allFound = true;

	for (const auto &path : files) {
		if (checkFileExists(path))
			continue;

		allFound = false;
	}

	return allFound;
}
