#pragma once

#include "FileHandler/SplnetFileReader.hpp"
#include "FileHandler/SplnetFileWriter.hpp"

#include <vector>

class Route {
	uint64_t _id;
	std::vector<uint32_t> _anchors;

public:
	explicit Route(SplnetFileReader &fileReader, bool isFinal = false);

	void writeToFile(SplnetFileWriter &fileWriter, bool isFinal = false) const;
};
