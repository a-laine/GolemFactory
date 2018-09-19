#pragma once

#include <list>
#include <algorithm>

#include "InstanceVirtual.h"

class InstanceContainer : public InstanceVirtual
{
	public:
		//  Default
		InstanceContainer();
		virtual ~InstanceContainer();
		//

		//	Public functions
		void addInstance(InstanceVirtual* ins);

		void removeInstance(InstanceVirtual* ins);
		const std::list<InstanceVirtual*>* getChildList() const;
		//

	protected:
		//	Attributes
		std::list<InstanceVirtual*> child;
		//
};
