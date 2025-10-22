#pragma once

#include <Utiles/Singleton.h>
#include <EntityComponent/Component.hpp>

#include <vector>
#include <map>

class ComponentUpdater : public Singleton<ComponentUpdater>
{
	friend class Singleton<ComponentUpdater>;

	public:
		struct UpdateElement
		{
			Component::UpdatePass m_pass;
			Component::UpdateCallback m_callback;
			Component* m_component;
			Entity* m_entity;
		};

		//  Public functions
		void init();
		void updateFromGameState(Component::UpdatePass _pass, float _dt);
		void add(Component::UpdatePass _pass, const Component::UpdateCallback& _callback, Component* _component, Entity* _entity);
		void remove(Component::UpdatePass _pass, Component* _component);
		void remove(Component::UpdatePass _pass, Entity* _entity);
		void removeAll(Entity* _entity);

	private:
		//  Default
		ComponentUpdater();
		~ComponentUpdater();
		//

		std::map<Component::UpdatePass, std::vector<UpdateElement>> componentUpdateList;
		std::vector<UpdateElement> addList;
		std::vector<UpdateElement> removeList;

		struct PassInfos
		{
			std::string m_name;
			bool m_isMultithreadable;
		};
		std::vector<PassInfos> m_passInfos;
};