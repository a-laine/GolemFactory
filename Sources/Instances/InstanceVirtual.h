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
	friend class InstanceContainer;

	public:
		//  Default
		InstanceVirtual();
		virtual ~InstanceVirtual();
		//

		//	Set/Get functions
		void setPosition(glm::vec3 p);
		void setSize(glm::vec3 s);
		void setOrientation(glm::mat4 m);

		glm::vec3 getPosition() const;
		glm::vec3 getSize() const;
		glm::mat4 getOrientation() const;

		glm::mat4 getModelMatrix() const;
		virtual glm::vec3 getBBSize();
		//
		
	protected:
		// Attributes
		uint32_t id;
		std::atomic_uint count;		//!< The number of clients pointing the instance.
		
		glm::vec3 position, size;
		glm::mat4 orientation;
		//
};
