#include <iostream>

#include <fmt/format.h>

#include "util.hpp"

bool checkFileExists(const std::filesystem::path &path) {
	if (std::filesystem::exists(path))
		return true;

	std::cerr << fmt::format("File \"{}\" not found, aborting\n", path.string());
	return false;
}
