#pragma once

#include "FileHandler/SplnetFileReader.hpp"
#include "FileHandler/SplnetFileWriter.hpp"

#include <vector>

#include <nlohmann/json.hpp>

class Route {
	uint64_t _id;
	std::vector<uint32_t> _anchors;

public:
	Route() = default;
	explicit Route(SplnetFileReader &fileReader, bool isFinal = false);

	void writeToFile(SplnetFileWriter &fileWriter, bool isFinal = false) const;

	[[nodiscard]] auto id() const { return _id; }

	bool operator==(const Route &other) const = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Route, _id, _anchors);
};

template <> struct fmt::formatter<Route> : formatter<string_view> {
	format_context::iterator format(const Route &a, format_context &ctx) const {
		return format_to(ctx.out(), "Route #{}", a.id());
	}
};
