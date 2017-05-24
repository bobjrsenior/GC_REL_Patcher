#pragma once
#include <stdio.h>
#include <fstream>
#include <stdint.h>

namespace RELPatch {

	inline uint32_t readBigInt(std::fstream &fileStream) {
		unsigned char bytes[4];
		fileStream.read((char*)bytes, 4);

		uint32_t value = ((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
		return ((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
	}

	inline uint16_t readBigShort(std::fstream &fileStream) {
		unsigned char bytes[2];
		fileStream.read((char*)bytes, 2);
		return ((bytes[0] << 8) | bytes[1]);
	}

	inline uint8_t readBigByte(std::fstream &fileStream) {
		return (uint8_t)fileStream.get();
	}

	inline void writeBigInt(std::fstream &fileStream, uint32_t value) {
		unsigned char bytes[4];
		bytes[0] = (unsigned char)(value >> 24);
		bytes[1] = (unsigned char)(value >> 16);
		bytes[2] = (unsigned char)(value >> 8);
		bytes[3] = (unsigned char)(value);

		fileStream.write((char*)bytes, 4);
	}

	inline void writeBigShort(std::fstream &fileStream, uint16_t value) {
		unsigned char bytes[2];
		bytes[0] = (unsigned char)(value >> 8);
		bytes[1] = (unsigned char)(value);

		fileStream.write((char*)bytes, 2);
	}

	inline void writeBigByte(std::fstream &fileStream, uint8_t value) {
		fileStream.put((char)value);
	}
}