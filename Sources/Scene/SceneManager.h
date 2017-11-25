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
		//	Miscellaneous
		enum Grain
		{
			COARSE,
			INSTANCE_BB,
			INSTANCE_MESH
		};
		//

		//  Scene modifier
		bool addStaticObject(InstanceVirtual* obj);
		bool removeObject(InstanceVirtual* obj);
		//

		//  Set/get functions
		void setCameraAttributes(glm::vec3 position, glm::vec3 direction, glm::vec3 vertical, glm::vec3 left, float verticalAngle, float horizontalAngle);
		void setViewDistance(float distance,int depth);
		void setWorldSize(glm::vec3 size);
		void setWorldPosition(glm::vec3 position);

		void getInstanceList(std::list<std::pair<int, InstanceVirtual*> >& list, const Grain& grain = COARSE);
		void getInstanceOnRay(std::list<std::pair<int, InstanceVirtual*> >& list, const Grain& grain = COARSE);
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

		glm::vec3 camPosition;				//!< Camera position
		glm::vec3 camDirection;				//!< Camera forward vector
		glm::vec3 camVertical;				//!< Camera relative vertical vector
		glm::vec3 camLeft;					//!< Camera left vector
		float camVerticalAngle;				//!< Frustrum angle vertical
		float camHorizontalAngle;			//!< Frustrum angle horizontal
		//
};
