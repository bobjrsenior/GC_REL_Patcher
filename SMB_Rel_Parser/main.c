#include "structs.h"
#include "fileFunctions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void findPointer(const char *filename, uint32_t offset, uint32_t tolerance);
void parseHeader(FILE* file, Header *header);
void parseSectionInfoTable(FILE* file, SectionInfoTable *sectionInfoTable, uint32_t sectionTableOffset, uint32_t sectionCount);
void parseImportTable(FILE* file, ImportTable *importTable, uint32_t importTableOffset, uint32_t importTableCount);


int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		int length = strlen(argv[i]);
		int validRel = 0;
		if (length > 4) {
			if (argv[i][length - 4] == '.' &&
				argv[i][length - 3] == 'r' &&
				argv[i][length - 2] == 'e' &&
				argv[i][length - 1] == 'l') {
				printf("Good");
				validRel = 1;
				findPointer(argv[i], 0x0020B448, 0x20);
			}
		}
	}
}

void findPointer(const char *filename, uint32_t offset, uint32_t tolerance) {
	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		printf("Unable to open file: %s\n", filename);
		return;
	}
	Header fileHeader;
	parseHeader(file, &fileHeader);

	SectionInfoTable *sectionInfoTable = malloc(sizeof(SectionInfoTable) * fileHeader.sectionCount);
	parseSectionInfoTable(file, sectionInfoTable, fileHeader.sectionInfoOffset, fileHeader.sectionCount);

	ImportTable *importTable = malloc(fileHeader.importTableSize);
	uint32_t importTableCount = fileHeader.importTableSize >> 3;
	parseImportTable(file, importTable, fileHeader.importTableOffset, importTableCount);

	int x = 5;

	free(sectionInfoTable);
	free(importTable);
}

void parseHeader(FILE* file, Header *header) {
	// Save the current position and go to the beginning of the file
	uint32_t savePos = ftell(file);
	fseek(file, 0, SEEK_SET);

	header->moduleID					= readBigInt(file);
	header->nextModuleLink				= readBigInt(file);
	header->previousModuleLink			= readBigInt(file);
	header->sectionCount				= readBigInt(file);
	header->sectionInfoOffset			= readBigInt(file);
	header->moduleNameOffset			= readBigInt(file);
	header->moduleNameSize				= readBigInt(file);
	header->moduleVersion				= readBigInt(file);
	header->bssSize						= readBigInt(file);
	header->relocationTableOffset		= readBigInt(file);
	header->importTableOffset			= readBigInt(file);
	header->importTableSize				= readBigInt(file);
	header->prologSection				= readBigByte(file);
	header->epilogSection				= readBigByte(file);
	header->unresolvedSection			= readBigByte(file);
	header->padding						= readBigByte(file);
	header->prologFunctionOffset		= readBigInt(file);
	header->epilogFunctionOffset		= readBigInt(file);
	header->unresolvedFunctionOffset	= readBigInt(file);
	header->moduleAlignment				= readBigInt(file);
	header->bssAlignment				= readBigInt(file);
	header->unknown						= readBigInt(file);

	// Return the the previous file position
	fseek(file, savePos, SEEK_SET);
}

void parseSectionInfoTable(FILE* file, SectionInfoTable *sectionInfoTable, uint32_t sectionTableOffset, uint32_t sectionCount) {
	// Save the current position and go to the beginning of the file
	uint32_t savePos = ftell(file);
	fseek(file, sectionTableOffset, SEEK_SET);
	
	for (uint32_t i = 0; i < sectionCount; i++) {
		sectionInfoTable[i].offset		= readBigInt(file);
		sectionInfoTable[i].size		= readBigInt(file);
	}

	// Return the the previous file position
	fseek(file, savePos, SEEK_SET);
}

void parseImportTable(FILE* file, ImportTable *importTable, uint32_t importTableOffset, uint32_t importTableCount) {
	// Save the current position and go to the beginning of the file
	uint32_t savePos = ftell(file);
	fseek(file, importTableOffset, SEEK_SET);

	for (uint32_t i = 0; i < importTableCount; i++) {
		importTable[i].moduleID				= readBigInt(file);
		importTable[i].relocationsOffset	= readBigInt(file);

	}

	// Return the the previous file position
	fseek(file, savePos, SEEK_SET);
}