#pragma once
#include <fstream>

//#include <glm/glm.hpp>
#include "Math/TMath.h"

#include <Resources/AnimationClip.h>
#include <Utiles/Parser/Variant.h>

class AnimationSaver
{
	public:
		//	Public functions
		static void save(AnimationClip* animation, const std::string& resourcesPath, std::string fileName = "");
		//

	protected:
		//	Protected functions
		static bool epsilon(const float& a, const float& b, const float& e);
		static bool epsilon(const vec4f& a, const vec4f& b, const float& e);
		static bool epsilon(const quatf& a, const quatf& b, const float& e);
		//
};