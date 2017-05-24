
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

	relFile.writeToSection(1, 32, 0xDEADBEEF);

	relFile.writeToSection(1, 36, (uint16_t) 0xDEAD);

	relFile.writeToSection(1, 38, (uint8_t)0xBE);

	relFile.writeToSection(0, 32, 0xDEADBEEF);
	relFile.writeToSection(-1, 32, 0xDEADBEEF);
	relFile.writeToSection(99999, 32, 0xDEADBEEF);
}

