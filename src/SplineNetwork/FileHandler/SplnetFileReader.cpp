#include "SplnetFileReader.hpp"

#include <utility>

SplnetFileReader::SplnetFileReader(std::filesystem::path path)
    : _path(std::move(path)) {
	_file.exceptions(std::ios::badbit | std::ios::failbit);
	_file.open(_path, std::ios::in | std::ios::binary);
}

void SplnetFileReader::expectSectionHeader(uint16_t id) {
	expect(id);
	expect<uint16_t>(0x01);
	expect<uint16_t>(0x03);
	expect<uint16_t>(0x03);
}
void SplnetFileReader::expectElementHeader() {
	expect<uint16_t>(0x0b);
	expect<uint16_t>(0x01);
}
void SplnetFileReader::expectElementFooter(bool isFinal) {
	expect<uint16_t>(0x04);
	expect<uint16_t>(0x04);

	// The last element has a sentinel last value of 0x04 instead of 0x03
	if (isFinal)
		expect<uint16_t>(0x04);
	else
		expect<uint16_t>(0x03);
}
