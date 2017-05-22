#pragma once
#include <stdio.h>
#include <stdint.h>

namespace RELPatch {

	inline uint32_t readBigInt(FILE *file) {
		uint32_t c1 = getc(file) << 24;
		uint32_t c2 = getc(file) << 16;
		uint32_t c3 = getc(file) << 8;
		uint32_t c4 = getc(file);
		return (c1 | c2 | c3 | c4);
	}

	inline uint32_t readLittleInt(FILE *file) {
		uint32_t c1 = getc(file);
		uint32_t c2 = getc(file) << 8;
		uint32_t c3 = getc(file) << 16;
		uint32_t c4 = getc(file) << 24;
		return (c1 | c2 | c3 | c4);
	}

	inline uint16_t readBigShort(FILE *file) {
		uint16_t c1 = (uint16_t)getc(file) << 8;
		uint16_t c2 = (uint16_t)getc(file);
		return (c1 | c2);
	}

	inline uint16_t readLittleShort(FILE *file) {
		uint16_t c1 = (uint16_t)getc(file);
		uint16_t c2 = (uint16_t)getc(file) << 8;
		return (c1 | c2);
	}

	inline uint8_t readBigByte(FILE *file) {
		return (uint8_t)getc(file);
	}

	inline uint8_t readLittleByte(FILE *file) {
		return (uint8_t)getc(file);
	}

	inline void writeBigInt(FILE *file, uint32_t value) {
		putc((value >> 24), file);
		putc((value >> 16), file);
		putc((value >> 8), file);
		putc((value), file);
	}

	inline void writeLittleInt(FILE *file, uint32_t value) {
		putc((value), file);
		putc((value >> 8), file);
		putc((value >> 16), file);
		putc((value >> 24), file);
	}

	inline void writeBigShort(FILE *file, uint16_t value) {
		putc((value >> 8), file);
		putc((value), file);
	}

	inline void writeLittleShort(FILE *file, uint16_t value) {
		putc((value), file);
		putc((value >> 8), file);
	}
}