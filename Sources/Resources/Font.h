#pragma once

#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ResourceVirtual.h"

class Font : public ResourceVirtual
{
    public:
        //  Miscellaneous
        struct Patch{
            glm::vec2 corner1; //!< Upper left char corner coordinates.
			glm::vec2 corner2; //!< Bottom right char corner coordinates.
        };
        //

        //  Default
        Font(std::string path,std::string fontName);
        ~Font();

        bool isValid() const;
        //

        //  Set/get functions
        char getDefaultChar();
        char getBeginChar();
        char getEndChar();
        int getArraySize();
        //

        //  Public functions
        Patch getPatch(char c);
        //

        //  Attributes
        GLuint texture;                 //!< Texture Id
		glm::vec2 size;                //!< Texture size
        static std::string extension;   //!< Default extension used for infos files
        //

    private:
        //  Private functions
        void clear();                   //!< Clear if an error occur in initialization
        //

        //  Attributes
        unsigned short int begin,end,defaultChar;   //!< Some index info
        Patch* charTable;               //!< Coordinates of every char in texture
        //
};
