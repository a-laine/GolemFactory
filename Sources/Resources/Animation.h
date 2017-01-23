#pragma once

#include <vector>
#include <fstream>
#include <map>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Joint.h"


class Animation : public ResourceVirtual
{
    friend class ResourceManager;

    public:
        //  Default
		Animation(const std::string& animationName, const std::vector<KeyFrame>& animations);
		Animation(const std::string& path, const std::string& animationName);
        ~Animation();

        bool isValid() const;
        //

		//	Attributes
		static std::string extension;   //!< Default extension
		//

    protected:
        //	Attributes
		std::vector<KeyFrame> timeLine;
		//
};
