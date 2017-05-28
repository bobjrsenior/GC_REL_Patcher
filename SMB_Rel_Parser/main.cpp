
#include "relFile.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
	/*for (int i = 1; i < argc; i++) {
		int length = strlen(argv[i]);
		int validRel = 0;
		if (length > 4) {
			if (argv[i][length - 4] == '.' &&
				argv[i][length - 3] == 'r' &&
				argv[i][length - 2] == 'e' &&
				argv[i][length - 1] == 'l') {
				validRel = 1;
				RELPatch::RELFile relFile(argv[i]);
			}
		}
	}*/
	RELPatch::RELFile relFile("mkb2.main_loop.rel");
	
	std::vector<RELPatch::RelocationTable> pointers = relFile.findPointerAddresses(5, 0x373E8, 64);
	std::cout << "Results:" << std::endl;
	for (uint32_t i = 0; i < pointers.size(); i++) {
		std::cout << "Absolute offset of relocation entry: " << pointers[i].absoluteRelocationOffset << '\n'
			<< "Relocation Type: " << (uint32_t)pointers[i].relocationType << '\n'
			<< "Destination Module ID: " << pointers[i].moduleID << '\n'
			<< "Source Section ID: " << (uint32_t)pointers[i].sourceSectionIndex << '\n'
			<< "Source Section Offset: " << pointers[i].sourceSectionOffset << '\n'
			<< "Destination Section ID: " << (uint32_t)pointers[i].sectionIndex << '\n'
			<< "Destination Section Offset: " << pointers[i].symbolOffset << '\n'
			<< "Difference from desired pointer: " << 0x373E8 - pointers[i].sourceSectionOffset << std::endl;
	}
	
	// Write single value to section
	relFile.writeToSection(1, 32, 0xDEADBEEF);
	relFile.writeToSection(1, 36, (uint16_t) 0xDEAD);
	relFile.writeToSection(1, 38, (uint8_t)0xBE);

	// Section ID checks
	relFile.writeToSection(0, 32, 0xDEADBEEF);
	relFile.writeToSection((uint32_t) -1, 32, 0xDEADBEEF);
	relFile.writeToSection(99999, 32, 0xDEADBEEF);

	uint32_t uint32Arr[5] = { 0xDEADBEEF , 0xDEADBEEF , 0xDEADBEEF , 0xDEADBEEF, 0xDEADBEEF };
	uint16_t uint16Arr[5] = { 0xDEAD , 0xDEAD , 0xDEAD , 0xDEAD, 0xDEAD };
	uint8_t uint8Arr[5] = { 0xDE , 0xDE , 0xDE , 0xDE, 0xDE };

	// Write multiple values to section
	relFile.writeToSection(1, 44, uint32Arr, 5);
	relFile.writeToSection(1, 72, uint16Arr, 5);
	relFile.writeToSection(1, 86, uint8Arr, 5);

	// Write single value to relocations
	relFile.writeToRelocations(32, 0xDEADBEEF);
	relFile.writeToRelocations(36, (uint16_t)0xDEAD);
	relFile.writeToRelocations(38, (uint8_t)0xDE);

	// Write multiple values to relocations
	relFile.writeToRelocations(44, uint32Arr, 5);
	relFile.writeToRelocations(72, uint16Arr, 5);
	relFile.writeToRelocations(86, uint8Arr, 5);
	
	// Move a section to the back
	relFile.moveSectionToEnd(1);

	// Resize a section
	relFile.expandSectionUnsafe(1, 1024);
	
	return 0;
}

