#pragma once

#include <string>
#include <fstream>
#include <cctype>
#include <sys/stat.h>
#include <iomanip>

class ToolBox
{
	public:
		static std::string openAndCleanCStyleFile(std::string targetFile, std::string commentBlockEntry = "/*", std::string commentLineEntry = "//");
		static void clearWhitespace(std::string& input);
		static bool isPathExist(const std::string& fileName);

		template <typename T> static std::string to_string_with_precision(const T& a_value, const int& n = 2)
		{
			std::ostringstream out;
			out << std::setprecision(n) << a_value;
			return out.str();
		}
};
