#pragma once

#include "Anchor.hpp"
#include "Diff.hpp"
#include "Route.hpp"
#include "Strip.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>

#include <fmt/ostream.h>

class SplineNetwork {
	std::map<uint32_t, Anchor> _anchors;
	std::map<uint32_t, Route> _routes;
	std::map<std::pair<uint32_t, uint32_t>, Strip> _strips;

	static std::tuple<uint32_t, uint32_t, uint32_t> parseFileHeader(SplnetFileReader &fileReader);
	void parseAnchorList(SplnetFileReader &fileReader, uint32_t count);
	void parseRouteList(SplnetFileReader &fileReader, uint32_t count);
	void parseStripList(SplnetFileReader &fileReader, uint32_t count);

public:
	SplineNetwork() = default;
	explicit SplineNetwork(const std::filesystem::path &path);

	void writeToFile(const std::filesystem::path &path) const;

	/// Calculate the changes to, other
	/// Usually called on the vanilla network with `other` being the modded network
	[[nodiscard]] Diff calculateDiff(const SplineNetwork &other) const;
	/// Apply the changes to this network
	/// Usually called on the vanilla network
	void applyDiff(Diff diff);

	template <typename K, typename T>
	void applyChangeList(std::map<K, T> &items, const NetworkItemChanges<K, T> &changes) {
		for (const auto &[id, versionPair] : changes.edits) {
			const auto &[oldVersion, newVersion] = versionPair;
			auto it = items.find(id);
			if (it == items.end()) {
				fmt::print(
				    std::cerr,
				    "{} did not exist in network. New version will still be inserted, but take care.\n"
				    "\tThis is likely due to the area previously edited being heavily altered, be very careful.\n",
				    oldVersion);
			} else {
				if (oldVersion != it->second) {
					fmt::print(std::cerr,
					           "{} data does not match previous version.\n"
					           "\tThis is not necessarily a problem, some nodes may just have been nudged, but be "
					           "careful.\n",
					           oldVersion);
				}
			}
			items[id] = newVersion;
		}

		for (const auto &[id, deletedItem] : changes.deletions) {
			auto it = items.find(id);
			if (it == items.end()) {
				fmt::print(
				    std::cerr,
				    "{} did not exist in network. Skipping deletion.\n"
				    "\tThis is likely due to the area previously edited being heavily altered, be very careful.\n",
				    deletedItem);
				continue;
			}
			if (deletedItem != it->second) {
				fmt::print(std::cerr,
				           "{} data does not match previous version.\n"
				           "\tThis is not necessarily a problem, some nodes may just have been nudged, but be "
				           "careful.\n",
				           deletedItem);
			}
			items.erase(it);
		}
		for (const auto &[id, newItem] : changes.additions) {
			// The remapping should make this never happen, but for safety's sake this is here
			if (items.contains(id)) {
				fmt::print(std::cerr,
				           "{} already exists in the network.\n"
				           "Insertion could be extremely dangerous, aborting.\n",
				           newItem);
				throw std::runtime_error("Attempting to insert item with existing id, aborting to maintain coherence.");
			}
			items[id] = newItem;
		}
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SplineNetwork, _anchors, _routes, _strips);
};
