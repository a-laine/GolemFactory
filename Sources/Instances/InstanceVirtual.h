#pragma once

#include <iostream>
#include <atomic>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Resources/ResourceManager.h" 


class InstanceVirtual
{
	friend class InstanceManager;

	public:
		//  Default
		InstanceVirtual();
		virtual ~InstanceVirtual();
		//

		//	Set/Get functions
		void setPosition(glm::vec3 p);
		void setSize(glm::vec3 s);

		glm::vec3 getPosition();
		glm::vec3 getSize();
		//
		
	protected:
		// Attributes
		uint32_t id;
		glm::vec3 position, size;
		std::atomic_uint count; //!< The number of clients pointing the instance.
		//
};

