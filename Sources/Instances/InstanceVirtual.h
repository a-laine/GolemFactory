#pragma once

/*!
 *	\file InstanceVirtual.h
 *	\brief Declaration of the Instance class.
 *	\author Thibault LAINE 
 */

#include <iostream>
#include <atomic>
#include <list>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Resources/ResourceManager.h" 


/*! \class InstanceVirtual
 *  \brief Base class for instance implementation.
 *
 *	The InstanceManager is THE instance container. All other game object manage instance pointers.
 *  An Instance is a shared resources and the actual implementation is not thread safe so be careful with usage.
 *  This is a virtual class, to implement a special instance prefere InstanceDrawable or InstanceContainer for your instance base (if possible).
 *
 */
class InstanceVirtual
{
	friend class NodeVirtual;
	friend class InstanceManager;
	friend class InstanceContainer;

	public:
		//  Miscellaneous
		enum InstanceType
		{
			NONE = 0,       //!< 
			CONTAINER = 1,  //!< 
			DRAWABLE = 2,	//!< 
			ANIMATABLE = 3	//!< 
		};

		//  Default
		InstanceVirtual(InstanceType instanceType = NONE);
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
		
		InstanceType getType() const;
		uint32_t getId() const;
		virtual glm::vec3 getBBMax() const;
		virtual glm::vec3 getBBMin() const;
		virtual float getBSRadius() const;

		virtual Shader* getShader() const;
		virtual Animation* getAnimation() const;
		virtual Skeleton* getSkeleton() const;
		virtual Mesh* getMesh() const;
		virtual std::vector<glm::mat4> getPose() const;

		virtual const std::list<InstanceVirtual*>* getChildList() const;
		//
		
	protected:
		// Attributes
		InstanceType type;
		uint32_t id;				//!< The unique identifying number to design instance
		std::atomic_uint count;		//!< The number of clients pointing the instance.
		
		glm::vec3 position;			//!< Instance position
		glm::vec3 size;				//!< Instance size (or scale) factor
		glm::mat4 orientation;		//!< Instance orientation
		//
};
