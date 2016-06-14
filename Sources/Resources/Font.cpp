#include "Font.h"
#include "Utiles/Parser/Reader.h"

#include "Loader/ImageLoader.h"

//  Static attributes
std::string Font::extension = ".Font";
//

//  Default
Font::Font(std::string path,std::string fontName) : ResourceVirtual(path,fontName,ResourceVirtual::FONT)
{
    //  Initialization
    texture = 0;
    charTable = nullptr;
    Variant v;
    Variant* tmp = NULL;

    try { Reader::parseFile(v, path + fontName + extension);
          tmp = &(v.getMap().begin()->second);                 }
    catch(std::exception& e) {std::cout<<"\tfail load file: "<<path + fontName + extension<<std::endl;return;}
    Variant& fontInfo = *tmp;

    std::string textureFile;
    try { textureFile = path + fontInfo["texture"].toString(); }
    catch(std::exception& e) {std::cout<<"\tfail extract texture name"<<std::endl;return;}

    //  Loading the image
    int x,y,n;
    uint8_t* image = ImageLoader::loadFromFile(textureFile,x,y,n,ImageLoader::RGB_ALPHA);
    if(!image) {std::cout<<"\tfail loading font texture image"<<std::endl;return;}

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
    if(!glIsTexture(texture)) {std::cout<<"\tfail load texture"<<std::endl;return;}

    //  Read file header
    std::string arrayFile;
    try { arrayFile = path + fontInfo["font"].toString(); }
    catch(std::exception& e) {clear(); std::cout<<"\tfail extract array char"<<std::endl;return;}

    std::ifstream file(arrayFile.c_str(), std::ios::in);
	if(!file){clear(); std::cout<<"\tfail load array char file"<<std::endl;return;}

	//  Parse info & allocate array
    size.x = x; size.y = y;

	try { begin = fontInfo["begin"].toInt(); }
    catch(std::exception& e) { begin = 0; }

    try { end = fontInfo["end"].toInt(); }
    catch(std::exception& e) { end = 512; }

    try { defaultChar = fontInfo["default"].toInt(); }
    catch(std::exception& e) { defaultChar = begin; }
    if(defaultChar>end) defaultChar = begin;

	charTable = new Patch[end-begin+1];
	if(!charTable){clear(); std::cout<<"\tfail init patch array"<<std::endl;return;}

	//  Load coordinate array
	std::string line;
	unsigned short int i;
	glm::u16vec2 c1,c2;
	while(getline(file, line))
    {
	    if(sscanf(line.c_str(), "%hu = {%hu, %hu, %hu, %hu}",&i,&c1.x,&c1.y,&c2.x,&c2.y)== 5)
        {
            charTable[i-begin].corner1.x = c1.x/size.x;
            charTable[i-begin].corner1.y = c1.y/size.y;
            charTable[i-begin].corner2.x = c2.x/size.x;
            charTable[i-begin].corner2.y = c2.y/size.y;
        }
        else {clear(); std::cout<<"\terror extracting patch line: "<<line<<std::endl;return;}
    }
}
Font::~Font()
{
    if(glIsTexture(texture)) glDeleteTextures(1,&texture);
    if(charTable) delete[] charTable;
}


bool Font::isValid() const
{
    return (bool)glIsTexture(texture);
}
//

//  Public functions
Font::Patch Font::getPatch(char c)
{
    if(!glIsTexture(texture)) return Patch();
    else if(c>end || c<begin) return charTable[defaultChar-begin];
    else return charTable[c-begin];
}
//

//  Set/get functions
char Font::getDefaultChar(){return (char)defaultChar;}
char Font::getBeginChar(){return (char)begin;}
char Font::getEndChar(){return (char)end;}
int Font::getArraySize(){return end-begin+1;}
//

//  Private functions
void Font::clear()
{
    if(glIsTexture(texture)) glDeleteTextures(1,&texture);
    texture = 0;
    if(charTable) delete[] charTable;
    charTable = nullptr;
}
//
