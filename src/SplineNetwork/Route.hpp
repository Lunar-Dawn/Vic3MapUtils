#pragma once

#include "FileHandler/SplnetFileReader.hpp"
#include "FileHandler/SplnetFileWriter.hpp"

#include <vector>

#include <nlohmann/json.hpp>

class Route {
	// Seems to be a 24-bit incremental ID, and an 8-bit type
	// How does this type interact with multi-route Strips? No idea.
	uint32_t _id = -1;
	std::vector<uint32_t> _anchors;

public:
	Route() = default;
	explicit Route(SplnetFileReader &fileReader, bool isFinal = false);

	void writeToFile(SplnetFileWriter &fileWriter, bool isFinal = false) const;

	[[nodiscard]] auto id() const { return _id; }
	void id(uint32_t set) { _id = set; }

	void remapAnchors(const std::map<uint32_t, uint32_t> &map);

	bool operator==(const Route &other) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Route, _id, _anchors);
};

template <> struct fmt::formatter<Route> : formatter<string_view> {
	static format_context::iterator format(const Route &a, format_context &ctx) {
		return format_to(ctx.out(), "Route #{}", a.id());
	}
};
