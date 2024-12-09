#include "Route.hpp"
Route::Route(SplnetFileReader &fileReader, bool isFinal) {
	fileReader.expectElementHeader();

	fileReader.expect<uint16_t>(0x029c);
	_id = fileReader.read<uint32_t>();
	fileReader.expect<uint32_t>(0x00);
	fileReader.expect<uint16_t>(0x05f7);
	fileReader.expect<uint16_t>(0x0001);
	fileReader.expect<uint16_t>(0x0003);

	while (fileReader.peek<uint16_t>() == 0x14) {
		fileReader.expect<uint16_t>(0x14);
		_anchors.emplace_back(fileReader.read<uint32_t>());
	}

	fileReader.expectElementFooter(isFinal);
}
void Route::writeToFile(SplnetFileWriter &fileWriter, bool isFinal) const {
	fileWriter.writeElementHeader();
	fileWriter.write<uint16_t>(0x029c);
	fileWriter.write(_id);
	fileWriter.write<uint32_t>(0x00);
	fileWriter.write<uint16_t>(0x05f7);
	fileWriter.write<uint16_t>(0x0001);
	fileWriter.write<uint16_t>(0x0003);

	for (const uint32_t &anchorID : _anchors) {
		fileWriter.write<uint16_t>(0x14);
		fileWriter.write(anchorID);
	}

	fileWriter.writeElementFooter(isFinal);
}

void Route::remapAnchors(const std::map<uint32_t, uint32_t> &map) {
	for (auto &anchor : _anchors) {
		if (map.contains(anchor))
			anchor = map.at(anchor);
	}
}
