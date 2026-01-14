#include "EntropyEngine.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

void EntropyEngine::browseForFile()
{
    OPENFILENAMEW ofn{};
    wchar_t szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"All Files\0*.*\0Executable Files\0*.exe\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (!GetOpenFileNameW(&ofn)) return;

    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0) {
        std::cerr << "[!] Error determining buffer size: " << GetLastError() << std::endl;
        return;
    }

    char charArray[256];
    if (WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, -1, charArray, bufferSize, nullptr, nullptr) == 0) {
        std::cerr << "[!] Error converting string: " << GetLastError() << std::endl;
        return;
    }

    strcpy_s(Settings::fileDirectoryBuf, sizeof(Settings::fileDirectoryBuf), charArray);
    Settings::fileDirectory = std::string(Settings::fileDirectoryBuf);

    std::cout << "[?] File Directory: " << Settings::fileDirectory << std::endl;
    std::cout << "[?] File DirectoryBuf: " << Settings::fileDirectoryBuf << std::endl;

    scan();
}

void EntropyEngine::scan()
{
    auto start = std::chrono::steady_clock::now();
    const auto& path = Settings::fileDirectory;
    if (path.empty()) { Variables::errorType = 1; return; }

    std::cout << "[?] Starting Scan..." << std::endl;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        Variables::errorType = 2;
        std::cerr << "[!] Failed to open binary: " << GetLastError() << std::endl;
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    Variables::fileContents.resize(size);
    if (!file.read(reinterpret_cast<char*>(Variables::fileContents.data()), size)) {
        Variables::fileContents.clear();
        Variables::errorType = 3;
        std::cerr << "[!] Failed to read binary: " << GetLastError() << std::endl;
        return;
    }

    Variables::sections.clear();
    Variables::sectionCache.clear();
    Variables::selectedSectionIndex = -1;
    Variables::imageBase = -1;

    auto* base = Variables::fileContents.data();
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        Variables::errorType = 4;
        std::cerr << "[!] Binary is not a valid PE file" << std::endl;
        return;
    }

    auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        Variables::errorType = 5;
        std::cerr << "[!] Binary has an invalid PE Signature" << std::endl;
        return;
    }

    Variables::imageBase = nt->OptionalHeader.ImageBase;
    std::cout << "[?] Image Base: " << std::hex << Variables::imageBase << std::endl;

    auto* sec = IMAGE_FIRST_SECTION(nt);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
        PESection s;
        s.name = std::string(reinterpret_cast<char*>(sec->Name), strnlen(reinterpret_cast<char*>(sec->Name), 8));
        s.offset = sec->PointerToRawData;
        s.virtualAddress = sec->VirtualAddress;
        s.size = sec->SizeOfRawData;
        Variables::sections.push_back(s);
    }

    std::cout << "[+] Scan Complete!" << std::endl;
    std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "Scan time: " << elapsed.count() << " seconds" << std::endl;

    Variables::errorType = 99;
}

void EntropyEngine::drawError()
{
    ImVec4 yellow{ 1.f, 1.f, 0.f, 1.f };
    ImVec4 red{ 1.f, 0.f, 0.f, 1.f };

    switch (Variables::errorType) {
    case -1: ImGui::TextColored(yellow, "Scan A binary first."); break;
    case 1:  ImGui::TextColored(yellow, "Please select a binary First."); break;
    case 2:  ImGui::TextColored(red, "Failed to open binary. ( Last Error: %u )", GetLastError()); break;
    case 3:  ImGui::TextColored(red, "Failed to read binary. ( Last Error: %u )", GetLastError()); break;
    case 4:  ImGui::TextColored(red, "Binary is not a valid PE file."); break;
    case 5:  ImGui::TextColored(red, "Binary has an invalid PE signature"); break;
    }
}

std::string EntropyEngine::selectedSectionName()
{
    if (Variables::selectedSectionIndex == -1) return "";
    if (Variables::selectedSectionIndex == -2) return "All Sections";
    auto& sec = Variables::sections[Variables::selectedSectionIndex];
    return sec.name.empty() ? "" : sec.name;
}

double EntropyEngine::computeEntropy(const uint8_t* data, size_t size)
{
    if (!data || size == 0) return 0.0;

    uint32_t counts[256]{};
    for (size_t i = 0; i < size; ++i) counts[data[i]]++;

    double entropy = 0.0;
    for (uint32_t c : counts) {
        if (!c) continue;
        double p = static_cast<double>(c) / size;
        entropy -= p * log2(p);
    }
    return entropy;
}

SectionEntropyData EntropyEngine::computeSectionEntropy(uint8_t* data, uint32_t sectionRva, uint32_t sectionSize, uint32_t blockSize)
{
    auto start = std::chrono::steady_clock::now();
    SectionEntropyData result{};
    result.sectionRva = sectionRva;
    result.sectionSize = sectionSize;
    result.blockSize = blockSize;

    if (!data || sectionSize == 0 || blockSize == 0) return result;

    uint32_t offset = 0;
    while (offset < sectionSize) {
        uint32_t remaining = sectionSize - offset;
        uint32_t currentBlockSize = remaining < blockSize ? remaining : blockSize;

        EntropyBlock block{};
        block.rva = sectionRva + offset;
        block.size = currentBlockSize;
        block.entropy = computeEntropy(data + offset, currentBlockSize);

        result.blocks.push_back(block);
        offset += currentBlockSize;
    }

    std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
    std::cout << "[?] Entropy Section Calculation (RVA:" << sectionRva << ", Size:" << sectionSize << ", Block Size: " << Settings::blockSize << "B ) time: " << elapsed.count() << " seconds" << std::endl;

    return result;
}

SectionEntropyData EntropyEngine::getCurrentSectionData()
{
    if (Variables::selectedSectionIndex == -1) return {};

    auto& currentSection = Variables::sections[Variables::selectedSectionIndex];
    if (currentSection.offset + currentSection.size > Variables::fileContents.size()) return {};

    auto& sectionCache = Variables::sectionCache[currentSection.name];
    auto it = sectionCache.find(Settings::blockSize);
    if (it != sectionCache.end() && !it->second.blocks.empty()) return it->second;

    auto entropyData = computeSectionEntropy(
        Variables::fileContents.data() + currentSection.offset,
        currentSection.virtualAddress,
        currentSection.size,
        Settings::blockSize
    );

    sectionCache[Settings::blockSize] = entropyData;
    return entropyData;
}
SectionEntropyData EntropyEngine::getAllSectionsEntropy()
{
    SectionEntropyData total{};
    if (Variables::sections.empty()) return total;

    uint64_t minRva = UINT64_MAX;
    uint64_t maxRva = 0;

    for (auto& sec : Variables::sections) {
        auto& sectionCache = Variables::sectionCache[sec.name];
        SectionEntropyData data;

        auto it = sectionCache.find(Settings::blockSize);
        if (it != sectionCache.end()) data = it->second;
        else {
            uint8_t* secData = Variables::fileContents.data() + sec.offset;
            data = computeSectionEntropy(secData, sec.virtualAddress, sec.size, Settings::blockSize);
            sectionCache[Settings::blockSize] = data;
        }

        // Merge blocks
        for (auto& blk : data.blocks) {
            total.blocks.push_back(blk);
            minRva = min(minRva, blk.rva);
            maxRva = max(maxRva, blk.rva + blk.size);
        }
    }

    total.sectionRva = minRva;
    total.sectionSize = maxRva - minRva;
    total.blockSize = Settings::blockSize;
    total.sectionName = "All Sections";

    return total;
}
