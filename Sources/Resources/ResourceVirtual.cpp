#include "ResourceVirtual.h"

//  Default
ResourceVirtual::ResourceVirtual(const std::string& resourceName, ResourceType resourceType) : name(resourceName),type(resourceType)
{
    count = 0;
}
ResourceVirtual::ResourceVirtual(ResourceType resourceType) : name("unknown"), type(resourceType)
{
	count = 0;
}
ResourceVirtual::~ResourceVirtual(){}
//

//  Public functions
bool ResourceVirtual::isValid() const
{
	return true;
}
//

//	Protected functions
std::string ResourceVirtual::openAndCleanCStyleFile(std::string fileName, std::string commentBlockEntry, std::string commentLineEntry)
{
	//	open file to parse
	std::string output;
	std::ifstream file(fileName);
	if (!file.good())
	{
		std::cerr << "ResourceVirtual : Fail to open file :" << std::endl;
		std::cerr << "                  " << fileName << std::endl;
		return output;
	}

	//	parsing parameters initialization
	unsigned int parsingCommentBlockIndex = 0;
	unsigned int parsingCommentLineIndex = 0;
	bool actuallyParsingCommentBlock = false;
	bool actuallyParsingCommentLine = false;
	char currentChar;

	//	parse file and remove all comment block and line
	while (file.get(currentChar))
	{
		if (actuallyParsingCommentBlock)
		{
			if (currentChar == commentBlockEntry[parsingCommentBlockIndex])
			{
				if (parsingCommentBlockIndex == 0)	//	end of parsing entry block backward so stop parsing block
				{
					//	reset comment entry parsing parameter
					parsingCommentBlockIndex = 0;
					parsingCommentLineIndex = 0;
					actuallyParsingCommentBlock = false;
					actuallyParsingCommentLine = false;
				}
				else parsingCommentBlockIndex--;	//	parsing backward for end of block detection
			}
			else parsingCommentBlockIndex = commentBlockEntry.size() - 1;	//	parsing block fail so reset parameter
		}
		else if (actuallyParsingCommentLine)
		{
			if (currentChar == '\n' || currentChar == '\r')	//	end line char detected so stop parsing comment line ('\r' and / or '\n' depending on platform)
			{
				//	reset comment entry parsing parameter
				parsingCommentBlockIndex = 0;
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = false;
				actuallyParsingCommentLine = false;

				//	push the end line char to keep a cooherence if using getline on the string after
				output.push_back(currentChar);
			}
		}
		else if (!commentBlockEntry.empty() && currentChar == commentBlockEntry[parsingCommentBlockIndex])
		{
			if (parsingCommentBlockIndex >= commentBlockEntry.size() - 1)	//	comment block entry string match entirely -> begin parsing a comment line
			{
				//	erase last pushed char coresponding to the comment block entry string
				output.erase(std::prev(output.end(), parsingCommentBlockIndex), output.end());

				//	set parameters for parsing a block
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = true;
				actuallyParsingCommentLine = false;
				continue;
			}
			else parsingCommentBlockIndex++;	//	parsing forward

			if (!commentLineEntry.empty() && currentChar == commentLineEntry[parsingCommentLineIndex])
			{
				if (parsingCommentLineIndex >= commentLineEntry.size() - 1)	//	comment line entry string match entirely -> begin parsing a comment line
				{
					//	erase last pushed char coresponding to the comment line entry string
					output.erase(std::prev(output.end(), parsingCommentLineIndex), output.end());

					//	set parameters for parsing a line
					parsingCommentBlockIndex = 0;
					parsingCommentLineIndex = 0;
					actuallyParsingCommentBlock = false;
					actuallyParsingCommentLine = true;
					continue;
				}
				else parsingCommentLineIndex++;	//	parsing forward
			}
			else parsingCommentLineIndex = 0;	//	parsing line entry fail so reset parameter

			output.push_back(currentChar);
		}
		else if (!commentLineEntry.empty() && currentChar == commentLineEntry[parsingCommentLineIndex])
		{
			parsingCommentBlockIndex = 0;	//	parsing block fail so reset parameter
			if (parsingCommentLineIndex >= commentLineEntry.size() - 1)	//	comment line entry string match entirely -> begin parsing a comment line
			{
				//	erase last pushed char coresponding to the comment line entry string
				output.erase(std::prev(output.end(), parsingCommentLineIndex), output.end());

				//	set parameters for parsing a line
				parsingCommentBlockIndex = 0;
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = false;
				actuallyParsingCommentLine = true;
				continue;
			}
			else parsingCommentLineIndex++;	//	parsing forward

			output.push_back(currentChar);	//	not begining parsing comment block or line so push current char to output string
		}
		else
		{
			//	all parsing test fail so reset parameters
			parsingCommentBlockIndex = 0;
			parsingCommentLineIndex = 0;
			actuallyParsingCommentBlock = false;
			actuallyParsingCommentLine = false;

			//	not begining parsing comment block or line so push current char to output string
			output.push_back(currentChar);
		}
	}

	//	end
	file.close();
	if (output.empty())
	{
		std::cerr << "ResourceVirtual : A file was sucessfully parsed for removing all comment but the result is an empty file !" << std::endl;
		std::cerr << "                  Please check this file for more details :" << std::endl;
		std::cerr << "                  " << fileName << std::endl;
	}
	return output;
}
