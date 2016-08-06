#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Utiles/Singleton.h"
#include "Utiles/Mutex.h"
#include "NodeVirtual.h"


class SceneManager : public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;

	public:
		//  Public functions
		bool addStaticObject(InstanceVirtual* obj);
		bool removeObject(InstanceVirtual* obj);
		//

		//	Debug
		void print();
		//

		//  Set/get functions
		void setCameraAttributes(glm::vec3 position, glm::vec3 direction, glm::vec3 vertical, glm::vec3 left, float verticalAngle, float horizontalAngle);
		void setViewDistance(float distance,int depth);
		void setWorldSize(glm::vec3 size);
		void setWorldPosition(glm::vec3 position);

		void getInstanceList(std::vector<std::pair<int, InstanceVirtual*> >& list);
		std::vector<float> getMaxViewDistanceStack();
		//

	private:
		//  Default
		SceneManager();		//!< Default constructor.
		~SceneManager();	//!< Default destructor.
		//

		//  Attributes
		std::vector<NodeVirtual*> world;
		std::vector<float> viewMaxDistance;
		//
};
