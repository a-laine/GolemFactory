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
		static bool epsilon(const float& a, const float& b, const float& e);
		static bool epsilon(const glm::vec3& a, const glm::vec3& b, const float& e);
		static bool epsilon(const glm::fquat& a, const glm::fquat& b, const float& e);
		//
};