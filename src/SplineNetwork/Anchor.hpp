#pragma once

#include "FileHandler/SplnetFileReader.hpp"
#include "FileHandler/SplnetFileWriter.hpp"

#include <nlohmann/json.hpp>

class Anchor {
	// How to interpret this field is unclear, either it's a 32-bit ID with special meaning to bits 23 and 28,
	// an 8-bit flags field with only bit 4 used right now followed by a 24-bit ID with special meaning on bit 23
	// or a bitfield starting with maybe some padding, maybe a 6-bit type, and then a 23-bit ID.
	uint32_t _id;

	// These, very nicely, map directly to pixes in provinces.png with (0,0) at the bottom left.
	float _posX;
	float _posY;

public:
	Anchor() = default;
	explicit Anchor(SplnetFileReader &fileReader, bool isFinal = false);

	void writeToFile(SplnetFileWriter &fileWriter, bool isFinal = false) const;

	[[nodiscard]] auto id() const { return _id; }
	[[nodiscard]] bool isSubAnchor() const { return _id & (1 << 28); }
	[[nodiscard]] bool isWaterAnchor() const { return _id & (1 << 23); }
	/// The id without the signaling bits, as entered in the editor
	[[nodiscard]] auto niceID() const { return _id & ((1 << 23) - 1); }

	bool operator==(const Anchor &other) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Anchor, _id, _posX, _posY);
};
