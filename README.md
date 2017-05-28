# GC REL Patcher
A tool for generically patching GameCube rel files.

## TODO

Finish api functions, write inline documentation, determine any other api functions needed.

## API (Early/In progress)

**Global/Uncategorized Functions**

Finds a list of relocations that reference a specified offset into a section

    findPointerAddresses(uint32_t sectionID, uint32_t offset) // Implemented?
	findPointerAddresses(uint32_t sectionID, uint32_t offset, uint32_t tolerance) // Implemented?

Get the current filesize

    filesize(); // Implmented

**Section Functions**

The size of a section in bytes
    
    sectionSize(uint32_t sectionID) // Implemented

The absolute offset of a section in bytes
    
    sectionOffset(uint32_t sectionID) // Implemented

Move a section to the end of the rel file

    moveSectionToEnd(uint32_t sectionID) // Implemented

Resize a section. No bounds or overlap checking is done

    resizeSectionUnsafe(uint32_t sectionID, newSize); // Implemented
    expandSectionUnsafe(uint32_t sectionID, uint32_t amount) // Implemented

Copies data from one location within a section to another

    copyData(uint32_t sectionID, uint32_t sourceOffset, uint32_t destinationOffset, uint32_t amount)
    copyData(uint32_t sourceSectionID, uint32_t sourceOffset, uint32_t destinationSectionID, uint32_t destinationOffset, uint32_t amount)
    
Write n-bytes to the specified section at the specified offset
    
    writeToSection(uint32_t sectionID, uint32_t offset, uint32_t value) // Implemented
    writeToSection(uint32_t sectionID, uint32_t offset, uint16_t value) // Implemented
    writeToSection(uint32_t sectionID, uint32_t offset, uint8_t value) // Implemented

Write n-byte values count times to the specified section at the specified offset
    
    writeToSection(uint32_t sectionID, uint32_t offset, uint32_t *values, uint32_t count) // Implemented
    writeToSection(uint32_t sectionID, uint32_t offset, uint16_t *values, uint32_t count) // Implemented
    writeToSection(uint32_t sectionID, uint32_t offset, uint8_t *values, uint32_t count) // Implemented

May not be added

    // Writes 3-bytes to the specified section at the specified offset
    writeToSection24(uint32_t sectionID, uint32_t offset, uint32_t value) // Unimplemented
    // Writes 2-bytes to the high value of specified section at the specified offset (assumes 4-byte location)
    writeToSection16HI(uint32_t sectionID, uint32_t offset, uint32_t value) // Unimplemented
    // Writes 2-bytes to the low value of specified section at the specified offset (assumes 4-byte location)
    writeToSection16LO(uint32_t sectionID, uint32_t offset, uint32_t value) // Unimplemented
    // Writes 14-bits to specified section at the specified offset
    writeToSection14(uint32_t sectionID, uint32_t offset, uint32_t value) // Unimplemented


**Relocation functions**

The absolute offset of the relocations in bytes
    
    relocationsOffset(uint32_t sectionID) // Implemented

Write n-bytes to the specified offset in the relocations section

    writeToRelocations(uint32_t offset, uint32_t value) // Implemented
    writeToRelocations(uint32_t offset, uint16_t value) // Implemented
    writeToRelocations(uint32_t offset, uint8_t value) // Implemented

Write n-byte values count times to the specified offset in the relocations section

    writeToRelocations(uint32_t offset, uint32_t *values, uint32_t count) // Implemented
    writeToRelocations(uint32_t offset, uint32_t *values, uint16_t count) // Implemented
    writeToRelocations(uint32_t offset, uint32_t *values, uint8_t count) // Implemented