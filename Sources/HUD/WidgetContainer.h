#pragma once

#include <vector>
#include <algorithm>

#include "WidgetVirtual.h"

class WidgetContainer
{
	public:
		//  Default
		WidgetContainer();
		virtual ~WidgetContainer();
		//

		//	Public functions
		virtual void add(WidgetVirtual* w);
		virtual bool remove(WidgetVirtual* w);
		//

		//  Set/get functions
		std::vector<WidgetVirtual*>& getWidgetList();
		//

	protected:
		//  Attributes
		std::vector<WidgetVirtual*> widgetList;
		//
};