#pragma once

#include "FileHandler/SplnetFileReader.hpp"
#include "FileHandler/SplnetFileWriter.hpp"

#include <vector>

class Strip {
	// Both of these do some strange left-shifting of their bits
	// _sourceID uses 6(?) bits to encode the type, but _destinationID
	// is just left-shifted 3 times for some unfathomable reason, those bits are always 0
	//
	// This is also why I think the anchor ID is 23/24 bits, otherwise this would overflow
	// OTOH since we don't have even close to 2^23 anchors it might just break when you get above that
	uint32_t _sourceID;
	uint32_t _destinationID;

	// A Strip can contain multiple Paths, for forks/merges in roads
	// Why Paradox coded this and then used it _once_, in Norway, only the stars may know
	// Made reverse engineering harder, that's for sure
	std::vector<uint64_t> _routeIDs;

public:
	explicit Strip(SplnetFileReader &fileReader, bool isFinal = false);

	void writeToFile(SplnetFileWriter &fileWriter, bool isFinal = false) const;
};
