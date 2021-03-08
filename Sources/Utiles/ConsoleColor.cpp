#include "ConsoleColor.h"

//	Aroud message
std::string ConsoleColor::print(std::string& msg, const Color& color)
{
	return std::string();
}
//

//	Into stream
std::ostream& ConsoleColor::red(std::ostream& stream) { stream << getColorString(RED); return stream; }
std::ostream& ConsoleColor::green(std::ostream& stream) { stream << getColorString(GREEN); return stream; }
std::ostream& ConsoleColor::yellow(std::ostream& stream) { stream << getColorString(YELLOW); return stream; }
std::ostream& ConsoleColor::blue(std::ostream& stream) { stream << getColorString(BLUE); return stream; }
std::ostream& ConsoleColor::magenta(std::ostream& stream) { stream << getColorString(MAGENTA); return stream; }
std::ostream& ConsoleColor::cyan(std::ostream& stream) { stream << getColorString(CYAN); return stream; }
std::ostream& ConsoleColor::white(std::ostream& stream) { stream << getColorString(WHITE); return stream; }
std::ostream& ConsoleColor::classic(std::ostream& stream) { stream << getColorString(CLASSIC); return stream; }
//

//	Into string
std::string ConsoleColor::getColorString(const Color& color)
{
	switch (color)
	{
		#if defined(GF_OS_WINDOWS)
			case RED:		return "";
			case GREEN:		return "";
			case YELLOW:	return "";
			case BLUE:		return "";
			case MAGENTA:	return "";
			case CYAN:		return "";
			case WHITE:		return "";
			default:		return "";
		#elif defined(GF_OS_MACOS) || defined(GF_OS_LINUX)
			case RED:		return "\033[31m";
			case GREEN:		return "\033[32m";
			case YELLOW:	return "\033[33m";
			case BLUE:		return "\033[34m";
			case MAGENTA:	return "\033[35m";
			case CYAN:		return "\033[36m";
			case WHITE:		return "\033[37m";
			default:		return "";
		#endif // OS

	}
}
//


