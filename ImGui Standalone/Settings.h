#pragma once
#include <string>

namespace Settings {
	inline bool darkMode = true;
	inline char fileDirectoryBuf[256];
	inline std::string fileDirectory;
	inline int blockSize = 128;
	inline bool showConsole = false;

}