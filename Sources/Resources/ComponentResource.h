#pragma once

#include "EntityComponent\Component.hpp"
#include "Resources\ResourceManager.h"

template<class T>
class ComponentResource : public Component
{
	GF_DECLARE_COMPONENT_CLASS()
	public:
		//  Default
		ComponentResource(T* res)
		{

		};
		virtual ~ComponentResource() override
		{
			ResourceManager::getInstance()->release(resource);
		};
		//

		//	Set / Get function
		T* getResource() { return resource; };
		//

	protected:
		//	Attributes
		T* resource;
		//
};
