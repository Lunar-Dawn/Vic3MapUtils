#include "Strip.hpp"
Strip::Strip(SplnetFileReader &fileReader, bool isFinal) {
	fileReader.expectElementHeader();

	fileReader.expect<uint16_t>(0x029c);
	_sourceID = fileReader.read<uint32_t>();
	_destinationID = fileReader.read<uint32_t>();
	fileReader.expect<uint16_t>(0x05f5);
	fileReader.expect<uint16_t>(0x0001);
	fileReader.expect<uint16_t>(0x0003);

	// Route IDs are prefixed by 0x029c, so we scan until we see the footer instead
	// This only scans more than one Route twice in vanilla,
	// and there's no tooling to make more forks in the editor (in fact, the editor easily breaks existing forks)
	// But it needs to be handled
	while (fileReader.peek<uint16_t>() == 0x029c) {
		fileReader.expect<uint16_t>(0x029c);
		_routeIDs.emplace_back(fileReader.read<uint64_t>());
	}

	fileReader.expectElementFooter(isFinal);
}
void Strip::writeToFile(SplnetFileWriter &fileWriter, bool isFinal) const {
	fileWriter.writeElementHeader();

	fileWriter.write<uint16_t>(0x029c);
	fileWriter.write(_sourceID);
	fileWriter.write(_destinationID);
	fileWriter.write<uint16_t>(0x05f5);
	fileWriter.write<uint16_t>(0x0001);
	fileWriter.write<uint16_t>(0x0003);

	for (const uint64_t &routeID : _routeIDs) {
		fileWriter.write<uint16_t>(0x029c);
		fileWriter.write(routeID);
	}

	fileWriter.writeElementFooter(isFinal);
}
void Strip::remapRoutes(const std::map<uint32_t, uint32_t> &map) {
	for (auto &routeID : _routeIDs) {
		routeID = map.at(routeID);
	}
}
