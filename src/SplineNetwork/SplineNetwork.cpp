#include "SplineNetwork.hpp"

#include <filesystem>

#include <fmt/format.h>

#include "FileHandler/SplnetFileReader.hpp"
#include "FileHandler/SplnetFileWriter.hpp"

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

	for (int i = 0; i < _anchorCount; ++i) {
		Anchor anchor(fileReader, i == _anchorCount - 1);
		_anchors.emplace(anchor.id(), anchor);
	}
}
void SplineNetwork::parseRouteList(SplnetFileReader &fileReader) {
	fileReader.expectSectionHeader(0x05f5);

	for (int i = 0; i < _routeCount; ++i) {
		Route route(fileReader, i == _routeCount - 1);
		_routes.emplace(route.id(), std::move(route));
	}
}
void SplineNetwork::parseStripList(SplnetFileReader &fileReader) {
	fileReader.expectSectionHeader(0x05f6);

	for (int i = 0; i < _stripCount; ++i) {
		Strip strip(fileReader, i == _stripCount - 1);
		_strips.emplace(std::pair{strip.rawDestinationID(), strip.rawSourceID()}, strip);
	}
}

void SplineNetwork::writeToFile(const std::filesystem::path &path) const {
	SplnetFileWriter fileWriter(path);

	fileWriter.write<uint16_t>(0x00ee);
	fileWriter.write<uint16_t>(0x0001);
	fileWriter.write<uint16_t>(0x000c);
	fileWriter.write<uint16_t>(0x0004);
	fileWriter.write<uint16_t>(0x0000);
	fileWriter.write<uint16_t>(0x045a);
	fileWriter.write<uint16_t>(0x0001);
	fileWriter.write<uint16_t>(0x0003);
	fileWriter.write<uint16_t>(0x000c);
	fileWriter.write<uint32_t>(_anchorCount);
	fileWriter.write<uint16_t>(0x000c);
	fileWriter.write<uint32_t>(_routeCount);
	fileWriter.write<uint16_t>(0x000c);
	fileWriter.write<uint32_t>(_stripCount);
	fileWriter.write<uint16_t>(0x0004);

	fileWriter.writeSectionHeader(0x05f4);
	for (auto it = _anchors.cbegin(); it != _anchors.cend(); ++it) {
		it->second.writeToFile(fileWriter, it == std::prev(_anchors.cend()));
	};

	fileWriter.writeSectionHeader(0x05f5);
	for (auto it = _routes.cbegin(); it != _routes.cend(); ++it) {
		it->second.writeToFile(fileWriter, it == std::prev(_routes.cend()));
	};

	fileWriter.writeSectionHeader(0x05f6);
	for (auto it = _strips.cbegin(); it != _strips.cend(); ++it) {
		it->second.writeToFile(fileWriter, it == std::prev(_strips.cend()));
	};
}
