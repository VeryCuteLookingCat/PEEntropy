#pragma once
#include "pch.h"
#include <vector>
#include <unordered_map>

struct PESection {
	std::string name;
	size_t offset;
	size_t virtualAddress;
	size_t size;
};
struct EntropyBlock {
	uint64_t rva;
	uint64_t size;
	double entropy;
};
struct SectionEntropyData {
	std::string sectionName;
	uint64_t sectionRva;
	uint64_t sectionSize;
	uint64_t blockSize;

	std::vector<EntropyBlock> blocks;
};

namespace EntropyEngine {
	namespace Variables {
		inline int errorType = -1;
		inline uint64_t imageBase = -1;
		inline std::vector<uint8_t> fileContents;
		inline std::vector<PESection> sections;
		inline int selectedSectionIndex = -1;
		inline std::unordered_map<
			std::string,
			std::unordered_map<uint32_t, SectionEntropyData>
		> sectionCache;

	}
	void browseForFile();
	void scan();
	void drawError();
	std::string selectedSectionName();
	double computeEntropy(const uint8_t* data, size_t size);
	SectionEntropyData computeSectionEntropy(uint8_t* data, uint32_t sectionRva, uint32_t sectionSize, uint32_t blockSize);
	SectionEntropyData getCurrentSectionData();
	SectionEntropyData getAllSectionsEntropy();
}