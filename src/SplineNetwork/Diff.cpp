#include "Diff.hpp"

#include <iostream>
#include <ranges>
#include <set>

#include <fmt/ostream.h>

void Diff::mergeDiff(Diff other) {
	std::set<uint32_t> reservedAnchorIds;
	std::set<uint32_t> reservedRouteIds;
	for (const auto &id : anchorChanges.additions | std::views::keys) {
		reservedAnchorIds.emplace(id);
	}
	for (const auto &id : routeChanges.additions | std::views::keys) {
		reservedRouteIds.emplace(id);
	}

	other.remapCollisions(reservedAnchorIds, reservedRouteIds);

	anchorChanges.merge(other.anchorChanges);
	routeChanges.merge(other.routeChanges);
	stripChanges.merge(other.stripChanges);
}
void Diff::remapCollisions(std::set<uint32_t> anchorIds, std::set<uint32_t> routeIds) {
	// A map between the old and new ids for anchors, only remapped for sub-anchors
	std::map<uint32_t, uint32_t> anchorIdRemapping;

	// Map reserved ids to themselves, keeping them the same
	for (const auto &id : anchorIds) {
		anchorIdRemapping[id] = id;
	}

	uint32_t landId = 1 | 1 << 28;
	uint32_t waterId = 1 | 1 << 28 | 1 << 23;

	bool hubCollisions = false;
	for (const auto &[id, anchor] : anchorChanges.additions) {
		if (!anchorIds.contains(id)) {
			anchorIdRemapping[id] = id;
			anchorIds.emplace(id);
			continue;
		}

		if (!anchor.isSubAnchor()) {
			hubCollisions = true;
			fmt::print(std::cerr,
			           "Attempting to remap Hub Anchor #{} (type {}), meaning multiple networks contain it. "
			           "Which to keep would be ambiguous, aborting.\n",
			           anchor.niceID(), (uint32_t)anchor.isWaterAnchor());
			continue;
		}

		auto &newId = id & (1 << 23) ? waterId : landId;
		while (anchorIds.contains(newId))
			newId++;

		anchorIds.emplace(newId);
		anchorIdRemapping[id] = newId;
	}
	if (hubCollisions) {
		throw std::runtime_error("Refusing to merge networks with shared Hub Anchors.");
	}

	// A map between the old and new ids for routes
	std::map<uint32_t, uint32_t> routeIdRemapping;
	// Map reserved ids to themselves, keeping them the same
	for (const auto &id : routeIds) {
		routeIdRemapping[id] = id;
	}

	std::array<uint32_t, 4> newRouteIds = {1, 1, 1, 1};

	for (auto &[id, route] : routeChanges.additions) {
		if (!routeIds.contains(id)) {
			routeIdRemapping[id] = id;
			routeIds.emplace(id);
			continue;
		}

		const auto type = id & 0xFF;
		auto &newId = newRouteIds[type];
		while (routeIds.contains(newId << 8 | type))
			newId++;

		routeIds.emplace(newId << 8 | type);
		routeIdRemapping[id] = newId << 8 | type;
	}

	std::map<uint32_t, Anchor> newAnchorAdditions;
	for (auto &[id, anchor] : anchorChanges.additions) {
		anchor.id(anchorIdRemapping.at(id));
		newAnchorAdditions.emplace(anchor.id(), std::move(anchor));
	}
	anchorChanges.additions = std::move(newAnchorAdditions);

	std::map<uint32_t, Route> newRouteAdditions;
	for (auto &[id, route] : routeChanges.additions) {
		route.id(routeIdRemapping.at(id));
		route.remapAnchors(anchorIdRemapping);
		newRouteAdditions.emplace(route.id(), std::move(route));
	}
	routeChanges.additions = std::move(newRouteAdditions);

	for (auto &[id, strip] : stripChanges.additions) {
		if (anchorIdRemapping.contains(strip.sourceID()))
			strip.sourceID(anchorIdRemapping.at(strip.sourceID()));
		if (anchorIdRemapping.contains(strip.destinationID()))
			strip.destinationID(anchorIdRemapping.at(strip.destinationID()));
		strip.remapRoutes(routeIdRemapping);
	}
}
