#include "ComponentUpdater.h"

#include <Utiles/ProfilerConfig.h>

ComponentUpdater::ComponentUpdater()
{}

ComponentUpdater::~ComponentUpdater()
{}

void ComponentUpdater::init()
{
#define BUCKET_MACRO(name,multithreaded)\
	m_passInfos.push_back({"e"#name, ##multithreaded});\

	#include "SchedulerBuckets.h"
#undef BUCKET_MACRO
}


void ComponentUpdater::updateFromGameState(Component::UpdatePass _pass, float _dt)
{
	SCOPED_CPU_MARKER(m_passInfos[(int)_pass].m_name.c_str());

	// add stuff
	for (const auto& element : addList)
	{
		auto it = componentUpdateList.find(element.m_pass);
		if (it != componentUpdateList.end())
			it->second.push_back(element);
		else
		{
			auto& list = componentUpdateList[element.m_pass];
			list.push_back(element);
		}
	}
	addList.clear();

	// remove stuff
	for (const auto& element : removeList)
	{
		auto it = componentUpdateList.find(element.m_pass);
		if (it == componentUpdateList.end())
			continue;
		int id = -1;
		for (int i = 0; i < it->second.size(); i++)
			if (it->second[i].m_component == element.m_component)
			{
				id = i;
				break;
			}
		if (id >= 0)
			it->second.erase(it->second.begin() + id);
	}
	removeList.clear();

	auto list = componentUpdateList.find(_pass);
	if (list != componentUpdateList.end())
	{
		for (auto& element : list->second)
		{
			element.m_callback(_pass, _dt);
		}
	}
}

void ComponentUpdater::add(Component::UpdatePass _pass, const Component::UpdateCallback& _callback, Component* _component, Entity* _entity)
{
	addList.push_back({ _pass, _callback, _component, _entity });
}

void ComponentUpdater::remove(Component::UpdatePass _pass, Component* _component)
{
	removeList.push_back({ _pass, 0, _component, nullptr });
}




