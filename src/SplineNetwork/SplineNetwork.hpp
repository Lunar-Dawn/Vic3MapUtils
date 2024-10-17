#pragma once

#include "Anchor.hpp"
#include "Route.hpp"
#include "Strip.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>

class SplineNetwork {
	uint32_t _anchorCount = 0;
	uint32_t _routeCount = 0;
	uint32_t _stripCount = 0;

	std::map<uint32_t, Anchor> _anchors;
	std::map<uint32_t, Route> _routes;
	std::map<std::pair<uint32_t, uint32_t>, Strip> _strips;

	void parseFileHeader(SplnetFileReader &fileReader);
	void parseAnchorList(SplnetFileReader &fileReader);
	void parseRouteList(SplnetFileReader &fileReader);
	void parseStripList(SplnetFileReader &fileReader);

public:
	explicit SplineNetwork(const std::filesystem::path &path);

	void writeToFile(const std::filesystem::path &path) const;
};
