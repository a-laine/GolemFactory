#include "ComponentUpdater.h"

ComponentUpdater::ComponentUpdater()
{}

ComponentUpdater::~ComponentUpdater()
{}

void ComponentUpdater::update(float _dt)
{
	// remove stuff
	for (const auto& element : removeList)
	{
		auto it = componentUpdateList.find(element.m_pass);
		if (it == componentUpdateList.end())
			continue;
		int id = -1;
		for (int i = 0; i < it->second.size(); i++)
			if (it->second[i].component == element.m_component)
			{
				id = i;
				break;
			}
		if (id >= 0)
			it->second.erase(it->second.begin() + id);
	}
	removeList.clear();

	// add stuff
	for (const auto& element : addList)
	{
		auto it = componentUpdateList.find(element.m_pass);
		if (it != componentUpdateList.end())
			it->second.push_back({ element.m_callback, element.m_component });
		else
		{
			auto& list = componentUpdateList[element.m_pass];
			list.push_back({ element.m_callback, element.m_component });
		}
	}
	addList.clear();

	// updates
	for (auto& it : componentUpdateList)
	{
		for (int i = 0; i < it.second.size(); i++)
			(*it.second[i].updateFunction)(it.second[i].component, _dt);
	}
}

void ComponentUpdater::add(Component::UpdatePass _pass, Component::UpdateCallback _callback, void* _component)
{
	addList.push_back({ _pass, _callback, _component });
}

void ComponentUpdater::remove(Component::UpdatePass _pass, void* _component)
{
	removeList.push_back({ _pass, nullptr, _component });
}




