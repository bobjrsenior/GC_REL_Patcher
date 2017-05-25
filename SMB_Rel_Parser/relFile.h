#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include "structs.h"
#include "fileFunctions.h"
#include <string>

namespace RELPatch {

	class RELFile {
	public:

	private:
		std::unique_ptr<Header> header;
		std::unique_ptr<SectionInfoTable[]> sectionInfoTable;
		std::unique_ptr<ImportTable[]> importTable;
		std::fstream relFile;

	public:
		RELFile(char *filename) {
			relFile.open(filename, std::ios::binary | std::ios::in | std::ios::out);
			if (relFile.is_open()) {
				parseRel();
			}
		}

		RELFile(std::string filename) {
			relFile.open(filename, std::ios::binary | std::ios::in | std::ios::out);
			if (relFile.is_open()) {
				parseRel();
			}
		}

		~RELFile() {
			if (relFile.is_open()) {
				relFile.close();
			}
		}

		/*
			Retreives the current filesize of the rel file.
		*/
		std::streamoff filesize() {
			// Save the current position and go to the beginning of the file
			std::streamoff savePos = relFile.tellg();
			relFile.seekg(0, std::fstream::end);
			
			std::streamoff filesize = relFile.tellg();
			// Return the the previous file position
			relFile.seekg(savePos, std::fstream::beg);

			return filesize;
		}

		/*
			Checks to see if the given <sectionID> is executable.
			Returns 1 if exectutable
			Returns 0 if not executable
		*/
		uint8_t isSectionExecutable(uint32_t sectionID) {
			if (validSection(sectionID)) {
				return sectionInfoTable[sectionID].offset & 0x1;
			}
			return 0;
		}

		/*
			Write a 4-byte <value> to the specified <offset> relative to the  <sectionID>'s offset.
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint32_t value) {
			if (validSection(sectionID)) {
				relFile.seekg(toAddress(sectionInfoTable[sectionID].offset, offset), std::fstream::beg);
				writeBigInt(relFile, value);
			}
		}

		/*
			Write a 2-byte <value> to the specified <offset> relative to the  <sectionID>'s offset.
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint16_t value) {
			if (validSection(sectionID)) {
				relFile.seekg(toAddress(sectionInfoTable[sectionID].offset, offset), std::fstream::beg);
				writeBigShort(relFile, value);
			}
		}

		/*
			Write a 1-byte <value> to the specified <offset> relative to the section id's offset.
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint8_t value) {
			if (validSection(sectionID)) {
				relFile.seekg(toAddress(sectionInfoTable[sectionID].offset, offset), std::fstream::beg);
				writeBigByte(relFile, value);
			}
		}

		/*
			Write a series of <count> 4-byte <values> to the specified <offset> relative to the <sectionID>'s offset.
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint32_t *values, int32_t count) {
			if(validSection(sectionID)) {
				write(toAddress(sectionInfoTable[sectionID].offset, offset), values, count);
			}
		}

		/*
			Write a series of <count> 2-byte <values> to the specified <offset> relative to the <sectionID>'s offset.
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint16_t *values, int32_t count) {
			if (validSection(sectionID)) {
				write(toAddress(sectionInfoTable[sectionID].offset, offset), values, count);
			}
		}

		/*
			Write a series of <count> 1-byte <values> to the specified <offset> relative to the <sectionID>'s offset.
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint8_t *values, int32_t count) {
			if(validSection(sectionID)) {
				write(toAddress(sectionInfoTable[sectionID].offset, offset), values, count);
			}
		}

		/*
			Moves the <sectionID>'s section to the back of the file.
			This will increase the filesize, so be careful about using it multiple times.
			Especially if it is used multiple times on one <sectionID>.
		*/
		void moveSectionToEnd(uint32_t sectionID) {
			if (validSection(sectionID)) {
				// The new section will now be at the current end of the file
				std::streamoff newSectionOffset = filesize();
				uint8_t isExecutable = isSectionExecutable(sectionID);

				// Promoted to int64 to avoid unsigned ambiguities
				int64_t offset = toAddress(sectionInfoTable[sectionID].offset);
				int64_t bytesLeft = sectionInfoTable[sectionID].size;
				int64_t bytesWritten = 0;
				int64_t maxBufferSize = 1 << 14; // 256 KiB
				int64_t buffserSize;
				if (bytesLeft < maxBufferSize) {
					buffserSize = bytesLeft;
				}
				else {
					buffserSize = maxBufferSize;
				}
				// Allocate our buffer
				std::unique_ptr<char[]> buffer = std::make_unique<char[]>((size_t)buffserSize);

				// Loop until nothing is left
				while (bytesLeft > 0) {
					relFile.seekg(offset + bytesWritten, std::fstream::beg);

					// Make sure not to read more than there is
					if (bytesLeft < maxBufferSize) {
						buffserSize = bytesLeft;
					}
					else {
						buffserSize = maxBufferSize;
					}

					// Read in the buffer
					relFile.read(buffer.get(), buffserSize);
					bytesLeft -= buffserSize;

					// Seek to the end and write out the buffer
					relFile.seekg(0, std::fstream::end);
					relFile.write(buffer.get(), buffserSize);
					bytesWritten += buffserSize;
				}

				// Update our stored section offset
				sectionInfoTable[sectionID].offset = toSectionOffsetFormat((uint32_t)newSectionOffset, isExecutable);
				
				// Update the rel file's section offset
				relFile.seekg(header->sectionInfoOffset + (0x8 * sectionID), std::fstream::beg);
				writeBigInt(relFile, sectionInfoTable[sectionID].offset);
			}
		}

		////////


		/*
			Write a 4-byte <value> to the specified <offset> relative to the start of the relocations.
		*/
		void writeToRelocations(uint32_t offset, uint32_t value) {
			relFile.seekg(toAddress(header->relocationTableOffset, offset), std::fstream::beg);
			writeBigInt(relFile, value);
		}

		/*
			Write a 2-byte <value> to the specified <offset> relative to the start of the relocations.
		*/
		void writeToRelocations(uint32_t offset, uint16_t value) {
			relFile.seekg(toAddress(header->relocationTableOffset, offset), std::fstream::beg);
			writeBigShort(relFile, value);
		}

		/*
			Write a 1-byte <value> to the specified <offset> relative to the start of the relocations.
		*/
		void writeToRelocations(uint32_t offset, uint8_t value) {
			relFile.seekg(toAddress(header->relocationTableOffset, offset), std::fstream::beg);
			writeBigByte(relFile, value);
		}

		/*
			Write a series of <count> 4-byte <values> to the specified <offset> relative to the start of the relocations.
		*/
		void writeToRelocations(uint32_t offset, uint32_t *values, int32_t count) {
			write(toAddress(header->relocationTableOffset, offset), values, count);
		}

		/*
			Write a series of <count> 2-byte <values> to the specified <offset> relative to the start of the relocations.
		*/
		void writeToRelocations(uint32_t offset, uint16_t *values, int32_t count) {
			write(toAddress(header->relocationTableOffset, offset), values, count);
		}

		/*
			Write a series of <count> 1-byte <values> to the specified <offset> relative to the start of the relocations.
		*/
		void writeToRelocations(uint32_t offset, uint8_t *values, int32_t count) {
			write(toAddress(header->relocationTableOffset, offset), values, count);
		}

	private:

		/*
			Converts a <raw> rel position into an absolute offset.
			This is done by zeroing out the least significant bit (the executable bit).
		*/
		uint32_t toAddress(uint32_t raw) {
			return raw & (~1);
		}

		/*
			Combines a <raw> rel position with an <offset> into an absolute offset.
			This is done by zeroing out the least significant bit (the executable bit) and adding the <offset>.
		*/
		std::streamoff toAddress(uint32_t raw, uint32_t offset) {
			return (std::streamoff) ((raw & (~1)) + offset);
		}

		/*
			Combines an absolute <offset> and a boolean of if the address <isExecutable> into a raw rel position.
		*/
		uint32_t toSectionOffsetFormat(uint32_t offset, uint8_t isExecutable) {
			return toAddress(offset) | isExecutable;
		}

		/*
			Checks if a <sectionID> is valid
			A <sectionID> is valid if it exists in the rel file and has an offset other than 0
		*/
		bool validSection(uint32_t sectionID) {
			if (sectionID >= 0 && sectionID < header->sectionCount && sectionInfoTable[sectionID].offset != 0) {
				return true;
			}
			return false;
		}

		/*
			Write a series of <count> 4-byte <values> to the rel file at the specified <offset>.
		*/
		void write(std::streamoff offset, uint32_t *values, int32_t count) {
			relFile.seekg(offset, std::fstream::beg);
			for (int32_t i = 0; i < count; i++) {
				writeBigInt(relFile, values[i]);
			}
		}

		/*
			Write a series of <count> 2-byte <values> to the rel file at the specified <offset>.
		*/
		void write(std::streamoff offset, uint16_t *values, int32_t count) {
			relFile.seekg(offset, std::fstream::beg);
			for (int32_t i = 0; i < count; i++) {
				writeBigShort(relFile, values[i]);
			}
		}

		/*
			Write a series of <count> 1-byte <values> to the rel file at the specified <offset>.
		*/
		void write(std::streamoff offset, uint8_t *values, int32_t count) {
			relFile.seekg(offset, std::fstream::beg);
			for (int32_t i = 0; i < count; i++) {
				writeBigByte(relFile, values[i]);
			}
		}

		/*
			Parses the rel file's headers by calling other helper functions
		*/
		void parseRel() {
			parseHeader();

			parseSectionInfoTable();

			parseImportTable();
		}

		/*
			Parses the rel file's main header
		*/
		void parseHeader() {
			// Save the current position and go to the beginning of the file
			std::streamoff savePos = relFile.tellg();
			relFile.seekg(0, std::fstream::beg);

			header = std::make_unique<Header>();
			header->moduleID = readBigInt(relFile);
			header->nextModuleLink = readBigInt(relFile);
			header->previousModuleLink = readBigInt(relFile);
			header->sectionCount = readBigInt(relFile);
			header->sectionInfoOffset = readBigInt(relFile);
			header->moduleNameOffset = readBigInt(relFile);
			header->moduleNameSize = readBigInt(relFile);
			header->moduleVersion = readBigInt(relFile);
			header->bssSize = readBigInt(relFile);
			header->relocationTableOffset = readBigInt(relFile);
			header->importTableOffset = readBigInt(relFile);
			header->importTableSize = readBigInt(relFile);
			header->prologSection = readBigByte(relFile);
			header->epilogSection = readBigByte(relFile);
			header->unresolvedSection = readBigByte(relFile);
			header->padding = readBigByte(relFile);
			header->prologFunctionOffset = readBigInt(relFile);
			header->epilogFunctionOffset = readBigInt(relFile);
			header->unresolvedFunctionOffset = readBigInt(relFile);

			// Version specific
			if (header->moduleVersion > 1) {
				header->moduleAlignment = readBigInt(relFile);
				header->bssAlignment = readBigInt(relFile);
				if (header->moduleVersion > 2) {
					header->unknown = readBigInt(relFile);
				}
			}

			header->importTableCount = header->importTableSize >> 3;

			// Return the the previous file position
			relFile.seekg(savePos, std::fstream::beg);
		}

		/*
			Parses the rel file's section info table
		*/
		void parseSectionInfoTable() {
			// Save the current position and go to the beginning of the file
			std::streamoff savePos = relFile.tellg();
			relFile.seekg((std::streamoff) header->sectionInfoOffset, std::fstream::beg);

			sectionInfoTable = std::make_unique<SectionInfoTable[]>(header->sectionCount);

			for (uint32_t i = 0; i < header->sectionCount; i++) {
				sectionInfoTable[i].offset = readBigInt(relFile);
				sectionInfoTable[i].size = readBigInt(relFile);
				//uint32_t offset = sectionInfoTable[i].offset & ((~0) ^ 0x1);
				//if (offset <= 0x0020B448 && offset + sectionInfoTable[i].size > 0x0020B448) {
				//	printf("OFFSET: %d\n", offset);
				//	int x = 5;
				//}
				//std::cout << sectionInfoTable[i].size << std::endl;
			}

			// Return the the previous file position
			relFile.seekg(savePos, std::fstream::beg);
		}

		/*
			Parses the rel file's section import table
		*/
		void parseImportTable() {
			// Save the current position and go to the beginning of the file
			std::streamoff savePos = relFile.tellg();
			relFile.seekg((std::streamoff) header->importTableOffset, std::fstream::beg);

			importTable = std::make_unique<ImportTable[]>(header->importTableCount);

			for (uint32_t i = 0; i < header->importTableCount; i++) {
				importTable[i].moduleID = readBigInt(relFile);
				importTable[i].relocationsOffset = readBigInt(relFile);
			}

			// Return the the previous file position
			relFile.seekg(savePos, std::fstream::beg);
		}
	};
}