#include "ConsoleColor.h"


#if defined(GF_OS_WINDOWS)
namespace ConsoleColor
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}
#endif // GF_OS_WINDOWS


//	Into stream
std::ostream& ConsoleColor::red(std::ostream& stream) { stream << getColorString(Color::RED); return stream; }
std::ostream& ConsoleColor::green(std::ostream& stream) { stream << getColorString(Color::GREEN); return stream; }
std::ostream& ConsoleColor::yellow(std::ostream& stream) { stream << getColorString(Color::YELLOW); return stream; }
std::ostream& ConsoleColor::blue(std::ostream& stream) { stream << getColorString(Color::BLUE); return stream; }
std::ostream& ConsoleColor::magenta(std::ostream& stream) { stream << getColorString(Color::MAGENTA); return stream; }
std::ostream& ConsoleColor::cyan(std::ostream& stream) { stream << getColorString(Color::CYAN); return stream; }
std::ostream& ConsoleColor::white(std::ostream& stream) { stream << getColorString(Color::WHITE); return stream; }
std::ostream& ConsoleColor::classic(std::ostream& stream) { stream << getColorString(Color::CLASSIC); return stream; }
//

//	Into string
std::string ConsoleColor::getColorString(const Color& color)
{
	switch (color)
	{
		#if defined(GF_OS_WINDOWS)
			case Color::RED:
				SetConsoleTextAttribute(ConsoleColor::hConsole, FOREGROUND_RED);
				return "";
			case Color::GREEN:
				SetConsoleTextAttribute(ConsoleColor::hConsole, FOREGROUND_GREEN);
				return "";
			case Color::YELLOW:
				SetConsoleTextAttribute(ConsoleColor::hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
				return "";
			case Color::BLUE:
				SetConsoleTextAttribute(ConsoleColor::hConsole, FOREGROUND_BLUE);
				return "";
			case Color::MAGENTA:
				SetConsoleTextAttribute(ConsoleColor::hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
				return "";
			case Color::CYAN:
				SetConsoleTextAttribute(ConsoleColor::hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
				return "";
			case Color::WHITE:
				SetConsoleTextAttribute(ConsoleColor::hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				return "";
			default:
				SetConsoleTextAttribute(ConsoleColor::hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				return "";

		#elif defined(GF_OS_MACOS) || defined(GF_OS_LINUX)
			case Color::RED:		return "\033[31m";
			case Color::GREEN:		return "\033[32m";
			case Color::YELLOW:	return "\033[33m";
			case Color::BLUE:		return "\033[34m";
			case Color::MAGENTA:	return "\033[35m";
			case Color::CYAN:		return "\033[36m";
			case Color::WHITE:		return "\033[37m";
			default:		return "\033[37m";

		#endif // OS
	}
}
//


