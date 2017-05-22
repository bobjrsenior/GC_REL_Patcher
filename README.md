# GC REL Patcher
A tool for generically patching GameCube rel files.

## API (Early/In progress)

**Section Functions**

Move a section to the end of the rel file

    moveSectionToEnd(uint32_t sectionID)

Resize a section. If the filesize needs to grow, the default seek data will be left. If the filsize doesn't need to grow, the existing data is used

    resizeSection(uint32_t sectionID, newSize); Resizes the section to the specified file (truncation/zero padding)
    
Write n-bytes to the specified section at the specified offset
    
    writeToSection(uint32_t sectionID, uint32_t offset, uint32_t value)
    writeToSection(uint32_t sectionID, uint32_t offset, uint16_t value)
    writeToSection(uint32_t sectionID, uint32_t offset, uint8_t value)

Write n-byte values count times to the specified section at the specified offset
    
    writeToSection(uint32_t sectionID, uint32_t offset, uint32_t *values, uint32_t count)
    writeToSection(uint32_t sectionID, uint32_t offset, uint16_t *values, uint32_t count)
    writeToSection(uint32_t sectionID, uint32_t offset, uint8_t *values, uint32_t count)

May not be added

    // Writes 3-bytes to the specified section at the specified offset
    writeToSection24(uint32_t sectionID, uint32_t offset, uint32_t value)
    // Writes 2-bytes to the high value of specified section at the specified offset (assumes 4-byte location)
    writeToSection16HI(uint32_t sectionID, uint32_t offset, uint32_t value)
    // Writes 2-bytes to the low value of specified section at the specified offset (assumes 4-byte location)
    writeToSection16LO(uint32_t sectionID, uint32_t offset, uint32_t value)
    // Writes 14-bits to specified section at the specified offset
    writeToSection14(uint32_t sectionID, uint32_t offset, uint32_t value)


**Relocation functions**

Write n-bytes to the specified offset in the relocations section

    writeToRelocations(uint32_t offset, uint32_t value)
    writeToRelocations(uint32_t offset, uint16_t value)
    writeToRelocations(uint32_t offset, uint8_t value)

Write n-byte values count times to the specified offset in the relocations section

    writeToRelocations(uint32_t offset, uint32_t *values, uint32_t count)
    writeToRelocations(uint32_t offset, uint32_t *values, uint16_t count)
    writeToRelocations(uint32_t offset, uint32_t *values, uint8_t count)