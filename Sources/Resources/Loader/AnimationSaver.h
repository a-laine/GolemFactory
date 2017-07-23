#pragma once
#include <fstream>

#include <glm/glm.hpp>

#include "../Animation.h"
#include "Utiles/Parser/Variant.h"

class AnimationSaver
{
	public:
		//	Public functions
		static void save(Animation* animation, const std::string& resourcesPath, std::string fileName = "");
		//
};