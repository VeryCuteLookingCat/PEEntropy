# PE Entropy

![License](https://img.shields.io/badge/license-MIT-green)

A **PE file entropy visualization tool** for Windows, designed for malware analysts, reverse engineers, and anyone who wants an intuitive look at entropy patterns in binaries. 

Built using **ImGui** with the **DirectX 11 backend**.

---

## Features
- **PE section scanning** – automatically detects and lists all sections.  
- **Per-block entropy analysis** – configurable block sizes for fine → coarse inspection.  
- **All-sections mode** – visualize entropy across the entire binary in a single graph.  
- **Interactive entropy graph**
  - Hover blocks to inspect **RVA, VA, entropy, and size**
  - **Click blocks to lock selection**
- **Copy-paste ready addresses**
  - Displays **IDA/Ghidra-ready RVA and VA**
  - One-click selection → paste → jump to code/data instantly
- **Dark / Light theme toggle**
- **Optional console output** for debugging and scan logs

## Visual Studio Runtime Required
- If the program fails to launch, install the Microsoft Visual C++ Redistributable (x64):
https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist

---

## Why This Tool Exists

Entropy alone is not useful unless you can **act on it**.

This tool is designed to:
- Quickly locate **packed payloads, encrypted blobs, and shellcode**
- Provide **precise RVAs and VAs** you can jump to immediately in:
  - **IDA**
  - **Ghidra**
- Reduce time wasted manually correlating entropy spikes with disassembly

The goal is **shortening the distance** between:
> *“This looks interesting”* → *“I’m reversing it.”*
---

## Screenshots
![Menu](https://raw.githubusercontent.com/VeryCuteLookingCat/PEEntropy/refs/heads/main/Images/Menu.png)


---

## Getting Started

1. Build the project using Visual Studio or your preferred IDE.  
2. Run `EntropyTool.exe`.  
3. Use the toolbar to select a PE file and scan its sections.  
4. Hover over the entropy graph to inspect individual blocks.  

---

## Usage Tips

- **Block size:** Smaller blocks = finer resolution; larger blocks = smoother graph.  
- **All sections:** Select “All Sections” at the top of the section list to visualize total entropy.  
- **Console output:** Enable the console via the toolbar to see scan progress and detailed information.  

---

## Credits

- Base ImGui integration inspired by [adamhlt/ImGui-Standalone](https://github.com/adamhlt/ImGui-Standalone) – thank you for providing a minimal ImGui foundation!  

---

## License

This project is licensed under the **GPLv3 License** – see [LICENSE](LICENSE) for details.  
