#pragma once

#include "FileHandler/SplnetFileReader.hpp"

#include <vector>

class Route {
	uint64_t _id;
	std::vector<uint32_t> _anchors;

public:
	explicit Route(SplnetFileReader &fileReader, bool isFinal = false);
};
