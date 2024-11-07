#pragma once

#include <filesystem>
#include <ranges>

bool checkFileExists(const std::filesystem::path &path);
template <typename... Args> bool checkFilesExist(const Args &...files) {
	bool allFound = true;
	for (const auto &f : {files...}) {
		if (checkFileExists(f))
			continue;

		allFound = false;
	}
	return allFound;
}
