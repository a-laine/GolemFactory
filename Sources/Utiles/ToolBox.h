#pragma once

#include <string>
#include <fstream>
#include <cctype>


class ToolBox
{
	public:
		static std::string openAndCleanCStyleFile(std::string targetFile, std::string commentBlockEntry = "/*", std::string commentLineEntry = "//");
		static void clearWhitespace(std::string& input);
};
