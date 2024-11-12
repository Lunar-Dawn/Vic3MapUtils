#include "Diff.hpp"

#include <iostream>
#include <ranges>
#include <set>

#include <fmt/ostream.h>

void Diff::mergeDiff(const Diff &other) {
	// A map between the old and new ids for anchors, only remapped for sub-anchors
	std::map<uint32_t, uint32_t> anchorIdRemapping;
	std::set<uint32_t> usedAnchorIds;

	// Map our ids to themselves, keeping them the same
	for (const auto &id : anchorChanges.additions | std::views::keys) {
		usedAnchorIds.emplace(id);
	}

	uint32_t landId = 1 << 28;
	uint32_t waterId = 1 << 23 | 1 << 28;

	bool hubCollisions = false;
	for (const auto &[id, anchor] : other.anchorChanges.additions) {
		if (!usedAnchorIds.contains(id)) {
			anchorIdRemapping[id] = id;
			usedAnchorIds.emplace(id);
			continue;
		}

		if (!anchor.isSubAnchor()) {
			hubCollisions = true;
			fmt::print(
			    std::cerr,
			    "Multiple networks contain Hub Anchor #{} (type {}), merging would produce an invalid network.\n",
			    anchor.niceID(), (uint32_t)anchor.isWaterAnchor());
			continue;
		}
		auto &newId = id & (1 << 23) ? waterId : landId;
		while (usedAnchorIds.contains(newId))
			newId++;

		usedAnchorIds.emplace(newId);
		anchorIdRemapping[id] = newId;
	}
	if (hubCollisions) {
		throw std::runtime_error("Refusing to merge networks with shared Hub Anchors.");
	}

	for (const auto &[id, anchor] : other.anchorChanges.additions) {
		auto copy = anchor;
		copy.id(anchorIdRemapping.at(id));
		anchorChanges.additions.emplace(copy.id(), copy);
	}

	// A map between the old and new ids for routes
	std::map<uint32_t, uint32_t> routeIdRemapping;
	std::set<uint32_t> usedRouteIds;
	// Map our ids to themselves, keeping them the same
	for (const auto &id : routeChanges.additions | std::views::keys) {
		usedRouteIds.emplace(id);
	}

	std::array<uint32_t, 4> newRouteIds = {0, 0, 0, 0};

	for (const auto &[id, route] : other.routeChanges.additions) {
		if (!usedRouteIds.contains(id)) {
			routeIdRemapping[id] = id;
			usedRouteIds.emplace(id);
		} else {
			const auto type = id & 0xFF;
			auto &newId = newRouteIds[type];
			while (usedRouteIds.contains(newId << 8 | type))
				newId++;

			usedRouteIds.emplace(newId << 8 | type);
			routeIdRemapping[id] = newId << 8 | type;
		}

		auto copy = route;
		copy.id(routeIdRemapping.at(id));
		copy.remapAnchors(anchorIdRemapping);
		routeChanges.additions.emplace(copy.id(), copy);
	}

	for (const auto &[id, strip] : other.stripChanges.additions) {
		auto copy = strip;
		copy.sourceID(anchorIdRemapping[copy.sourceID()]);
		copy.destinationID(anchorIdRemapping[copy.destinationID()]);
		copy.remapRoutes(routeIdRemapping);
		stripChanges.additions.emplace(copy.idPair(), copy);
	}
}
