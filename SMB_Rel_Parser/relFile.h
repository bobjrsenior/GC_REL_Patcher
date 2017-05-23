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
		std::ofstream relFile;

	public:
		RELFile(char *filename) {
			std::ifstream relInputFile(filename, std::ios::binary | std::ios::in);
			if (relInputFile.is_open()) {
				parseRel(relInputFile);
				relInputFile.close();
				relFile.open(filename, std::ios::binary | std::ios::in);
			}
			
		}

		RELFile(std::string filename) {
			std::ifstream relInputFile(filename, std::ios::binary | std::ios::in);
			if (relInputFile.is_open()) {
				parseRel(relInputFile);
				relInputFile.close();
				relFile.open(filename, std::ios::binary | std::ios::in);
			}
		}

		~RELFile() {
			if (relFile.is_open()) {
				relFile.close();
			}
		}



	private:
		void parseRel(std::ifstream &relInputFile) {
			parseHeader(relInputFile);

			parseSectionInfoTable(relInputFile);

			parseImportTable(relInputFile);
		}

		void parseHeader(std::ifstream &relInputFile) {
			// Save the current position and go to the beginning of the file
			uint32_t savePos = relInputFile.tellg();
			relInputFile.seekg(0, std::ifstream::_Seekbeg);

			readBigInt(relInputFile);
			header = std::make_unique<Header>();
			header->moduleID = readBigInt(relInputFile);
			header->nextModuleLink = readBigInt(relInputFile);
			header->previousModuleLink = readBigInt(relInputFile);
			header->sectionCount = readBigInt(relInputFile);
			header->sectionInfoOffset = readBigInt(relInputFile);
			header->moduleNameOffset = readBigInt(relInputFile);
			header->moduleNameSize = readBigInt(relInputFile);
			header->moduleVersion = readBigInt(relInputFile);
			header->bssSize = readBigInt(relInputFile);
			header->relocationTableOffset = readBigInt(relInputFile);
			header->importTableOffset = readBigInt(relInputFile);
			header->importTableSize = readBigInt(relInputFile);
			header->prologSection = readBigByte(relInputFile);
			header->epilogSection = readBigByte(relInputFile);
			header->unresolvedSection = readBigByte(relInputFile);
			header->padding = readBigByte(relInputFile);
			header->prologFunctionOffset = readBigInt(relInputFile);
			header->epilogFunctionOffset = readBigInt(relInputFile);
			header->unresolvedFunctionOffset = readBigInt(relInputFile);
			header->moduleAlignment = readBigInt(relInputFile);
			header->bssAlignment = readBigInt(relInputFile);
			header->unknown = readBigInt(relInputFile);

			header->importTableCount = header->importTableSize >> 3;

			// Return the the previous file position
			relInputFile.seekg(savePos, std::ifstream::_Seekbeg);
		}

		void parseSectionInfoTable(std::ifstream &relInputFile) {
			// Save the current position and go to the beginning of the file
			uint32_t savePos = relInputFile.tellg();
			relInputFile.seekg(0, std::ifstream::_Seekbeg);

			sectionInfoTable = std::make_unique<SectionInfoTable[]>(header->sectionCount);

			for (uint32_t i = 0; i < header->sectionCount; i++) {
				sectionInfoTable[i].offset = readBigInt(relInputFile);
				sectionInfoTable[i].size = readBigInt(relInputFile);
				uint32_t offset = sectionInfoTable[i].offset & ((~0) ^ 0x1);
				if (offset <= 0x0020B448 && offset + sectionInfoTable[i].size > 0x0020B448) {
					printf("OFFSET: %d\n", offset);
					int x = 5;
				}
			}

			// Return the the previous file position
			relInputFile.seekg(savePos, std::ifstream::_Seekbeg);
		}

		void parseImportTable(std::ifstream &relInputFile) {
			// Save the current position and go to the beginning of the file
			uint32_t savePos = relInputFile.tellg();
			relInputFile.seekg(0, std::ifstream::_Seekbeg);

			importTable = std::make_unique<ImportTable[]>(header->importTableCount);

			for (uint32_t i = 0; i < header->importTableCount; i++) {
				importTable[i].moduleID = readBigInt(relInputFile);
				importTable[i].relocationsOffset = readBigInt(relInputFile);
			}

			// Return the the previous file position
			relInputFile.seekg(savePos, std::ifstream::_Seekbeg);
		}
	};
}