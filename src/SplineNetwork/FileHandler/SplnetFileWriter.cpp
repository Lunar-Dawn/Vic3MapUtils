#include "SplnetFileWriter.hpp"

SplnetFileWriter::SplnetFileWriter(std::filesystem::path path)
    : _path(std::move(path)) {
	_file.exceptions(std::ios::badbit | std::ios::failbit);
	_file.open(_path, std::ios::out | std::ios::binary | std::ios::trunc);
}

void SplnetFileWriter::writeSectionHeader(uint16_t id) {
	write(id);
	write<uint16_t>(0x01);
	write<uint16_t>(0x03);
	write<uint16_t>(0x03);
}
void SplnetFileWriter::writeElementHeader() {
	write<uint16_t>(0x0b);
	write<uint16_t>(0x01);
}
void SplnetFileWriter::writeElementFooter(bool isFinal) {
	write<uint16_t>(0x04);
	write<uint16_t>(0x04);

	// The last element has a sentinel last value of 0x04 instead of 0x03
	if (isFinal)
		write<uint16_t>(0x04);
	else
		write<uint16_t>(0x03);
}
