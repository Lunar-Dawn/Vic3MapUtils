#pragma once

#include <filesystem>
#include <fstream>

class SplnetFileWriter {
	std::ofstream _file;
	std::streamoff _writePos = 0;
	const std::filesystem::path _path;

public:
	explicit SplnetFileWriter(std::filesystem::path path);

	template <typename T> void write(T value) {
		_file.write(reinterpret_cast<char *>(&value), sizeof(T));
		_writePos += sizeof(T);
	}

	void writeSectionHeader(uint16_t id);
	void writeElementHeader();
	void writeElementFooter(bool isFinal = false);
};
