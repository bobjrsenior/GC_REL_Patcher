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

		void writeToSection(uint32_t sectionID, uint32_t offset, uint32_t value) {
			if (sectionID >= 0 && sectionID < header->sectionCount) {
				relFile.seekg(toAddress(sectionInfoTable[sectionID].offset, offset), std::fstream::beg);
				writeBigInt(relFile, value);
			}
		}


	private:
		uint32_t toAddress(uint32_t raw) {
			return raw & ((~0) ^ 0x1);
		}

		std::streamoff toAddress(uint32_t raw, uint32_t offset) {
			return (std::streamoff) ((raw & ((~0) ^ 0x1)) + offset);
		}

		void parseRel() {
			parseHeader();

			parseSectionInfoTable();

			parseImportTable();
		}

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
			}

			// Return the the previous file position
			relFile.seekg(savePos, std::fstream::beg);
		}

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