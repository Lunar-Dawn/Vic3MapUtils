#pragma once

#include <map>
#include <set>
#include <utility>

#include <nlohmann/json.hpp>

#include "Anchor.hpp"
#include "NetworkItemChanges.hpp"
#include "Route.hpp"
#include "Strip.hpp"

/// A list of changes that can be applied to a Network
/// Contains complete versions of everything so we can check that the correct version gets replaced
/// Avoids someone moving an anchor that later get reused for something else getting moved somewhere unexpected
struct Diff {
	NetworkItemChanges<uint32_t, Anchor> anchorChanges;
	NetworkItemChanges<std::pair<uint32_t, uint32_t>, Strip> stripChanges;
	NetworkItemChanges<uint32_t, Route> routeChanges;

	/// Merge another diff into this one, will reindex sub-anchors and routes in other
	/// Will print warnings and throw if multiple hub anchors with the same ID are present
	void mergeDiff(Diff other);

	/// Remap subanchors and routes with respect to the provided reserved ids
	void remapCollisions(std::set<uint32_t> anchorIds, std::set<uint32_t> routeIds);
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Diff, anchorChanges, stripChanges, routeChanges);
