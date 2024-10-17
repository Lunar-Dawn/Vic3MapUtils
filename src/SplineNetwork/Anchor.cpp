#include "Anchor.hpp"

Anchor::Anchor(SplnetFileReader &fileReader, bool isFinal) {
	fileReader.expectElementHeader();

	fileReader.expect<uint16_t>(0x14);
	_id = fileReader.read<uint32_t>();
	fileReader.expect<uint16_t>(0x4c);
	fileReader.expect<uint16_t>(0x01);
	fileReader.expect<uint16_t>(0x03);
	fileReader.expect<uint16_t>(0x0d);
	_posX = fileReader.read<float>();
	fileReader.expect<uint16_t>(0x0d);
	_posY = fileReader.read<float>();

	fileReader.expectElementFooter(isFinal);
}
void Anchor::writeToFile(SplnetFileWriter &fileWriter, bool isFinal) const {
	fileWriter.writeElementHeader();

	fileWriter.write<uint16_t>(0x14);
	fileWriter.write(_id);
	fileWriter.write<uint16_t>(0x4c);
	fileWriter.write<uint16_t>(0x01);
	fileWriter.write<uint16_t>(0x03);
	fileWriter.write<uint16_t>(0x0d);
	fileWriter.write(_posX);
	fileWriter.write<uint16_t>(0x0d);
	fileWriter.write(_posY);

	fileWriter.writeElementFooter(isFinal);
}
