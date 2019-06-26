#pragma once

#include <glm/gtx/component_wise.hpp>

#include "Scene/SceneManager.h"



class VirtualSceneQuerry
{
	public:
		virtual SceneManager::CollisionType operator() (const NodeVirtual* node) { return SceneManager::NONE; };
		std::vector<const NodeVirtual*>& getResult() { return result; }

	private:
		std::vector<const NodeVirtual*> result;
};
