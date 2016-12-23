#include "InstanceManager.h"


//  Default
InstanceManager::InstanceManager(unsigned int maximum) : maxInstance(maximum), nbInstance(0)
{
	maxInstance = ((maximum >> 5) + 1) << 5;
	summary.assign(maxInstance >> 5, 0);
	instanceList.assign(maxInstance, nullptr);
	reallocSize = 2;
}
InstanceManager::~InstanceManager()
{
	for (auto element : instanceList)
		if (element) delete element;
	instanceList.clear();
	summary.clear();
	clearGarbage();
}
//


//  Public functions
InstanceVirtual* InstanceManager::add(InstanceVirtual* ins)
{
	//  assign instance to founded slot
	mutexList.lock();
	uint32_t index = getAvalibleId();
	if (index == std::numeric_limits<uint32_t>::max())
	{
		mutexList.unlock();
		std::cerr << "no id available" << std::endl;
		return nullptr;
	}
	instanceList[index] = ins;
	ins->id = index;

	//  end
	ins->count++;
	mutexList.unlock();
	nbInstance++;
	return ins;
}
InstanceVirtual* InstanceManager::get(uint32_t id)
{
	if (id >= instanceList.size()) return nullptr;
	mutexList.lock();
	if (instanceList[id]) instanceList[id]->count++;
	mutexList.unlock();
	return instanceList[id];
}


void InstanceManager::release(InstanceVirtual* ins)
{
	//  decrement client count
	if (!ins || ins->id >= instanceList.size()) return;
	ins->count--;
	if (ins->count.load()>0) return;

	//  add to garbage
	mutexGarbage.lock();
	garbage.insert(garbage.end(), ins);
	mutexGarbage.unlock();

	//  release instance slot
	nbInstance--;
	mutexList.lock();
	instanceList[ins->id] = nullptr;
	freeId(ins->id);
	mutexList.unlock();
}
void InstanceManager::clearGarbage()
{
	std::vector<InstanceVirtual*> garbageCopy;

	mutexGarbage.lock();
	garbage.swap(garbageCopy);
	mutexGarbage.unlock();

	for (auto element : garbageCopy)
		if (element) delete element;
	garbageCopy.clear();
}

InstanceDrawable* InstanceManager::getInstanceDrawable(std::string meshName, std::string shaderName)
{
	InstanceDrawable* ins = new InstanceDrawable(meshName, shaderName);
	if (!ins || !add(ins))
	{
		if(ins) delete ins;
		return nullptr;
	}
	return ins;
}
//


//  Set/get functions
void InstanceManager::setMaxNumberOfInstances(unsigned int maxIns) { maxInstance = std::max(maxInstance.load(), ((maxIns >> 5) + 1) << 5); }
void InstanceManager::setReallocSize(uint8_t size) { reallocSize = size; }
void InstanceManager::reserveInstanceSize(unsigned int size)
{
	size = std::max(((size >> 5) + 1) << 5, maxInstance.load());
	if (size>instanceList.size())
	{
		mutexList.lock();
		summary.resize((size >> 5), 0);
		instanceList.resize(size, nullptr);
		mutexList.unlock();
	}
}


unsigned int InstanceManager::getNumberOfInstances() const { return nbInstance.load(); }
unsigned int InstanceManager::getMaxNumberOfInstances() const { return maxInstance.load(); }
unsigned int InstanceManager::getInstanceCapacity() const { return instanceList.capacity(); }
unsigned int InstanceManager::getInstanceSlot() const { return instanceList.size(); }
//


//  Private functions
uint32_t InstanceManager::getAvalibleId()
{
	//  search a free page
	unsigned int page = 0;
	for (; page<summary.size(); page++)
		if (summary[page] != 0xFFFFFFFF) break;
	if (page >= summary.size())
	{
		if (instanceList.size() < maxInstance) // allocate more space
		{
			uint8_t tmp = reallocSize.load();
			if (summary.capacity() <= summary.size())
				summary.reserve(summary.size() + tmp);
			if (instanceList.capacity() <= instanceList.size())
				instanceList.reserve(instanceList.size() + 32 * tmp);

			page = summary.size();
			summary.push_back(0);
			for (int i = 0; i<32; i++)
				instanceList.push_back(nullptr);
		}
		else return std::numeric_limits<uint32_t>::max();
	}

	//  search a free slot in 'page'
	unsigned int slot = 0;
	for (; slot<32; slot++)
		if (~summary[page] & (1 << slot)) break;
	if (slot >= 32) return std::numeric_limits<uint32_t>::max();

	summary[page] |= (1 << slot);
	return (page << 5) + slot;
}
void InstanceManager::freeId(uint32_t id) { summary[id >> 5] &= ~(1 << (id % 32)); }
//
