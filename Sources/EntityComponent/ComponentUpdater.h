#pragma once

#include <Utiles/Singleton.h>
#include <EntityComponent/Component.hpp>

#include <vector>
#include <map>

class ComponentUpdater : public Singleton<ComponentUpdater>
{
	friend class Singleton<ComponentUpdater>;

	public:
		//  Public functions
		void update(float _dt);
		void add(Component::UpdatePass _pass, Component::UpdateCallback _callback, void* _component);
		void remove(Component::UpdatePass _pass, void* _component);

	private:
		//  Default
		ComponentUpdater();
		~ComponentUpdater();
		//

		struct TmpElement
		{
			Component::UpdatePass m_pass;
			Component::UpdateCallback m_callback;
			void* m_component;
		};
		std::map<Component::UpdatePass, std::vector<Component::ComponentUpdateData>> componentUpdateList;
		std::vector<TmpElement> addList;
		std::vector<TmpElement> removeList;
};