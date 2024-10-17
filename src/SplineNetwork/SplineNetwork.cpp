#include "SplineNetwork.hpp"

#include <filesystem>

#include <fmt/format.h>

#include "FileHandler/SplnetFileReader.hpp"

SplineNetwork::SplineNetwork(const std::filesystem::path &path) {
	fmt::print("Reading \"{}\"\n", path.string());

	SplnetFileReader fileReader(path);

	parseFileHeader(fileReader);
	parseAnchorList(fileReader);
	parseRouteList(fileReader);
	parseStripList(fileReader);
}
void SplineNetwork::parseFileHeader(SplnetFileReader &fileReader) {
	fileReader.expect<uint16_t>(0x00ee);
	fileReader.expect<uint16_t>(0x0001);
	fileReader.expect<uint16_t>(0x000c);
	fileReader.expect<uint16_t>(0x0004);
	fileReader.expect<uint16_t>(0x0000);
	fileReader.expect<uint16_t>(0x045a);
	fileReader.expect<uint16_t>(0x0001);
	fileReader.expect<uint16_t>(0x0003);
	fileReader.expect<uint16_t>(0x000c);
	_anchorCount = fileReader.read<uint32_t>();
	fileReader.expect<uint16_t>(0x000c);
	_routeCount = fileReader.read<uint32_t>();
	fileReader.expect<uint16_t>(0x000c);
	_stripCount = fileReader.read<uint32_t>();
	fileReader.expect<uint16_t>(0x0004);
}
void SplineNetwork::parseAnchorList(SplnetFileReader &fileReader) {
	fileReader.expectSectionHeader(0x05f4);

	_anchors.reserve(_anchorCount);

	for (int i = 0; i < _anchorCount; ++i) {
		_anchors.emplace_back(fileReader, i == _anchorCount - 1);
	}
}
void SplineNetwork::parseRouteList(SplnetFileReader &fileReader) {
	fileReader.expectSectionHeader(0x05f5);

	_routes.reserve(_routeCount);

	for (int i = 0; i < _routeCount; ++i) {
		_routes.emplace_back(fileReader, i == _routeCount - 1);
	}
}
void SplineNetwork::parseStripList(SplnetFileReader &fileReader) {
	fileReader.expectSectionHeader(0x05f6);

	_strips.reserve(_stripCount);

	for (int i = 0; i < _stripCount; ++i) {
		_strips.emplace_back(fileReader, i == _stripCount - 1);
	}
}
