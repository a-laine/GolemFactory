#pragma once

#include <glm/gtx/component_wise.hpp>

#include "NodeVirtual.h"

class VirtualSceneQuerry
{
	public:
		//	Miscellaneous
		enum CollisionType
		{
			NONE = 0, //!< No collision
			INSIDE,   //!< Object fully inside
			OVERLAP   //!< Shapes are overlapping
		};
		//

		virtual VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node);
		std::vector<const NodeVirtual*>& getResult();

	protected:
		std::vector<const NodeVirtual*> result;
};
