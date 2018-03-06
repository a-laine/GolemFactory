#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION

#pragma warning(push, 0)
#include "stb_image.h"
#pragma warning(pop)


uint8_t* ImageLoader::loadFromFile(std::string file,int& width,int& heigth, int& channel, Mode forceChannel)
{
    return stbi_load(file.c_str(),&width,&heigth,&channel,forceChannel);
}

void ImageLoader::freeImage(uint8_t* data)
{
    stbi_image_free(data);
}
