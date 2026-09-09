#include <fstream>
#include "Core/Assert.h"

#include "FileUtils.h"

namespace Vulcanite {
	bool ReadFile(const std::string& fileName, std::vector<char>& fileCont) {
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			return false;
		}
		
		size_t fileSize = static_cast<size_t>(file.tellg());
		fileCont.resize(fileSize);

		file.seekg(0);
		file.read(fileCont.data(), fileSize);

		file.close();

		return true;
	}
}