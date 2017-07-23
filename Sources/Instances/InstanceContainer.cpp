#include "InstanceContainer.h"


//  Default
InstanceContainer::InstanceContainer() : InstanceVirtual(InstanceVirtual::CONTAINER) {}
InstanceContainer::~InstanceContainer()
{
	for (auto it = child.begin(); it != child.end(); it++)
		(*it)->count--;
	child.clear();
}
//

//	Public functions
void InstanceContainer::addInstance(InstanceVirtual* ins)
{
	if (!ins) return;
	ins->count++;
	child.push_back(ins);
}


void InstanceContainer::removeInstance(InstanceVirtual* ins)
{
	if (!ins) return;
	auto it = std::find(child.begin(),child.end(), ins);
	if (it != child.end())
	{
		child.remove(ins);
		ins->count--;
	}
}
const std::list<InstanceVirtual*>* InstanceContainer::getChildList() const
{
	return &child;
}
//
