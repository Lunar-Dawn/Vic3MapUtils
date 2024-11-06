#pragma once

#include "Anchor.hpp"
#include "Route.hpp"
#include "Strip.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <ranges>

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

	/// Calculate the differences (changed, added and removed values) between `from` and `to`
	/// and insert them into two different arrays
	template <typename T, typename K, std::weakly_incrementable O>
	void diffMaps(const std::map<K, T> &from, const std::map<K, T> &to, O deletedValues, O newValues) {
		// Transforms the IDs in the range into the value in the specific map
		auto fromTransform = std::views::transform([&](auto id) { return from.at(id); });
		auto toTransform = std::views::transform([&](auto id) { return to.at(id); });

		// All the IDs in both maps
		std::vector<K> preservedIDs;
		std::ranges::set_intersection(std::views::keys(from), std::views::keys(to), std::back_inserter(preservedIDs));
		// Only record if anything has changed
		auto changedElements =
		    preservedIDs | std::views::filter([&](const auto id) { return from.at(id) != to.at(id); });
		std::ranges::copy(changedElements | fromTransform, deletedValues);
		std::ranges::copy(changedElements | toTransform, newValues);

		// All the IDs only in the `from` map
		std::vector<K> deletedIDs;
		std::ranges::set_difference(std::views::keys(from), std::views::keys(to), std::back_inserter(deletedIDs));
		std::ranges::copy(deletedIDs | fromTransform, deletedValues);

		// All the IDs only in the `to` map
		std::vector<K> newIDs;
		std::ranges::set_difference(std::views::keys(to), std::views::keys(from), std::back_inserter(newIDs));
		std::ranges::copy(newIDs | toTransform, newValues);
	}

public:
	explicit SplineNetwork(const std::filesystem::path &path);

	void writeToFile(const std::filesystem::path &path) const;

	/// A list of changes that can be applied to a Network
	/// Contains complete versions of everything so we can check that the correct version gets replaced
	/// Avoids someone moving an anchor that later get reused for something else getting moved somewhere unexpected
	struct Diff {
		std::vector<Anchor> _anchorsDeleted;
		std::vector<Anchor> _anchorsAdded;

		std::vector<Strip> _stripsDeleted;
		std::vector<Strip> _stripsAdded;

		std::vector<Route> _routesDeleted;
		std::vector<Route> _routesAdded;
	};

	/// Calculate the changes to, other
	/// Usually called on the vanilla network with `other` being the modded network
	Diff calculateDiff(const SplineNetwork &other);
	/// Apply the changes to this network
	/// Usually called on the vanilla network
	void applyDiff(const Diff &diff);
};

/// Not 100% pleased with using underscored internal ids in the json export
/// But since they're meant to be ephemeral it's fine to save a lot of boilerplate
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SplineNetwork::Diff, _anchorsDeleted, _anchorsAdded, _stripsDeleted, _stripsAdded,
                                   _routesDeleted, _routesAdded);
