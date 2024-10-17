#include "Route.hpp"
Route::Route(SplnetFileReader &fileReader, bool isFinal) {
	fileReader.expectElementHeader();

	fileReader.expect<uint16_t>(0x029c);
	_id = fileReader.read<uint64_t>();
	fileReader.expect<uint16_t>(0x05f7);
	fileReader.expect<uint16_t>(0x0001);
	fileReader.expect<uint16_t>(0x0003);

	while (fileReader.peek<uint16_t>() == 0x14) {
		fileReader.expect<uint16_t>(0x14);
		_anchors.emplace_back(fileReader.read<uint32_t>());
	}

	fileReader.expectElementFooter(isFinal);
}
