#pragma once
#include <stdint.h>

typedef struct Header {
	uint32_t moduleID;					// Unique ID for this module. The main DOL file is module 0 so REL modules start at 1
	uint32_t nextModuleLink;			// Pointer to the next module forming linkedlist (always 0 until runtime)
	uint32_t previousModuleLink;		// Pointer to the previous module forming a linked list (always 0 until runtime)
	uint32_t sectionCount;				// Number of sections in the file
	uint32_t moduleNameOffset;			// Offset into the section table
	uint32_t moduleNameSize;			// Offset to the module name (can be 0, no name)
	uint32_t moduleVersion;				// Module version (1, 2, or 3)
	uint32_t bssSize;					// Size of the BSS section (allocated at runtime, not included in file)
	uint32_t relocationTableOffset;		// Absolute offset of the relocation table
	uint32_t importTableOffset;			// Absolute offset of the import table
	uint32_t importTableSize;			// Size of the import table
	uint8_t prologSection;				// Section index of the prolog function (called when the module is linked, 0 if no prolog function)
	uint8_t epilogSection;				// Section index of the epilog funtion (called when the modeule is unlinked, 0 if no epilog function)
	uint8_t unresolvedSection;			// Section index of the unresovled function (called when the module attempts to call an unlinked function, 0 if no unresolved function)
	uint8_t padding;					// Padding (for 4-byte alignment)
	uint32_t prologFunctionOffset;		// Section-relative offset of the prolog function (0 if no prolog function, converted to function pointer at runtime by OSLink)
	uint32_t epilogFunctionOffset;		// Section-relative offset of the epilog function (0 if no prolog function, converted to function pointer at runtime by OSLink)
	uint32_t unresolvedFunctionOffset;	// Section-relative offset of the unresolved function (0 if no prolog function, converted to function pointer at runtime by OSLink)
	uint32_t moduleAlignment;			// 32 for 4-byte alignment? (v2, v3 only)
	uint32_t bssAlignment;				// 32 for 4-byte alignment? (v2, v3 only)
	uint32_t unknown;					// (v3 only)
};

typedef struct SectionInfoTable {
	uint32_t offset;					// Absolute offset of the section (0x1 bit dertmines if executable. AND out bit for correct offset)
	uint32_t size;						// Size of section
};

typedef struct ImportTable {
	uint32_t moduleID;					// Module ID for this import (0 is the main DOL executable)
	uint32_t relocationsOffset;			// Absolute offset of the relocations for the import, pointing into the relocation table
};

typedef struct RelocationTable {
	uint16_t offset;					// Offset of this relocation relative to the offset of the last relocation entry
	uint8_t relocationType;				// Type of the relocation
	uint8_t sectionIndex;				// Section index of the symbol being patched to (only used for module patches)
	uint32_t symbolOffset;				// The section-relative offset of the symbol being patched to
};

const uint8_t R_RPC_NONE            = 0;	// Do nothing
const uint8_t R_RPC_ADDR32          = 1;	// Write the full 32-bit address of the symbol
const uint8_t R_RPC_ADDR24          = 2;	// Write the 24-bit address of the symbol, leave the existing value of the bottom 2 bits intact
const uint8_t R_RPC_ADDR16          = 3;	// Write the 16-bit address of the symbol
const uint8_t R_RPC_ADDR16_LO       = 4;	// Write the low 16 bits of the symbol address
const uint8_t R_RPC_ADDR16_HI       = 5;	// Write the high 16 bits of the symbol address
const uint8_t R_RPC_ADDR16_HA       = 6;	// Write the high 16 bits of the symbol address plus 0x100000
const uint8_t R_RPC_ADDR14          = 7;	// Write the 14-bit address of the symbol, leave the existing value of the bottom 2 bits intact (used for conditional branch instructions)
const uint8_t R_RPC_ADDR14_BRTAKEN  = 8;	// Write the 14-bit address of the symbol, leave the existing value of the bottom 2 bits intact
const uint8_t R_RPC_ADDR14_BRNTAKEN = 9;	// Write the 14-bit address of the symbol, leave the existing value of the bottom 2 bits intact
const uint8_t R_RPC_REL24           = 10;	// Write a 24-bit offset from the address of this instruction to the address of the symbol (used for branch instructions)
const uint8_t R_RPC_REL14           = 11;	// Write a 14-bit offset from the address of this instruction to the address of the symbol
const uint8_t R_DOLPHIN_NOP         = 201;	// Do nothing (used when the offset from one relocation to the next is larger than 0xFFFF bytes)
const uint8_t R_DOLPHIN_SECTION     = 202;	// Change the current section to SectionIndex and reset the current offset to 0
const uint8_t R_DOLPHIN_END         = 203;	// Marks the end of relocations for this import
