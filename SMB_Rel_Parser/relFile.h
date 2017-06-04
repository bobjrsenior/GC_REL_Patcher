#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include "structs.h"
#include "fileFunctions.h"
#include <string>
#include <vector>
#include <errno.h>

namespace RELPatch {

	class RELFile {
	public:

	private:
		std::unique_ptr<Header> header;
		std::unique_ptr<SectionInfoTable[]> sectionInfoTable;
		std::unique_ptr<ImportTable[]> importTable;
		std::fstream relFile;

	public:
		RELFile(char const*filename) {
			relFile.open(filename, std::ios::binary | std::ios::in | std::ios::out);
			if (relFile.is_open()) {
				parseRel();
			}
		}

		RELFile(std::string const& filename) {
			relFile.open(filename, std::ios::binary | std::ios::in | std::ios::out);
			if (relFile.is_open()) {
				parseRel();
			}
		}

		/*
			Retreives the current filesize of the rel file
		*/
		std::streamoff filesize() {
			// Save the current position and go to the beginning of the file
			std::streamoff savePos = relFile.tellg();
			relFile.seekg(0, std::fstream::end);
			
			std::streamoff filesize = relFile.tellg();
			// Return the the previous file position
			relFile.seekg(savePos, std::fstream::beg);

			return filesize;
		}

		/*
			Gets the size of <sectionID>
			Return -1 on invalid <sectionID>
		*/
		uint32_t sectionSize(uint32_t sectionID) {
			if (validSection(sectionID)) {
				return sectionInfoTable[sectionID].size;
			}
			return 0xFFFFFFFF;
		}

		/*
			Gets the size of <sectionID> rounded up to the next 4 bytes
			Return -1 on invalid <sectionID>
		*/
		uint32_t sectionSizeRounded(uint32_t sectionID) {
			if (validSection(sectionID)) {
				uint32_t size = sectionInfoTable[sectionID].size;
				if (size % 4 != 0) {
					size += 4 - (size % 4);
				}
				return sectionInfoTable[sectionID].size;
			}
			return 0xFFFFFFFF;
		}

		/*
			Gets the offset of <sectionID>
			Return -1 on invalid <sectionID>
		*/
		uint32_t sectionOffset(uint32_t sectionID) {
			if (validSection(sectionID)) {
				return toAddress(sectionInfoTable[sectionID].offset);
			}
			return 0xFFFFFFFF;
		}

		/*
			Checks to see if the given <sectionID> is executable
			Returns 1 if exectutable
			Returns 0 if not executable
		*/
		uint8_t isSectionExecutable(uint32_t sectionID) {
			if (validSection(sectionID)) {
				return sectionInfoTable[sectionID].offset & 0x1;
			}
			return 0;
		}

		/*
			Write a 4-byte <value> to the specified <offset> relative to the  <sectionID>'s offset
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint32_t value) {
			if (validSection(sectionID)) {
				relFile.seekg(toAddress(sectionInfoTable[sectionID].offset, offset), std::fstream::beg);
				writeBigInt(relFile, value);
			}
		}

		/*
			Write a 2-byte <value> to the specified <offset> relative to the  <sectionID>'s offset
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint16_t value) {
			if (validSection(sectionID)) {
				relFile.seekg(toAddress(sectionInfoTable[sectionID].offset, offset), std::fstream::beg);
				writeBigShort(relFile, value);
			}
		}

		/*
			Write a 1-byte <value> to the specified <offset> relative to the section id's offset
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint8_t value) {
			if (validSection(sectionID)) {
				relFile.seekg(toAddress(sectionInfoTable[sectionID].offset, offset), std::fstream::beg);
				writeBigByte(relFile, value);
			}
		}

		/*
			Write a series of <count> 4-byte <values> to the specified <offset> relative to the <sectionID>'s offset
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint32_t *values, int32_t count) {
			if(validSection(sectionID)) {
				write(toAddress(sectionInfoTable[sectionID].offset, offset), values, count);
			}
		}

		/*
			Write a series of <count> 2-byte <values> to the specified <offset> relative to the <sectionID>'s offset
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint16_t *values, int32_t count) {
			if (validSection(sectionID)) {
				write(toAddress(sectionInfoTable[sectionID].offset, offset), values, count);
			}
		}

		/*
			Write a series of <count> 1-byte <values> to the specified <offset> relative to the <sectionID>'s offset
		*/
		void writeToSection(uint32_t sectionID, uint32_t offset, uint8_t *values, int32_t count) {
			if(validSection(sectionID)) {
				write(toAddress(sectionInfoTable[sectionID].offset, offset), values, count);
			}
		}

		/*
			Moves the <sectionID>'s section to the back of the file
			This will increase the filesize, so be careful about using it multiple times
			Especially if it is used multiple times on one <sectionID>
		*/
		void moveSectionToEnd(uint32_t sectionID) {
			if (validSection(sectionID)) {
				// The new section will now be at the current end of the file
				std::streamoff newSectionOffset = filesize();
				uint8_t isExecutable = isSectionExecutable(sectionID);

				// Promoted to int64 to avoid unsigned ambiguities
				int64_t offset = toAddress(sectionInfoTable[sectionID].offset);
				
				copyData(toAddress(sectionInfoTable[sectionID].offset), (int64_t)newSectionOffset, sectionInfoTable[sectionID].size);

				// Update our stored section offset
				sectionInfoTable[sectionID].offset = toSectionOffsetFormat((uint32_t)newSectionOffset, isExecutable);
				std::cout << sectionInfoTable[sectionID].offset << std::endl;
				// Update the rel file's section offset
				relFile.seekg(header->sectionInfoOffset + (0x8 * sectionID), std::fstream::beg);
				writeBigInt(relFile, sectionInfoTable[sectionID].offset);
			}
		}

		/*
			Resizes <sectionID> to <newSize>
			No bounds/overlap checks are done
		*/
		uint32_t resizeSectionUnsafe(uint32_t sectionID, uint32_t newSize) {
			if (validSection(sectionID) && newSize > 0) {
				// Update our stored section offset
				sectionInfoTable[sectionID].size = newSize;

				// Update the rel file's section offset
				relFile.seekg(header->sectionInfoOffset + (0x8 * sectionID) + 0x4, std::fstream::beg);
				writeBigInt(relFile, sectionInfoTable[sectionID].size);

				return sectionInfoTable[sectionID].size;
			}
			return 0xFFFFFFFF;
		}

		/*
			Expands <sectionID> by <amount>
			Amount can only be positive (it's unsigned after all)
			No bounds/overlap checks are done
		*/
		uint32_t expandSectionUnsafe(uint32_t sectionID, uint32_t amount) {
			if (validSection(sectionID) && amount > 0) {
				return resizeSectionUnsafe(sectionID, sectionInfoTable[sectionID].size + amount);
			}
			return 0xFFFFFFFF;
		}

		/*
			Expands <sectionID> by <amount> rounded up to the next 4 bytes
			Amount can only be positive (it's unsigned after all)
			No bounds/overlap checks are done
		*/
		uint32_t expandSectionUnsafeRounded(uint32_t sectionID, uint32_t amount) {
			if (validSection(sectionID) && amount > 0) {
				uint32_t newSize = sectionInfoTable[sectionID].size;
				if (newSize % 4 != 0) {
					newSize += 4 - (newSize % 4);
				}
				newSize += amount;
				return resizeSectionUnsafe(sectionID, newSize);
			}
			return 0xFFFFFFFF;
		}

		/*
			Copy <amount> number of bytes from <sourceOffset> in <sectionID> to <destinationOffset> in <sectionID>
		*/
		void copyData(uint32_t sectionID, uint32_t sourceOffset, uint32_t destinationOffset, uint32_t amount) {
			copyData(sectionID, sourceOffset, sectionID, destinationOffset, amount);
		}

		/*
		Copy <amount> number of bytes from <sourceOffset> in <sourceSectionID> to <destinationOffset> in <destinationSectionID>
		*/
		void copyData(uint32_t sourceSectionID, uint32_t sourceOffset, uint32_t destinationSectionID, uint32_t destinationOffset, uint32_t amount) {
			if (validSection(sourceSectionID) && validSection(destinationSectionID)) {
				int64_t sourceSectionAbsoluteAddress = toAddress(sectionInfoTable[sourceSectionID].offset, sourceOffset);
				int64_t destinationSectionAbsoluteAddress = toAddress(sectionInfoTable[destinationSectionID].offset, destinationOffset);

				copyData(sourceSectionAbsoluteAddress, destinationSectionAbsoluteAddress, amount);
			}
		}

		/*
			Finds a list of relocation entries that point to <offset> within <sectionID>
		*/
		std::vector<RelocationTable> findPointerAddresses(uint32_t sectionID, uint32_t offset) {
			if (validSection(sectionID) && offset < toAddress(sectionInfoTable[sectionID].size)) {
				return findPointers(sectionID, offset);
			}
			std::vector<RelocationTable> empty;
			return empty;
		}

		/*
			Finds a list of relocation entries that point to <offset> within <sectionID> with an error range of <tolerance> bytes
		*/
		std::vector<RelocationTable> findPointerAddresses(uint32_t sectionID, uint32_t offset, uint32_t tolerance) {
			if (validSection(sectionID) && offset < toAddress(sectionInfoTable[sectionID].size)) {
				return findPointers(sectionID, offset, tolerance);
			}
			std::vector<RelocationTable> empty;
			return empty;
		}

		////////

		/*
		Gets the offset of the relocations
		*/
		uint32_t relocationsOffset() {
			return header->relocationTableOffset;
		}

		/*
			Write a 4-byte <value> to the specified <offset> relative to the start of the relocations
		*/
		void writeToRelocations(uint32_t offset, uint32_t value) {
			relFile.seekg(toAddress(header->relocationTableOffset, offset), std::fstream::beg);
			writeBigInt(relFile, value);
		}

		/*
			Write a 2-byte <value> to the specified <offset> relative to the start of the relocations
		*/
		void writeToRelocations(uint32_t offset, uint16_t value) {
			relFile.seekg(toAddress(header->relocationTableOffset, offset), std::fstream::beg);
			writeBigShort(relFile, value);
		}

		/*
			Write a 1-byte <value> to the specified <offset> relative to the start of the relocations
		*/
		void writeToRelocations(uint32_t offset, uint8_t value) {
			relFile.seekg(toAddress(header->relocationTableOffset, offset), std::fstream::beg);
			writeBigByte(relFile, value);
		}

		/*
			Write a series of <count> 4-byte <values> to the specified <offset> relative to the start of the relocations
		*/
		void writeToRelocations(uint32_t offset, uint32_t *values, int32_t count) {
			write(toAddress(header->relocationTableOffset, offset), values, count);
		}

		/*
			Write a series of <count> 2-byte <values> to the specified <offset> relative to the start of the relocations
		*/
		void writeToRelocations(uint32_t offset, uint16_t *values, int32_t count) {
			write(toAddress(header->relocationTableOffset, offset), values, count);
		}

		/*
			Write a series of <count> 1-byte <values> to the specified <offset> relative to the start of the relocations
		*/
		void writeToRelocations(uint32_t offset, uint8_t *values, int32_t count) {
			write(toAddress(header->relocationTableOffset, offset), values, count);
		}

	private:

		/*
			Converts a <raw> rel position into an absolute offset
			This is done by zeroing out the least significant bit (the executable bit)
		*/
		uint32_t toAddress(uint32_t raw) {
			return raw & (~1);
		}

		/*
			Combines a <raw> rel position with an <offset> into an absolute offset
			This is done by zeroing out the least significant bit (the executable bit) and adding the <offset>
		*/
		std::streamoff toAddress(uint32_t raw, uint32_t offset) {
			return (std::streamoff) ((raw & (~1)) + offset);
		}

		/*
			Combines an absolute <offset> and a boolean of if the address <isExecutable> into a raw rel position
		*/
		uint32_t toSectionOffsetFormat(uint32_t offset, uint8_t isExecutable) {
			uint32_t address = toAddress(offset);
			uint32_t orValue = address | isExecutable;
			return orValue;
		}

		/*
			Checks if a <sectionID> is valid
			A <sectionID> is valid if it exists in the rel file and has an offset other than 0
		*/
		bool validSection(uint32_t sectionID) {
			if (sectionID < header->sectionCount && sectionInfoTable[sectionID].offset != 0) {
				return true;
			}
			return false;
		}

		/*
			Copies <amount> bytes from absolute address <sourceOffset> to absolute address <destinationOffset>
		*/
		void copyData(int64_t sourceOffset, int64_t destinationOffset, int64_t amount) {
			int64_t maxBufferSize = 1 << 17; // 128 KiB
			int64_t bufferSize;
			
			int64_t distance = destinationOffset - sourceOffset;
			if (distance < maxBufferSize) {
				// Destination after source and source/destination overlap
				if(distance > amount){
					// Make sure we copy data without overlap
					// At most we can copy distance data at a time (working from the end backwards)

					int64_t tempSourceOffset = sourceOffset + amount - distance;
					int64_t tempDestinationOffset = destinationOffset + amount - distance;
					while (tempSourceOffset >= sourceOffset) {
						copyData(tempSourceOffset, tempDestinationOffset, distance, distance);

						tempSourceOffset -= distance;
						tempDestinationOffset -= distance;
					}
					return;
				}
				else {
					bufferSize = distance;
				}
			}
			else {
				bufferSize = maxBufferSize;

			}
			copyData(sourceOffset, destinationOffset, amount, bufferSize);
		}


		/*
			Copies <amount> bytes from absolute address <sourceOffset> to absolute address <destinationOffset>
			Copy at most <maxBufferSize> bytes at a time
		*/
		void copyData(int64_t sourceOffset, int64_t destinationOffset, int64_t amount, int64_t maxBufferSize) {
			// Promoted to int64 to avoid unsigned ambiguities
			int64_t bytesLeft = amount;
			int64_t bytesWritten = 0;
			int64_t buffserSize;
			if (bytesLeft < maxBufferSize) {
				buffserSize = bytesLeft;
			}
			else {
				buffserSize = maxBufferSize;

			}
			// Allocate our buffer
			std::unique_ptr<char[]> buffer = std::make_unique<char[]>((size_t)buffserSize);

			// Loop until nothing is left
			while (bytesLeft > 0) {
				relFile.seekg(sourceOffset + bytesWritten, std::fstream::beg);

				// Make sure not to read more than there is
				if (bytesLeft < maxBufferSize) {
					buffserSize = bytesLeft;
				}
				else {
					buffserSize = maxBufferSize;
				}

				// Read in the buffer
				relFile.read(buffer.get(), buffserSize);
				bytesLeft -= buffserSize;

				// Seek to the end and write out the buffer
				relFile.seekg(destinationOffset + bytesWritten, std::fstream::beg);
				relFile.write(buffer.get(), buffserSize);
				bytesWritten += buffserSize;
			}
		}

		/*
			Write a series of <count> 4-byte <values> to the rel file at the specified <offset>
		*/
		void write(std::streamoff offset, uint32_t *values, int32_t count) {
			relFile.seekg(offset, std::fstream::beg);
			for (int32_t i = 0; i < count; i++) {
				writeBigInt(relFile, values[i]);
			}
		}

		/*
			Write a series of <count> 2-byte <values> to the rel file at the specified <offset>
		*/
		void write(std::streamoff offset, uint16_t *values, int32_t count) {
			relFile.seekg(offset, std::fstream::beg);
			for (int32_t i = 0; i < count; i++) {
				writeBigShort(relFile, values[i]);
			}
		}

		/*
			Write a series of <count> 1-byte <values> to the rel file at the specified <offset>
		*/
		void write(std::streamoff offset, uint8_t *values, int32_t count) {
			relFile.seekg(offset, std::fstream::beg);
			for (int32_t i = 0; i < count; i++) {
				writeBigByte(relFile, values[i]);
			}
		}

		/*
			Parses the rel file's headers by calling other helper functions
		*/
		void parseRel() {
			parseHeader();

			parseSectionInfoTable();

			parseImportTable();
		}

		/*
			Parses the rel file's main header
		*/
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

		/*
			Parses the rel file's section info table
		*/
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
				//std::cout << sectionInfoTable[i].size << std::endl;
			}

			// Return the the previous file position
			relFile.seekg(savePos, std::fstream::beg);
		}

		/*
			Parses the rel file's section import table
		*/
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

		/////

		/*
			Finds a list of relocation entries that point to <offset> within <sectionID>
			Assumes sectionID is valid and <offset> is less than the size of <sectionID>
		*/
		std::vector<RelocationTable> findPointers(uint32_t sectionID, uint32_t offset) {
			return findPointers(sectionID, offset, 0);
		}

		/*
			Finds a list of relocation entries that point to <offset> within <sectionID> with an error range of <tolerance> bytes
			Assumes sectionID is valid and <offset> is less than the size of <sectionID>
		*/
		std::vector<RelocationTable> findPointers(uint32_t sectionID, uint32_t offset, uint32_t tolerance) {

			std::vector<RelocationTable> pointers;
			uint32_t minDifference = 0xFFFFFFFF;

			// Set the lower bound being careful about underflow
			uint32_t lowerBound;
			if (tolerance <= offset) {
				lowerBound = offset - tolerance;
			}
			else {
				lowerBound = 0;
			}
			


			// Find only relavant import tables (with the same module ID as this rel file)
			for (uint32_t i = 0; i < header->importTableCount; i++) {
				// Traverse the import tables relocations
				relFile.seekg((std::streamoff) importTable[i].relocationsOffset, std::fstream::beg);

				RelocationTable relTableDest;
				relTableDest.destinationSectionIndex = 0;
				relTableDest.destinationSectionOffset = 0;
				
				uint8_t currentSourceSectionID = 0;
				uint32_t currentSourceOffset = 0;

				uint8_t currentDestinationSectionID = 0;
				uint32_t currentDestinationOffset = 0;

				do{
					relTableDest.absoluteRelocationOffset = (uint32_t) relFile.tellg();
					relTableDest.offset = readBigShort(relFile);
					relTableDest.relocationType = readBigByte(relFile);
					relTableDest.sectionIndex = readBigByte(relFile);
					relTableDest.symbolOffset = readBigInt(relFile);

					currentSourceSectionID = relTableDest.sectionIndex;
					currentSourceOffset = relTableDest.symbolOffset;
					currentDestinationOffset += relTableDest.offset;

					// We are looking for a pointer in a specific section
					if (currentSourceSectionID == sectionID) {
						// If we are within tolerance and this is the closest offset so far or tied with the closest
						if (currentSourceOffset >= lowerBound && currentSourceOffset <= offset && offset - currentSourceOffset <= minDifference) {
							// If nothing else has been this close, clear the pointer list
							if (offset - currentSourceOffset < minDifference) {
								minDifference = offset - currentSourceOffset;
								pointers.clear();
							}

							// Add this pointers information to the pointer list
							relTableDest.moduleID = importTable[i].moduleID;
							relTableDest.destinationSectionIndex = currentDestinationSectionID;
							relTableDest.destinationSectionOffset = currentDestinationOffset;
							pointers.push_back(relTableDest);
							
							/*
							std::cout << "Calculated symbol" << std::endl;
							std::cout << "File Position of Relocations: " << relFile.tellg() << std::endl;
							std::cout << "File position of pointer: " << toAddress(sectionInfoTable[currentSourceSesourceSectionIndexctionID].offset, currentSourceOffset) << std::endl;
							std::cout << "Distance from wanted position: " << offset - currentSourceOffset << std::endl;
							std::cout << "Section: " << (uint32_t)currentSourceSectionID << std::endl;
							std::cout << "Offet: " << currentSourceOffset << std::endl;
							std::cout << "Relocation Type: " << (uint32_t)relTableDest.relocationType << std::endl;
							std::cout << "Module ID: " << i << std::endl;
							*/
						}
					}
					
					// Determine what to do based on the relocation type
					switch (relTableDest.relocationType) {
					case (uint8_t)RelocationType::R_DOLPHIN_SECTION:
						currentDestinationSectionID = relTableDest.sectionIndex;
						currentDestinationOffset = 0;
						break;
					}
				} while (relTableDest.relocationType != (uint8_t) RelocationType::R_DOLPHIN_END);
			}
			return pointers;
		}

		public:

		/*
			Applies the relocations and outputs the results in the file "relocatedRel.rel"
			Experimental
		*/
		void applyRelocations() {
			// Save the current position and go to the beginning of the file
			std::streamoff savePos = relFile.tellg();
			relFile.seekg(0, std::fstream::beg);

			// Create a duplicate rel to avoid breaking the original
			std::fstream relocated("relocatedRel.rel", std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
			if (!relocated.is_open()) {
				std::cout << "Failed to create relocations file: " << strerror(errno) << std::endl;
				return;
			}
			relocated << relFile.rdbuf();
			relocated.flush();
			// Return the the previous file position
			relFile.seekg(savePos, std::fstream::beg);

			uint8_t currentSourceSectionID = 0;
			uint32_t currentSourceOffset = 0;

			uint8_t currentDestinationSectionID = 0;
			uint32_t currentDestinationOffset = 0;

			uint32_t numRelocations = 0;
			// Find only relavant import tables (with the same module ID as this rel file)
			for (uint32_t i = 0; i < header->importTableCount; i++) {
				// Only do patches for this rel file for now
				if (importTable[i].moduleID == header->moduleID) {
					RelocationTable relTableSource;
					currentSourceSectionID = 0;
					currentSourceOffset = 0;

					std::streamoff relocationsPosition = (std::streamoff) importTable[i].relocationsOffset;
					do {
						// Make sure we are in the right position of the relocations
						relocated.seekg(relocationsPosition, std::fstream::beg);

						relTableSource.offset = readBigShort(relocated);
						relTableSource.relocationType = readBigByte(relocated);
						relTableSource.sectionIndex = readBigByte(relocated);
						relTableSource.symbolOffset = readBigInt(relocated);
						relocationsPosition = relocated.tellg();

						currentSourceSectionID = relTableSource.sectionIndex;
						currentSourceOffset = relTableSource.symbolOffset;

						currentDestinationOffset += relTableSource.offset;

						// Absolute destination/source address
						std::streamoff destinationAddress = toAddress(sectionInfoTable[currentDestinationSectionID].offset, currentDestinationOffset);
						std::streamoff sourceAddress = toAddress(sectionInfoTable[currentSourceSectionID].offset, currentSourceOffset);

						// Declare variables used in switch
						uint32_t existingValue;
						uint16_t lowBits;
						uint16_t highBits;
						uint8_t lowByte;
						uint32_t instructionToSymbolOffset;
						if(relTableSource.relocationType == (uint8_t)RelocationType::R_DOLPHIN_SECTION
							|| (validSection(currentSourceSectionID) && validSection(currentDestinationSectionID))) {
							// Determine what to do based on the relocation type
							switch (relTableSource.relocationType) {
							case (uint8_t)RelocationType::R_PPC_NONE:
								// Do nothing
								break;
							case (uint8_t)RelocationType::R_PPC_ADDR32:

								relocated.seekg(destinationAddress, std::fstream::beg);

								writeBigInt(relocated, (uint32_t)sourceAddress);

								break;
							case (uint8_t)RelocationType::R_PPC_ADDR24:
								relocated.seekg(destinationAddress, std::fstream::beg);
								existingValue = readBigInt(relocated);
								relocated.seekg(destinationAddress, std::fstream::beg);

								highBits = (uint16_t)((sourceAddress >> 16) & 0xFFFF);
								lowByte = (uint8_t)(sourceAddress & 0xFF);

								// and out low 2 bits of lowByte
								lowByte = (uint8_t)(lowByte & (~3));
								// or in the low 2 bits of existingValue
								lowByte |= (uint8_t)(existingValue & 0b11);

								writeBigByte(relocated, 0);
								writeBigShort(relocated, highBits);
								writeBigByte(relocated, lowByte);
								break;
							case (uint8_t)RelocationType::R_PPC_ADDR16_LO:
								// Offset by two in order to reach the low 16 bits
								relocated.seekg(destinationAddress + 2, std::fstream::beg);

								lowBits = (uint16_t)(sourceAddress & 0xFFFF);

								writeBigShort(relocated, lowBits);
								break;
							case (uint8_t)RelocationType::R_PPC_ADDR16_HI:
								relocated.seekg(destinationAddress, std::fstream::beg);

								highBits = (uint16_t)((sourceAddress >> 16) & 0xFFFF);

								writeBigShort(relocated, highBits);
								break;
							case (uint8_t)RelocationType::R_PPC_ADDR16_HA:
								relocated.seekg(destinationAddress, std::fstream::beg);

								highBits = (uint16_t)((sourceAddress >> 16) & 0xFFFF);
								highBits += 1; // ? High 16 bits plus 0x10000

								writeBigShort(relocated, highBits);
								break;
							case (uint8_t)RelocationType::R_PPC_ADDR14:
							case (uint8_t)RelocationType::R_PPC_ADDR14_BRTAKEN:
							case (uint8_t)RelocationType::R_PPC_ADDR14_BRNTAKEN:
								relocated.seekg(destinationAddress, std::fstream::beg);
								existingValue = readBigInt(relocated);
								relocated.seekg(destinationAddress, std::fstream::beg);

								lowBits = (uint16_t)(sourceAddress & 0x3FFF);

								// and out low 2 bits of lowBits
								lowBits = (uint16_t)(lowBits & (~3));
								// or in the low 2 bits of existingValue
								lowBits |= (uint16_t)(existingValue & 0b11);

								writeBigShort(relocated, 0);
								writeBigShort(relocated, lowBits);

								break;
							case (uint8_t)RelocationType::R_PPC_REL24:
								relocated.seekg(destinationAddress, std::fstream::beg);

								instructionToSymbolOffset = (uint32_t)(sourceAddress - relocationsPosition);

								highBits = (uint16_t)((instructionToSymbolOffset >> 16) & 0xFFFF);
								lowByte = (uint8_t)(instructionToSymbolOffset & 0xFF);

								writeBigByte(relocated, 0);
								writeBigShort(relocated, highBits);
								writeBigByte(relocated, lowByte);
								break;
							case (uint8_t)RelocationType::R_PPC_REL14:
								relocated.seekg(destinationAddress, std::fstream::beg);

								instructionToSymbolOffset = (uint32_t)(sourceAddress - relocationsPosition);

								lowBits = (uint16_t)(instructionToSymbolOffset & 0x3FFF);

								writeBigShort(relocated, 0);
								writeBigShort(relocated, lowBits);
								break;
							case (uint8_t)RelocationType::R_DOLPHIN_NOP:
								// Do nothing
								break;
							case (uint8_t)RelocationType::R_DOLPHIN_SECTION:
								currentDestinationSectionID = relTableSource.sectionIndex;
								currentDestinationOffset = 0;
								break;
							case (uint8_t)RelocationType::R_DOLPHIN_END:

								break;
							}
						}
						++numRelocations;
						if (numRelocations % 5000 == 0) {
							//std::cout << "Completed " << numRelocations << " Relocations" << std::endl;
						}
					} while (relTableSource.relocationType != (uint8_t)RelocationType::R_DOLPHIN_END);
				}
			}

		}
	};
}