#pragma once

#include <string>
#include <iostream>
#include <atomic>
#include <algorithm>
#include <fstream>

/*! \class ResourceVirtual
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
		enum VerboseLevel
		{
			ERRORS = 1,		//!< Just print errors in log
			WARNINGS = 2,	//!< Print errors and logs
			ALL = 3			//!< Print all logs (errors, warning and optionnal informations)
		};
		//

        //  Default
        ResourceVirtual(const std::string& resourceName = "unknown", ResourceType resourceType = NONE);
		ResourceVirtual(ResourceType resourceType = NONE);
	    virtual ~ResourceVirtual();

        virtual bool isValid() const;
        //

        //  Attributes
		static VerboseLevel logVerboseLevel;	//!< The verbose level define for all resources
        std::string name;						//!< The resource name.
        ResourceType type;						//!< The resource type.
        //

    protected:
        //	Attributes
        std::atomic_uint count;					//!< The number of clients using the resource.
        //

		//	Protected functions	
		void printErrorLog(const std::string& resourceName, const int& errorLine, bool& printHeader);
		//
};
