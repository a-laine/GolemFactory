#pragma once

#include "System.h"

#include <ostream>
#include <string>


namespace ConsoleColor
{
	//	Miscelleneous
	enum Color
	{
		RED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE,
		DEFAULT
	};
	//

	//	Aroud message
	std::string print(std::string& msg, const Color& color);
	//

	//	Into stream
	std::ostream& red(std::ostream& stream);
	std::ostream& green(std::ostream& stream);
	std::ostream& yellow(std::ostream& stream);
	std::ostream& blue(std::ostream& stream);
	std::ostream& magenta(std::ostream& stream);
	std::ostream& cyan(std::ostream& stream);
	std::ostream& white(std::ostream& stream);
	std::ostream& default(std::ostream& stream);
	//

	//	Into string
	std::string getColorString(const Color& color);
	//
}