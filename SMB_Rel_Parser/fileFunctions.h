#pragma once
#include <stdio.h>
#include <fstream>
#include <stdint.h>

namespace RELPatch {

	inline uint32_t readBigInt(std::fstream &fileStream) {
		char bytes[4];
		fileStream.read(bytes, 4);
		return (bytes[0] | bytes[1] | bytes[2] | bytes[3]);
	}

	inline uint16_t readBigShort(std::fstream &fileStream) {
		char bytes[2];
		fileStream.read(bytes, 2);
		return (bytes[0] | bytes[1]);
	}

	inline uint8_t readBigByte(std::fstream &fileStream) {
		return (uint8_t)fileStream.get();
	}

	inline void writeBigInt(std::fstream fileStream, uint32_t value) {
		char bytes[4];
		bytes[0] = (value >> 24);
		bytes[1] = (value >> 16);
		bytes[2] = (value >> 8);
		bytes[3] = (value);

		fileStream.write(bytes, 4);
	}

	inline void writeBigShort(std::fstream fileStream, uint16_t value) {
		char bytes[2];
		bytes[2] = (value >> 8);
		bytes[3] = (value);

		fileStream.write(bytes, 2);
	}

	inline void writeBigByte(std::fstream fileStream, uint8_t value) {
		fileStream.put((char)value);
	}
}