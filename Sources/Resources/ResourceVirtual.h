#pragma once

#include <string>
#include <iostream>
#include <atomic>

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
            FONT = 6        //!< Font
        };
        //

        //  Default
        ResourceVirtual(std::string path,std::string resourceName = "unknown",ResourceType resourceType = NONE);
        virtual ~ResourceVirtual();

        virtual bool isValid() const;
        //

        //  Attributes
        std::string name;       //!< The resource name.
        ResourceType type;      //!< The resource type.
        //

    protected:
        // Attributes
        std::atomic_uint count; //!< The number of clients using the resource.
        //
};
