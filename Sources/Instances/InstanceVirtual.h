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
		/*!
		*	\enum InstanceType
		*	\brief The type of the resources
		*/
		enum InstanceType
		{
			NONE = 0,       //!< Virtual
			CONTAINER = 1,  //!< Texture
			DRAWABLE = 2    //!< Shader
		};

		//  Default
		/*!
		 *  \brief Constructor
		 *  
		 *  Instance position is set to zero, size to one, and orientation matrix to identity
		 */
		InstanceVirtual(InstanceType instanceType = NONE);

		/*!
		 *  \brief Destructor
		 */
		virtual ~InstanceVirtual();
		//

		//	Set/Get functions
		InstanceType getType() const;


		/*!
		 *  \brief Define instance position
		 *  \param p : the new instance position
		 */
		void setPosition(glm::vec3 p);

		/*!
		 *  \brief Define instance size
		 *  \param s : the new instance size.
		 *  
		 *  It's possible to have diferent size factor on world axis.
		 *  read more about opengl size factor or matrix for more infos
		 */
		void setSize(glm::vec3 s);

		/*!
		 *  \brief Define instance orientation
		 *  \param m : the new instance orientation matrix
		 *
		 *  This is a 4*4 matrix. To compute it correctly see the glm::orientation functions or other.
		 */
		void setOrientation(glm::mat4 m);

		/*!
		 *  \brief Get instance position
		 *  \return the instance position
		 */
		glm::vec3 getPosition() const;

		/*!
		 *  \brief Get instance size
		 *  \return the instance size vector
		 */
		glm::vec3 getSize() const;

		/*!
		 *  \brief Get instance orientation
		 *  \return the instance orientation matrix
		 */
		glm::mat4 getOrientation() const;

		/*!
		 *  \brief Get instance model view matrix
		 *  \return the instance model view matrix
		 *
		 *  The matrix is computed whith the actual instance position, size and orientation
		 */
		glm::mat4 getModelMatrix() const;

		/*!
		 *  \brief Get instance bounding box
		 *  \return the instance bounding box
		 *
		 *  This is a virtual function, so you have to implement it depending on your specific instance.
		 *  Actually the InstanceVirtual::getBBSize function return always a zero vector (a point object !).
		 */
		virtual glm::vec3 getBBSize();

		/*!
		*  \brief Get instance bounding sphere radius
		*  \return the instance bounding sphere radius
		*
		*  This is a virtual function, so you have to implement it depending on your specific instance.
		*  Actually the InstanceVirtual::getBSRadius function return always zero (a point object !).
		*/
		virtual float getBSRadius();

		virtual Shader* getShader() const;
		virtual Animation* getAnimation() const;
		virtual Skeleton* getSkeleton() const;
		virtual Mesh* getMesh() const;
		virtual const std::list<InstanceVirtual*>& getChildList() const;
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
