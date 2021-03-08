#pragma once

#include <fstream>

#include <glm/glm.hpp>

#include <Resources/Skeleton.h>
#include <Utiles/Parser/Variant.h>

class SkeletonSaver
{
	public:
		//	Public functions
		static void save(Skeleton* skeleton, const std::string& resourcesPath, std::string fileName = "");
		//
};