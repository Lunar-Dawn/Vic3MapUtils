#pragma once

#include <cstdint>
#include <map>
#include <utility>

#include <nlohmann/json.hpp>

/// A collection of all the changes for type T in a network
template <typename K, typename T> struct NetworkItemChanges {
	std::map<K, T> deletions;
	std::map<K, T> additions;
	std::map<K, std::pair<T, T>> edits;

	/// Calculate the differences (changed, added and removed values) between `from` and `to`
	/// and insert them into the correct maps
	void diffMaps(const std::map<K, T> &from, const std::map<K, T> &to) {
		// Transform the ID in the range to a pair<old,new>
		auto editedTransform =
		    std::views::transform([&](auto id) { return std::pair(id, std::pair(from.at(id), to.at(id))); });
		// Transforms the IDs in the range into the value in the specific map
		auto fromTransform = std::views::transform([&](auto id) { return std::pair(id, from.at(id)); });
		auto toTransform = std::views::transform([&](auto id) { return std::pair(id, to.at(id)); });

		// All the IDs in both maps
		std::vector<K> preservedIDs;
		std::ranges::set_intersection(std::views::keys(from), std::views::keys(to), std::back_inserter(preservedIDs));
		// Only record if anything has changed
		auto changedElements =
		    preservedIDs | std::views::filter([&](const auto id) { return from.at(id) != to.at(id); });
		std::ranges::copy(changedElements | editedTransform, std::inserter(edits, edits.end()));

		// All the IDs only in the `from` map
		std::vector<K> deletedIDs;
		std::ranges::set_difference(std::views::keys(from), std::views::keys(to), std::back_inserter(deletedIDs));
		std::ranges::copy(deletedIDs | fromTransform, std::inserter(deletions, deletions.end()));

		// All the IDs only in the `to` map
		std::vector<K> newIDs;
		std::ranges::set_difference(std::views::keys(to), std::views::keys(from), std::back_inserter(newIDs));
		std::ranges::copy(newIDs | toTransform, std::inserter(additions, additions.end()));
	}
};
template <typename K, typename T> void to_json(nlohmann::json &json, const NetworkItemChanges<K, T> &changeList) {
	json["deletions"] = changeList.deletions;
	json["additions"] = changeList.additions;
	json["edits"] = changeList.edits;
}
template <typename K, typename T> void from_json(const nlohmann::json &json, NetworkItemChanges<K, T> &changeList) {
	json.at("deletions").get_to(changeList.deletions);
	json.at("additions").get_to(changeList.additions);
	json.at("edits").get_to(changeList.edits);
}
