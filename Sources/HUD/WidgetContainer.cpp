
#include "WidgetContainer.h"

//  Default
WidgetContainer::WidgetContainer() {}
WidgetContainer::~WidgetContainer() {}
//

//	Public functions
void WidgetContainer::add(WidgetVirtual* w) { widgetList.push_back(w); }
bool WidgetContainer::remove(WidgetVirtual* w)
{
	std::vector<WidgetVirtual*>::iterator it = std::find(widgetList.begin(), widgetList.end(), w);
	if (it != widgetList.end())
	{
		widgetList.erase(it);
		return true;
	}
	else return false;
}
//

//  Set/get functions
std::vector<WidgetVirtual*>& WidgetContainer::getWidgetList() { return widgetList; }
//