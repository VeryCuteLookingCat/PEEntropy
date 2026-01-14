# PE Entropy

![License](https://img.shields.io/badge/license-MIT-green)

A **PE file entropy visualization tool** for Windows, designed for malware analysts, reverse engineers, and anyone who wants an intuitive look at entropy patterns in binaries. 

Built using **ImGui** with the **DirectX 11 backend**.

---

## Features

- **PE section scanning** – automatically detects and lists all sections.  
- **Per-block entropy calculation** – adjustable block sizes for fine, balanced, smooth, or coarse analysis.  
- **All-sections mode** – visualize entropy for the entire binary in a single graph.  
- **Interactive graphs** – hover for block details (RVA, VA, size, entropy).  
- **Dark/Light theme toggle** and optional console output for logging.

## Visual Studio Runtime Required
- If the program fails to launch, install the Microsoft Visual C++ Redistributable (x64):
https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist

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
