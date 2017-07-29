#pragma once

/*!
 *	\file Font.h
 *	\brief Declaration of the Font resource class.
 *	\author Thibault LAINE
 */

#include <vector>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ResourceVirtual.h"

/*! \class Font
 *	\brief Font declaration.
 *
 *	The Font resource define an association of char and some texture coordinates.
 *	To draw a char just draw a textured quad with the returned texture coordinates.
 *
 */
class Font : public ResourceVirtual
{
    public:
        //  Miscellaneous
		/*!
		 *	\struct Patch
		 *	\brief The texture coordinate for a specified char
		 */
        struct Patch
		{
			Patch();
            glm::vec2 corner1;			//!< Upper left char corner coordinates.
			glm::vec2 corner2;			//!< Bottom right char corner coordinates.
        };
        //

        //  Default
		/*!
		 *  \brief Constructor
		 *  \param path : the directory location of the resource file
		 *	\param fontName : the resource name
		 */
        Font(const std::string& path, const std::string& fontName);

		/*!
		 *  \brief Destructor
		 */
        ~Font();

		/*!
		 *	\brief Function to check if the resource was successfully loaded.
		 *
		 *	This is a function is an overload of the herited function from ResourceVirtual class.
		 *	It's possible that the function return true even if the drawing is empty : 
		 *	actualy the function check just if texture was successfully loaded in OpenGl,
		 *	not if the char array association was succesfully loaded.
		 *
		 *	\return true if texture was successfully loaded in OpenGl
		 */
        bool isValid() const;
        //

        //  Set/get functions
		/*!
		 *	\brief Return default char defined in font.
		 *
		 *	The default char is used if user ask for a char out of range defined by begin and end char
		 *	
		 *
		 *	\return default char
		 */
        char getDefaultChar() const;
        char getBeginChar() const;
        char getEndChar() const;
        int getArraySize() const;
        //

        //  Public functions
        Patch getPatch(char c) const;
        //

        //  Attributes
        GLuint texture;                 //!< Texture Id
		glm::vec2 size;					//!< Texture size
        static std::string extension;   //!< Default extension used for infos files
        //

    private:
        //  Private functions
        void clear();                   //!< Clear if an error occur in initialization
        //

        //  Attributes
        unsigned short int begin,end,defaultChar;   //!< Some index info
		std::vector<Patch> charTable;               //!< Coordinates of every char in texture
        //
};
