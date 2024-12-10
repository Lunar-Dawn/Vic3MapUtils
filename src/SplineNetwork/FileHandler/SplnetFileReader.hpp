#pragma once

#include <filesystem>
#include <fstream>

#include <fmt/format.h>

class SplnetFileReader {
	std::ifstream _file;
	std::streamoff _readPos = 0;
	const std::filesystem::path _path;

public:
	explicit SplnetFileReader(std::filesystem::path path);

	/// Read sizeof(T) bytes from the file, and interpret them as a bitwise representation of T
	/// Warning: Do not use with anything but primitives or maybe POD structs of primitives
	template <typename T> [[nodiscard]] T read() {
		T value;
		_file.read(reinterpret_cast<char *>(&value), sizeof(T));

		_readPos += sizeof(T);

		return value;
	}
	/// Call read() with type T and check if the value matches valueExpected, if not, throw
	template <typename T> void expect(T valueExpected) {
		T valueRead = read<T>();
		if (valueRead != valueExpected) {
			throw std::runtime_error(fmt::format("Invalid value ({:#x}) at position {:#x}, expected {:#x}", valueRead,
			                                     _readPos - sizeof(T), valueExpected));
		}
	}
	/// Call read() with type T and then back up again, ensuring the same value gets read if called again
	template <typename T> T peek() {
		const auto startPos = _file.tellg();
		T ret = read<T>();
		_readPos -= sizeof(T);
		_file.seekg(startPos);
		return ret;
	}

	void expectSectionHeader(uint16_t id);
	void expectElementHeader();
	void expectElementFooter(bool isFinal = false);
};
