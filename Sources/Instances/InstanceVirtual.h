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
#include "EntityComponent/Component.hpp"

class World;


/*! \class InstanceVirtual
 *  \brief Base class for instance implementation.
 *
 *	The InstanceManager is THE instance container. All other game object manage instance pointers.
 *  An Instance is a shared resources and the actual implementation is not thread safe so be careful with usage.
 *  This is a virtual class, to implement a special instance prefere InstanceDrawable or InstanceContainer for your instance base (if possible).
 */
class InstanceVirtual
{
	friend class InstanceManager;

	public:
		//  Miscellaneous
		enum InstanceRenderingType
		{
			NONE = 0,       //!< 
			CONTAINER = 1,  //!< 
			DRAWABLE = 2,	//!< 
			ANIMATABLE = 3	//!< 
		};

		//  Default
		InstanceVirtual(/*InstanceType instanceType = NONE*/);
		virtual ~InstanceVirtual();
		//

		//	Component related
		void addComponent(Component* component, ClassID type);
		void removeComponent(Component* component);

		unsigned short getNbComponents() const;
		ClassID getTypeID(unsigned short index) const;
		Component* getComponent(unsigned short index);
		const Component* getComponent(unsigned short index) const;
		Component* getComponent(ClassID type);
		const Component* getComponent(ClassID type) const;
		void getAllComponents(ClassID type, std::vector<Component*>& components);
		void getAllComponents(ClassID type, std::vector<const Component*>& components) const;

		template<typename T> void addComponent(T* component) { addComponent(component, T::getStaticClassID()); }
		template<typename T> T* getComponent() { return static_cast<T*>(getComponent(T::getStaticClassID())); }
		template<typename T> const T* getComponent() const { return static_cast<const T*>(getComponent(T::getStaticClassID())); }
		template<typename T> void getAllComponents(std::vector<Component*>& components) { return getAllComponents(T::getStaticClassID(), components); }
		//

		//	Set/Get functions
		void setPosition(glm::vec3 p);
		void setSize(glm::vec3 s);
		void setOrientation(glm::mat4 m);
		void setParentWorld(World* parentWorld);

		uint32_t getId() const;
		glm::vec3 getPosition() const;
		glm::vec3 getSize() const;
		glm::mat4 getOrientation() const;
		glm::mat4 getModelMatrix();
		OrientedBox getBoundingVolume();
		World* getParentWorld() const;
		const InstanceRenderingType getRenderingType();
		const std::list<InstanceVirtual*>* getChildList() const;

		//InstanceType getType() const;
		
		//virtual glm::vec3 getBBMax() const;
		//virtual glm::vec3 getBBMin() const;
		//virtual float getBSRadius() const;

			
			/*virtual Shader* getShader() const;
			virtual Animation* getAnimation() const;
			virtual Skeleton* getSkeleton() const;
			virtual Mesh* getMesh() const;
			virtual std::vector<glm::mat4> getPose() const;*/

		//const std::list<InstanceVirtual*>* getChildList() const;
		//
		
		//uint32_t id;

	protected:
		//	Miscellaneous
		struct Element
		{
			Component* comp;
			ClassID type;
		};
		//

		// Attributes
		std::vector<Element> componentList;
		uint32_t id;				//!< The unique identifying number to design instance
		std::atomic_uint count;		//!< The number of clients pointing the instance.
		
		glm::vec3 position;			//!< Instance position
		glm::vec3 size;				//!< Instance size (or scale) factor
		glm::mat4 orientation;		//!< Instance orientation

		World* world;

		bool modelMatrixNeedUpdate;
		glm::mat4 model;
		//
};
