#pragma once

#include <atomic>
#include <algorithm>

#include "Utiles/Singleton.h"
#include "Utiles/Mutex.h"
#include "InstanceVirtual.h"
#include "InstanceDrawable.h"


class InstanceManager : public Singleton<InstanceManager>
{
	friend class Singleton<InstanceManager>;

	public:
		//  Public functions
		InstanceVirtual* add(InstanceVirtual* ins);
		InstanceVirtual* get(uint32_t id);

		void release(InstanceVirtual* ins);
		void clearGarbage();

		InstanceDrawable* getInstanceDrawable(std::string meshName = "default", std::string shaderName = "default");
		//

		//  Set/get functions
		void setMaxNumberOfInstances(unsigned int maxIns);
		void setReallocSize(uint8_t size);
		void reserveInstanceSize(unsigned int size);

		unsigned int getNumberOfInstances() const;
		unsigned int getMaxNumberOfInstances() const;
		unsigned int getInstanceCapacity() const;
		unsigned int getInstanceSlot() const;
		//


	private:
		//  Default
		InstanceManager(unsigned int maximum = 16384);
		~InstanceManager();
		//

		//  Private functions
		uint32_t getAvalibleId();
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

