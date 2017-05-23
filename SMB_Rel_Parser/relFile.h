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
			relFile.open(filename, std::ios::binary | std::ios::in);
			if (relFile.is_open()) {
				parseRel();
			}
		}

		RELFile(std::string filename) {
			relFile.open(filename, std::ios::binary | std::ios::in);
			if (relFile.is_open()) {
				parseRel();
			}
		}

		~RELFile() {
			if (relFile.is_open()) {
				relFile.close();
			}
		}


	private:
		void parseRel() {
			parseHeader();

			parseSectionInfoTable();

			parseImportTable();
		}

		void parseHeader() {
			// Save the current position and go to the beginning of the file
			uint32_t savePos = relFile.tellg();
			relFile.seekg(0, std::ifstream::_Seekbeg);

			readBigInt(relFile);
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
			header->moduleAlignment = readBigInt(relFile);
			header->bssAlignment = readBigInt(relFile);
			header->unknown = readBigInt(relFile);

			header->importTableCount = header->importTableSize >> 3;

			// Return the the previous file position
			relFile.seekg(savePos, std::ifstream::_Seekbeg);
		}

		void parseSectionInfoTable() {
			// Save the current position and go to the beginning of the file
			uint32_t savePos = relFile.tellg();
			relFile.seekg(0, std::ifstream::_Seekbeg);

			sectionInfoTable = std::make_unique<SectionInfoTable[]>(header->sectionCount);

			for (uint32_t i = 0; i < header->sectionCount; i++) {
				sectionInfoTable[i].offset = readBigInt(relFile);
				sectionInfoTable[i].size = readBigInt(relFile);
				uint32_t offset = sectionInfoTable[i].offset & ((~0) ^ 0x1);
				//if (offset <= 0x0020B448 && offset + sectionInfoTable[i].size > 0x0020B448) {
				//	printf("OFFSET: %d\n", offset);
				//	int x = 5;
				//}
			}

			// Return the the previous file position
			relFile.seekg(savePos, std::ifstream::_Seekbeg);
		}

		void parseImportTable() {
			// Save the current position and go to the beginning of the file
			uint32_t savePos = relFile.tellg();
			relFile.seekg(0, std::ifstream::_Seekbeg);

			importTable = std::make_unique<ImportTable[]>(header->importTableCount);

			for (uint32_t i = 0; i < header->importTableCount; i++) {
				importTable[i].moduleID = readBigInt(relFile);
				importTable[i].relocationsOffset = readBigInt(relFile);
			}

			// Return the the previous file position
			relFile.seekg(savePos, std::ifstream::_Seekbeg);
		}
	};
}