#pragma once

#include "System.h"

#include <ostream>
#include <string>


namespace ConsoleColor
{
	//	Miscelleneous
	enum class Color : int
	{
		RED = 0,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE,
		CLASSIC
	};
	//

	//	Into stream
	std::ostream& red(std::ostream& stream);
	std::ostream& green(std::ostream& stream);
	std::ostream& yellow(std::ostream& stream);
	std::ostream& blue(std::ostream& stream);
	std::ostream& magenta(std::ostream& stream);
	std::ostream& cyan(std::ostream& stream);
	std::ostream& white(std::ostream& stream);
	std::ostream& classic(std::ostream& stream);
	//

	//	Into string
	std::string getColorString(const Color& color);
	//
}