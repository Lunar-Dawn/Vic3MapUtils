#include "SplineNetwork.hpp"

#include <filesystem>
#include <iostream>
#include <set>

#include <fmt/format.h>

#include "FileHandler/SplnetFileReader.hpp"
#include "FileHandler/SplnetFileWriter.hpp"

SplineNetwork::SplineNetwork(const std::filesystem::path &path) {
	fmt::print("Reading \"{}\"\n", path.string());

	SplnetFileReader fileReader(path);

	auto [anchorCount, routeCount, stripCount] = parseFileHeader(fileReader);
	if (anchorCount)
		parseAnchorList(fileReader, anchorCount);
	if (routeCount)
		parseRouteList(fileReader, routeCount);
	if (stripCount)
		parseStripList(fileReader, stripCount);
}
std::tuple<uint32_t, uint32_t, uint32_t> SplineNetwork::parseFileHeader(SplnetFileReader &fileReader) {
	fileReader.expect<uint16_t>(0x00ee);
	fileReader.expect<uint16_t>(0x0001);
	fileReader.expect<uint16_t>(0x000c);
	fileReader.expect<uint16_t>(0x0004);
	fileReader.expect<uint16_t>(0x0000);
	fileReader.expect<uint16_t>(0x045a);
	fileReader.expect<uint16_t>(0x0001);
	fileReader.expect<uint16_t>(0x0003);
	fileReader.expect<uint16_t>(0x000c);
	auto anchorCount = fileReader.read<uint32_t>();
	fileReader.expect<uint16_t>(0x000c);
	auto routeCount = fileReader.read<uint32_t>();
	fileReader.expect<uint16_t>(0x000c);
	auto stripCount = fileReader.read<uint32_t>();
	fileReader.expect<uint16_t>(0x0004);

	return {anchorCount, routeCount, stripCount};
}
void SplineNetwork::parseAnchorList(SplnetFileReader &fileReader, uint32_t count) {
	fileReader.expectSectionHeader(0x05f4);

	for (uint32_t i = 0; i < count; ++i) {
		Anchor anchor(fileReader, i == count - 1);
		_anchors.emplace(anchor.id(), anchor);
	}
}
void SplineNetwork::parseRouteList(SplnetFileReader &fileReader, uint32_t count) {
	fileReader.expectSectionHeader(0x05f5);

	for (uint32_t i = 0; i < count; ++i) {
		Route route(fileReader, i == count - 1);
		_routes.emplace(route.id(), std::move(route));
	}
}
void SplineNetwork::parseStripList(SplnetFileReader &fileReader, uint32_t count) {
	fileReader.expectSectionHeader(0x05f6);

	for (uint32_t i = 0; i < count; ++i) {
		Strip strip(fileReader, i == count - 1);
		_strips.emplace(strip.idPair(), strip);
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
	fileWriter.write<uint32_t>(_anchors.size());
	fileWriter.write<uint16_t>(0x000c);
	fileWriter.write<uint32_t>(_routes.size());
	fileWriter.write<uint16_t>(0x000c);
	fileWriter.write<uint32_t>(_strips.size());
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

Diff SplineNetwork::calculateDiff(const SplineNetwork &other) {
	Diff diff;

	diff.anchorChanges.diffMaps(_anchors, other._anchors);
	diff.stripChanges.diffMaps(_strips, other._strips);
	diff.routeChanges.diffMaps(_routes, other._routes);

	return diff;
}

void SplineNetwork::applyDiff(Diff diff) {
	std::set<uint32_t> reservedAnchorIds;
	std::set<uint32_t> reservedRouteIds;
	for (const auto &id : _anchors | std::views::keys) {
		reservedAnchorIds.emplace(id);
	}
	for (const auto &id : _routes | std::views::keys) {
		reservedRouteIds.emplace(id);
	}
	diff.remapCollisions(reservedAnchorIds, reservedRouteIds);

	applyChangeList(_anchors, diff.anchorChanges);
	applyChangeList(_strips, diff.stripChanges);
	applyChangeList(_routes, diff.routeChanges);
}
