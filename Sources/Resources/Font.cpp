#include "Font.h"
#include "Utiles/Parser/Reader.h"
#include "Utiles/ToolBox.h"
#include "Loader/ImageLoader.h"

//  Static attributes
std::string Font::extension = ".font";
//

//  Default
Font::Font(const std::string& path, const std::string& fontName) : ResourceVirtual(fontName, ResourceVirtual::FONT)
{
    //  Initialization
    texture = 0;
    Variant v;
    Variant* tmp = NULL;

	//	Extract root variant from file
    try
	{
		Reader::parseFile(v, path + fontName + extension);
        tmp = &(v.getMap().begin()->second);
	}
    catch(std::exception&)
	{
		if (logVerboseLevel >= ResourceVirtual::ERRORS)
			std::cerr << "ERROR : loading font : " << fontName << " : fail to open or parse file" << std::endl;
		return;
	}
    Variant& fontInfo = *tmp;

	//	Extract texture name
    std::string textureFile;
    try
	{
		textureFile = path + fontInfo["texture"].toString();
	}
    catch(std::exception&) 
	{
		if (logVerboseLevel >= ResourceVirtual::ERRORS)
			std::cerr << "ERROR : loading font : " << fontName << " : texture attribute field not found" << std::endl;
		return;
	}

    //  Loading the image
    int x,y,n;
	uint8_t* image = ImageLoader::loadFromFile(textureFile, x, y, n, ImageLoader::RGB_ALPHA);
	if (!image)
	{
		if (logVerboseLevel >= ResourceVirtual::ERRORS)
			std::cerr << "ERROR : loading font : " << fontName << " : fail loading texture image" << std::endl;
		return;
	}

    //  Generate opengl texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D,0);
    ImageLoader::freeImage(image);
    if(!glIsTexture(texture))
	{
		if (logVerboseLevel >= ResourceVirtual::ERRORS)
			std::cerr << "ERROR : loading font : " << fontName << " : fail create OPENGL texture" << std::endl;
		return;
	}

	//  Parse patches infos
    size.x = (float)x; size.y = (float)y;

	try { begin = fontInfo["begin"].toInt(); }
    catch(std::exception&) { begin = 0; }

    try { end = fontInfo["end"].toInt(); }
    catch(std::exception&) { end = 512; }

    try { defaultChar = fontInfo["default"].toInt(); }
    catch(std::exception&) { defaultChar = begin; }
	if (defaultChar > end) defaultChar = begin;
	charTable.assign(end - begin + 1, Patch());

    //  Extract and prepare file array for parsing
    std::string arrayFile;
    try
	{
		arrayFile = ToolBox::openAndCleanCStyleFile(path + fontInfo["font"].toString());
	}
    catch(std::exception&)
	{
		if (logVerboseLevel >= ResourceVirtual::ERRORS)
			std::cerr << "ERROR : loading font : " << fontName << " : fail open char array" << std::endl;
		clear();
		return;
	}
	if(arrayFile.empty()) 
	{
		if (logVerboseLevel >= ResourceVirtual::WARNINGS)
			std::cerr << "WARNING : loading font : " << fontName << " : char array file empty" << std::endl;
		begin = 0;
		end = 0;
		defaultChar = 0;
	}



	//  Load coordinate array
	std::string line;
	glm::u16vec2 c1,c2;
	bool errorOccured = false;
	unsigned int index = 0;
	unsigned int lineCount = 0;
	std::stringstream arrayFileStream(arrayFile);
	while (!arrayFileStream.eof())
	{
		//	get next line
		std::getline(arrayFileStream, line);
		lineCount++;

		//	remove unrevelant char and replace coma by space
		for (auto it = line.begin(); it != line.end();)
		{
			if(*it == ' ' || *it == '=' || *it == '}')
				it = line.erase(it);
			else if (*it == ',' || *it == '{')
			{
				*it = ' ';
				it++;
			}
			else it++;
		}
		if (line.empty()) continue;

		//	parsing
		std::stringstream convertor(line);
		convertor >> index >> c1.x >> c1.y >> c2.x >> c2.y;

		if (!convertor.fail())
		{
			charTable[index - begin].corner1.x = c1.x / size.x;
			charTable[index - begin].corner1.y = c1.y / size.y;
			charTable[index - begin].corner2.x = c2.x / size.x;
			charTable[index - begin].corner2.y = c2.y / size.y;
		}
		else printErrorLog(fontName, lineCount, errorOccured);
	}
	charTable.shrink_to_fit();
}
Font::~Font()
{
    if(glIsTexture(texture)) glDeleteTextures(1,&texture);
}


bool Font::isValid() const
{
    return glIsTexture(texture) != 0;
}
//

//  Public functions
Font::Patch Font::getPatch(char c) const
{
	if (!glIsTexture(texture)) return Patch();
	else if (c > end || c < begin) return charTable[defaultChar - begin];
	else return charTable[c - begin];
}
//

//  Set/get functions
char Font::getDefaultChar() const { return (char)defaultChar; }
char Font::getBeginChar() const { return (char)begin; }
char Font::getEndChar() const { return (char)end; }
int Font::getArraySize() const { return charTable.size(); }
//

//  Private functions
void Font::clear()
{
	if (glIsTexture(texture))
		glDeleteTextures(1, &texture);
    texture = 0;
    charTable.clear();
}
//

//	Internal classes
Font::Patch::Patch() : corner1(0.f), corner2(0.f) {}
//
