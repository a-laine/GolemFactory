#pragma once

/*!
 *	\file InstanceManager.h
 *	\brief Declaration of the InstanceManager class.
 *	\author Thibault LAINE
 */

#include <atomic>
#include <algorithm>

#include "Utiles/Singleton.h"
#include "Utiles/Mutex.h"
#include "InstanceVirtual.h"
#include "InstanceDrawable.h"
#include "InstanceContainer.h"
#include "InstanceAnimatable.h"


 /*! \class InstanceManager
  *  \brief Instance container, and manager.
  *
  *	 The InstanceManager is THE instance container. All other game object use shared pointers.
  *  First job of the class is to give a unique id of each instance,
  *  Second one is to delete unused instances (move the to garbage and delete them when unsed by client),
  *  Third is to provide correctfully initialised InstanceDrawable, and InstanceContainer.
  *
  */
class InstanceManager : public Singleton<InstanceManager>
{
	friend class Singleton<InstanceManager>;

	public:
		//  Public functions
		/*!
		 *  \brief Add an instance to the container
		 *  \param ins : a pointer on the instance to add.
		 *  \return a valid pointer on the newly InstanceVirtual created.
		 */
		InstanceVirtual* add(InstanceVirtual* ins);

		/*!
		 *  \brief Demande a specific instance pointer from an id
		 *  \param id : the id defining the instance you want.
		 *  \return a valid pointer on the InstanceVirtual or null if id not related to an instance yet.
		 */
		InstanceVirtual* get(uint32_t id);

		/*!
		 *  \brief Specify you don't refer anymore to the instance (free it)
		 *  \param ins : the instance you release
		 */
		void release(InstanceVirtual* ins);

		/*!
		 *  \brief Empty the garbage
		 *
		 *  Calling the destructor of many class can be time consuming so this operation is deported to an different function than release.
		 *  Normally you can call this function from a different thread to gain performance.
		 */
		void clearGarbage();

		/*!
		 *  \brief Instanciate a new InstanceDrawable object
		 *  \param meshName : the mesh name for the instance. If not specified the default mesh is used.
		 *  \param shaderName : the shader name for the instance. If not specified the default shader is used.
		 *  \return a valid pointer on the newly InstanceDrawable created, or null an error occur.
		 *
		 *  The function return null if the instance allocation fail, or if the instance can't be added to container.
		 */
		InstanceDrawable* getInstanceDrawable(std::string meshName = "default", std::string shaderName = "default");

		/*!
		*  \brief Instanciate a new getInstanceAnimatable object
		*  \param meshName : the mesh name for the instance. If not specified the default mesh is used.
		*  \param shaderName : the shader name for the instance. If not specified the default shader is used.
		*  \return a valid pointer on the newly getInstanceAnimatable created, or null an error occur.
		*
		*  The function return null if the instance allocation fail, or if the instance can't be added to container.
		*/
		InstanceAnimatable* getInstanceAnimatable(std::string meshName = "default", std::string shaderName = "default");
		InstanceAnimatable* getInstanceAnimatable(std::string meshName, std::string skeletonName, std::string animationName, std::string shaderName);

		/*!
		 *  \brief Instanciate a new InstanceContainer object
		 *  \return a valid pointer on the newly InstanceContainer created, or null an error occur.
		 *
		 *  The function return null if the instance allocation fail, or if the instance can't be added to container.
		 */
		InstanceContainer* getInstanceContainer();
		//

		//  Set/get functions
		/*!
		 *  \brief Specify the instance limit count
		 *  \param maxIns : the new max size.
		 *
		 *  Change the maximun size of the container.
		 *  If reached the getInstanceXXX function will return null, as well as add function.
		 *  If the new maximum size is less than the actual instance count, the container size remain unchanged.
		 */
		void setMaxNumberOfInstances(unsigned int maxIns);

		/*!
		 *  \brief Change the reallocation size
		 *  \param size : the reallocation size.
		 *
		 *  As same as the sdt::vector class, if the container (dynamic array) is too small for instance count,
		 *  the container is reallocated to a new size incremented by a reallocator size.
		 */
		void setReallocSize(uint8_t size);

		/*!
		 *  \brief Reserve space in container
		 *  \param size : the size to reserve.
		 *
		 *  Read about reserve of std::vector for more infos
		 */
		void reserveInstanceSize(unsigned int size);

		/*!
		 *  \brief Get the actual number of instance present in container
		 *  \return number of instance
		 */
		unsigned int getNumberOfInstances() const;

		/*!
		 *  \brief Get the actual maximum number of instance allowed for the container
		 *  \return max number of instance
		 */
		unsigned int getMaxNumberOfInstances() const;

		/*!
		 *  \brief Get size of allocated storage capacity
		 *  \return size of the container before a reallocation happen
		 *
		 *  Read std::vector::capacity for more infos
		 */
		unsigned int getInstanceCapacity() const;

		/*!
		 *  ??? redondant with getInstanceCapacity because an unused slot is pretty equivalent to container capacity
		 */
		unsigned int getInstanceSlot() const;
		//


	private:
		//  Default
		/*!
		 *  \brief Constructor
		 *  \param maximum : the maximum of instance
		 */
		InstanceManager(unsigned int maximum = 20000);

		/*!
		 *  \brief Destructor
		 */
		~InstanceManager();
		//

		//  Private functions
		/*!
		 *  \brief Get a valid ID
		 *  \return a valid ID and complete summary (id not avalaible anymore)
		 */
		uint32_t getAvalibleId();

		/*!
		 *  \brief Release an ID
		 *  \param id : ID to release complete summary (id avalaible again)
		 */
		void freeId(uint32_t id);
		//

		//  Attributes
		Mutex mutexGarbage;                         //!< A mutex to prevent garbage collision.
		std::vector<InstanceVirtual*> garbage;      //!< The list of instances to delete.

		Mutex mutexList;                            //!< A mutex to prevent access collision.
		std::vector<uint32_t> summary;              //!< The available slot flag list.
		std::vector<InstanceVirtual*> instanceList; //!< The list of all instances.

		std::atomic_uchar reallocSize;              //!< The default reallocation size for summary.
		std::atomic_uint maxInstance;               //!< The maximum count of instance in list.
		std::atomic_uint nbInstance;                //!< The number of instance in list.
		//
};

