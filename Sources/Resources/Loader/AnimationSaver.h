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

	protected:
		//	Protected functions
		static bool epsilon(glm::vec3 a, glm::vec3 b, float e);
		static bool epsilon(glm::fquat a, glm::fquat b, float e);
		//
};