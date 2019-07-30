#pragma once

/*!
 *	\file Font.h
 *	\brief Declaration of the Font resource class.
 *	\author Thibault LAINE
 */

#include <vector>
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
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

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
		 *	\param fontName : the resource name
		 */
		explicit Font(const std::string& fontName);

		/*!
		 *  \brief Destructor
		 */
        ~Font();

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
        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;
        //

        //  Public functions
        void initialize(uint8_t* image, const glm::vec2& imageSize, unsigned short beginC, unsigned short endC, unsigned short defaultC, const std::vector<Patch>& table);
        void initialize(uint8_t* image, const glm::vec2& imageSize, unsigned short beginC, unsigned short endC, unsigned short defaultC, std::vector<Patch>&& table);
        Patch getPatch(char c) const;
        //

        //  Attributes
        GLuint texture;                 //!< Texture Id
		glm::vec2 size;					//!< Texture size
        //

    private:
        static std::string defaultName;

        //  Private functions
        bool initOpenGL(uint8_t* image, int sizeX, int sizeY);
        void clear();                   //!< Clear if an error occur in initialization
        //

        //  Attributes
        unsigned short int begin,end,defaultChar;   //!< Some index info
		std::vector<Patch> charTable;               //!< Coordinates of every char in texture
        //
};
