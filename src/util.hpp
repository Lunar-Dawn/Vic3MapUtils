#pragma once

#include <filesystem>
#include <ranges>

bool checkFileExists(const std::filesystem::path &path);
template <typename... Args>
    requires(... && std::is_constructible_v<Args, std::filesystem::path>)
bool checkFilesExist(const Args &...files) {
	bool allFound = true;
	for (const auto &f : {files...}) {
		if (checkFileExists(f))
			continue;

		allFound = false;
	}
	return allFound;
}
bool checkFilesExist(std::span<const std::filesystem::path> files);
