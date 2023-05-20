#pragma once

#include <string>
#include <iostream>
#include <atomic>
#include <algorithm>
#include <fstream>

#include <Utiles/ImguiConfig.h>

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
		enum class VerboseLevel
		{
            NONE = 0,
			ERRORS = 1,		//!< Just print errors in log
			WARNINGS = 2,	//!< Print errors and logs
			ALL = 3			//!< Print all logs (errors, warning and optionnal informations)
		};
		enum class ResourceType
		{
            NONE = 0,
            MESH,
            MATERIAL,
            TEXTURE,
            SHADER,
            SKELETON,
            ANIMATION,
            FONT
		};
		//

        //  Default
        ResourceVirtual(const std::string& resourceName = "unknown", ResourceType resourceType = ResourceType::NONE);
		ResourceVirtual();
	    virtual ~ResourceVirtual();

        bool isValid() const;
        ResourceType getType() const { return type; };
        virtual std::string getIdentifier() const;
        virtual void assign(const ResourceVirtual* other);
        //

        //  Attributes
		static VerboseLevel logVerboseLevel;	//!< The verbose level define for all resources
        std::string name;						//!< The resource name.
        //

        virtual void onDrawImGui();

        static void printErrorLog(const std::string& resourceName, const int& errorLine, bool& printHeader);

    protected:
        enum State
        {
            INVALID,
            LOADING,
            VALID
        };
        //	Attributes
        std::atomic_uint count;					//!< The number of clients using the resource.
        std::atomic<State> state;
        ResourceType type = ResourceType::NONE;
        //
};
