#pragma once

#include <map>
#include <utility>

#include <nlohmann/json.hpp>

#include "Anchor.hpp"
#include "Route.hpp"
#include "Strip.hpp"

/// A list of changes that can be applied to a Network
/// Contains complete versions of everything so we can check that the correct version gets replaced
/// Avoids someone moving an anchor that later get reused for something else getting moved somewhere unexpected
struct Diff {
	std::map<uint32_t, Anchor> _anchorsDeleted;
	std::map<uint32_t, Anchor> _anchorsAdded;

	std::map<std::pair<uint32_t, uint32_t>, Strip> _stripsDeleted;
	std::map<std::pair<uint32_t, uint32_t>, Strip> _stripsAdded;

	std::map<uint32_t, Route> _routesDeleted;
	std::map<uint32_t, Route> _routesAdded;
};

/// Not 100% pleased with using underscored internal ids in the json export
/// But since they're meant to be ephemeral it's fine to save a lot of boilerplate
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Diff, _anchorsDeleted, _anchorsAdded, _stripsDeleted, _stripsAdded, _routesDeleted,
                                   _routesAdded);
