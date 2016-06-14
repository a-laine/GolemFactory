#pragma once

#include <string>
#include <cstdint>


class ImageLoader
{
    public:
        enum Mode {
            DEFAULT = 0, // only used for req_comp

            GREY       = 1,
            GREY_ALPHA = 2,
            RGB        = 3,
            RGB_ALPHA  = 4
        };
        static uint8_t* loadFromFile(std::string file,int& width,int& heigth, int& channel, Mode forceChannel);
        static void freeImage(uint8_t* data);
};
