#pragma once

#include "FileHandler/SplnetFileReader.hpp"

class Anchor {
	// How to interpret this field is unclear, either it's a 32-bit ID with special meaning to bits 23 and 28,
	// an 8-bit flags field with only bit 4 used right now followed by a 24-bit ID with special meaning on bit 23
	// or a bitfield starting with maybe some padding, maybe a 6-bit type, and then a 23-bit ID.
	uint32_t _id;

	// These, very nicely, map directly to pixes in provinces.png with (0,0) at the bottom left.
	float _posX;
	float _posY;

public:
	explicit Anchor(SplnetFileReader &fileReader, bool isFinal = false);
};
