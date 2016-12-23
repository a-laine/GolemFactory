#pragma once

/*!
 *	\file ResourceVirtual.h
 *	\brief Declaration of the ResourceVirtual class.
 *	\author Thibault LAINE
 */

#include <string>
#include <iostream>
#include <atomic>
#include <algorithm>
#include <fstream>

/** \class ResourceVirtual
 *	\brief Base class for resource implementation.
 *
 *	The ResourceManager manage ResourceVirtual object for resource creation deletion.
 *	A resource is a shared object so the ResourceVirtual class implemant a system to trace the number of users.
 *	If this number reach 0 the ResourceManager delete the resource but not before.
 *	This is a virtual class so Resources is inherited from this class
 *
 */
class ResourceVirtual
{
    friend class ResourceManager;

    public:
        //  Miscellaneous
		/*!
		 *	\enum ResourceType
		 *	\brief The type of the resources
		 */
        enum ResourceType
        {
            NONE = 0,       //!< Virtual
            TEXTURE = 1,    //!< Texture
            SHADER = 2,     //!< Shader
            MESH = 3,       //!< Mesh
            SOUND = 4,      //!< Sound and music
            ANIMATION = 5,  //!< Animation
            FONT = 6,       //!< Font
			SKELETON = 7	//!< Skeleton
        };
        //

        //  Default
		/*!
		 *  \brief Constructor
		 *  \param resourceName : the resource name
		 *	\param resourceType : the resource type
		 */
        ResourceVirtual(std::string resourceName = "unknown", ResourceType resourceType = NONE);

		/*!
		 *  \brief Constructor
		 *  \param resourceType : the resource type
		 */
		ResourceVirtual(ResourceType resourceType = NONE);

		/*!
		 *  \brief Destructor
		 */
        virtual ~ResourceVirtual();

		/*!
		*	\brief Function to check if the resource was successfully loaded.
		*
		*	This is a function that need to be overloaded in herited class for proper use.
		*
		*	\return always true (actualy it's a virtual class !)
		*/
        virtual bool isValid() const;
        //

        //  Attributes
        std::string name;       //!< The resource name.
        ResourceType type;      //!< The resource type.
        //

    protected:
        //	Attributes
        std::atomic_uint count; //!< The number of clients using the resource.
        //

		//	Protected functions
		std::string openAndCleanCStyleFile(std::string fileName, std::string commentBlockEntry = "/*", std::string commentLineEntry = "//");
		//
};
