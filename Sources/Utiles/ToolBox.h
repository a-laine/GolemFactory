#pragma once

#include <string>
#include <fstream>
#include <cctype>
#include <sys/stat.h>


class ToolBox
{
	public:
		static std::string openAndCleanCStyleFile(std::string targetFile, std::string commentBlockEntry = "/*", std::string commentLineEntry = "//");
		static void clearWhitespace(std::string& input);
		static bool isPathExist(const std::string& fileName);
};
