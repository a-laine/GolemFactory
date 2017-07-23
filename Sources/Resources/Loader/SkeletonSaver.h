#pragma once

#include <fstream>

#include <glm/glm.hpp>

#include "../Skeleton.h"
#include "Utiles/Parser/Variant.h"

class SkeletonSaver
{
	public:
		//  Default
		SkeletonSaver();
		~SkeletonSaver();
		//

		//	Public functions
		void save(Skeleton* skeleton, const std::string& resourcesPath, std::string fileName = "");
		//

	protected:
		//	Attributes
		Variant* rootVariant;
		//
};