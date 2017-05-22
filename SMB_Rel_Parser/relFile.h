#pragma once
#include <memory>
#include "structs.h"
#include "fileFunctions.h"
#include <string>

namespace RELPatch {

	class RELFile {
	public:

	private:
		std::unique_ptr<RELHeader> header;
		std::unique_ptr<RELSectionInfoTable[]> sectionInfoTable;
		std::unique_ptr<RELImportTable[]> importTable;
		FILE *relFile;

	public:
		RELFile(char *filename) {
			relFile = fopen(filename, "rb");
			if (relFile != NULL) {
				parseRel();
			}
		}

		RELFile(std::string filename) {
			relFile = fopen(filename.c_str(), "rb");
			if (relFile != NULL) {
				parseRel();
			}
		}

		~RELFile() {
			if (relFile != NULL) {
				fclose(relFile);
				relFile = NULL;
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
			uint32_t savePos = ftell(relFile);
			fseek(relFile, 0, SEEK_SET);

			header = std::make_unique<RELHeader>();
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
			fseek(relFile, savePos, SEEK_SET);
		}

		void parseSectionInfoTable() {
			// Save the current position and go to the beginning of the file
			uint32_t savePos = ftell(relFile);
			fseek(relFile, header->sectionInfoOffset, SEEK_SET);

			sectionInfoTable = std::make_unique<RELSectionInfoTable[]>(header->sectionCount);

			for (uint32_t i = 0; i < header->sectionCount; i++) {
				sectionInfoTable[i].offset = readBigInt(relFile);
				sectionInfoTable[i].size = readBigInt(relFile);
				uint32_t offset = sectionInfoTable[i].offset & ((~0) ^ 0x1);
				if (offset <= 0x0020B448 && offset + sectionInfoTable[i].size > 0x0020B448) {
					printf("OFFSET: %d\n", offset);
					int x = 5;
				}
			}

			// Return the the previous file position
			fseek(relFile, savePos, SEEK_SET);
		}

		void parseImportTable() {
			// Save the current position and go to the beginning of the file
			uint32_t savePos = ftell(relFile);
			fseek(relFile, header->importTableCount, SEEK_SET);

			importTable = std::make_unique<RELImportTable[]>(header->importTableCount);

			for (uint32_t i = 0; i < header->importTableCount; i++) {
				importTable[i].moduleID = readBigInt(relFile);
				importTable[i].relocationsOffset = readBigInt(relFile);
			}

			// Return the the previous file position
			fseek(relFile, savePos, SEEK_SET);
		}
	};
}