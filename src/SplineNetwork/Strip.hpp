#pragma once

#include "FileHandler/SplnetFileReader.hpp"
#include "FileHandler/SplnetFileWriter.hpp"

#include <vector>

#include <nlohmann/json.hpp>

class Strip {
public:
	enum class Type : uint8_t {
		DIRT_ROAD = 0x00,
		RAILROAD = 0x01,
		SEA_CONNECTION = 0x02,
		PORT_CONNECTION = 0x03,
	};

private:
	// Both of these do some strange left-shifting of their bits
	// _sourceID uses 6(?) bits to encode the type, but _destinationID
	// is just left-shifted 3 times for some unfathomable reason, those bits are always 0
	//
	// This is also why I think the anchor ID is 23/24 bits, otherwise this would overflow
	// OTOH since we don't have even close to 2^23 anchors it might just break when you get above that
	uint32_t _sourceID = -1;
	uint32_t _destinationID = -1;

	// A Strip can contain multiple Routes, for forks/merges in roads
	// Why Paradox coded this and then used it _once_, in Norway, only the stars may know
	// Made reverse engineering harder, that's for sure
	std::vector<uint64_t> _routeIDs;

public:
	Strip() = default;
	explicit Strip(SplnetFileReader &fileReader, bool isFinal = false);

	void writeToFile(SplnetFileWriter &fileWriter, bool isFinal = false) const;

	[[nodiscard]] auto type() const { return (Type)(_sourceID & ((1 << 6) - 1)); }

	[[nodiscard]] auto sourceID() const { return _sourceID >> 6; }
	void sourceID(uint32_t set) { _sourceID = (uint32_t)type() | (set << 6); }
	[[nodiscard]] auto destinationID() const { return _destinationID >> 3; }
	void destinationID(uint32_t set) { _destinationID = set << 3; }

	[[nodiscard]] auto rawSourceID() const { return _sourceID; }
	[[nodiscard]] auto rawDestinationID() const { return _destinationID; }

	/// Id pair, used as std::map id, since it maps cleanly onto the sorting order used in the files
	[[nodiscard]] std::pair<uint32_t, uint32_t> idPair() const { return {rawDestinationID(), rawSourceID()}; }

	void remapRoutes(const std::map<uint32_t, uint32_t> &map);

	bool operator==(const Strip &other) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Strip, _sourceID, _destinationID, _routeIDs);
};

template <> struct fmt::formatter<Strip> : formatter<string_view> {
	static format_context::iterator format(const Strip &a, format_context &ctx) {
		return format_to(ctx.out(), "Strip {}->{}", a.sourceID(), a.destinationID());
	}
};
