#include "SplineNetwork.hpp"

#include <filesystem>
#include <iostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

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

	for (uint32_t i = 0; i < _anchorCount; ++i) {
		Anchor anchor(fileReader, i == _anchorCount - 1);
		_anchors.emplace(anchor.id(), anchor);
	}
}
void SplineNetwork::parseRouteList(SplnetFileReader &fileReader) {
	fileReader.expectSectionHeader(0x05f5);

	for (uint32_t i = 0; i < _routeCount; ++i) {
		Route route(fileReader, i == _routeCount - 1);
		_routes.emplace(route.id(), std::move(route));
	}
}
void SplineNetwork::parseStripList(SplnetFileReader &fileReader) {
	fileReader.expectSectionHeader(0x05f6);

	for (uint32_t i = 0; i < _stripCount; ++i) {
		Strip strip(fileReader, i == _stripCount - 1);
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

Diff SplineNetwork::calculateDiff(const SplineNetwork &other) {
	Diff diff;

	diffMaps(_anchors, other._anchors, std::inserter(diff._anchorsDeleted, diff._anchorsDeleted.begin()),
	         std::inserter(diff._anchorsAdded, diff._anchorsAdded.begin()));

	diffMaps(_strips, other._strips, std::inserter(diff._stripsDeleted, diff._stripsDeleted.begin()),
	         std::inserter(diff._stripsAdded, diff._stripsAdded.begin()));

	diffMaps(_routes, other._routes, std::inserter(diff._routesDeleted, diff._routesDeleted.begin()),
	         std::inserter(diff._routesAdded, diff._routesAdded.begin()));

	return diff;
}
void SplineNetwork::applyDiff(const Diff &diff) {
	for (const auto &[id, deletedAnchor] : diff._anchorsDeleted) {
		auto anchorIt = _anchors.find(id);
		if (anchorIt == _anchors.end()) {
			fmt::print(std::cerr,
			           "Anchor #{} did not exist in network. Skipping deletion.\n"
			           "\tThis is likely due to the area previously edited being heavily altered, be very careful.\n",
			           id);
			continue;
		} else {
			if (deletedAnchor != anchorIt->second) {
				fmt::print(
				    std::cerr,
				    "Anchor #{} data does not match previous version.\n"
				    "\tThis is not necessarily a problem, some nodes may just have been nudged, but be careful.\n",
				    id);
			}
		}
		_anchors.erase(anchorIt);
		_anchorCount--;
	}
	_anchors.insert(diff._anchorsAdded.begin(), diff._anchorsAdded.end());
	_anchorCount -= diff._anchorsAdded.size();

	for (const auto &[id, deletedStrip] : diff._stripsDeleted) {
		auto stripIt = _strips.find(id);
		if (stripIt == _strips.end()) {
			fmt::print(std::cerr,
			           "Strip {}->{} did not exist in network. Skipping deletion.\n"
			           "\tThis is likely due to the area previously edited being heavily altered, be very careful.\n",
			           id.second, id.first);
			continue;
		} else {
			if (deletedStrip != stripIt->second) {
				fmt::print(
				    std::cerr,
				    "Strip {}->{} data does not match previous version.\n"
				    "\tThis is not necessarily a problem, some railroads may just have been added, but be careful.\n",
				    id.second, id.first);
			}
		}
		_strips.erase(stripIt);
		_stripCount--;
	}
	_strips.insert(diff._stripsAdded.begin(), diff._stripsAdded.end());
	_stripCount -= diff._stripsAdded.size();

	for (const auto &[id, deletedRoute] : diff._routesDeleted) {
		auto routeIt = _routes.find(id);
		if (routeIt == _routes.end()) {
			fmt::print(std::cerr,
			           "Route #{} did not exist in network. Skipping deletion.\n"
			           "\tThis is likely due to the area previously edited being heavily altered, be very careful.\n",
			           id);
			continue;
		} else {
			if (deletedRoute != routeIt->second) {
				fmt::print(
				    std::cerr,
				    "Route #{} data does not match previous version.\n"
				    "\tThis is not necessarily a problem, some roads may just have been nudged, but be careful.\n",
				    id);
			}
		}
		_routes.erase(routeIt);
		_routeCount--;
	}
	_routes.insert(diff._routesAdded.begin(), diff._routesAdded.end());
	_routeCount -= diff._routesAdded.size();
}
