#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
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
			COARSE = 0,
			INSTANCE_BB = 1,
			INSTANCE_CAPSULE = 2,
			INSTANCE_MESH = 3
		};
		//

		//  Scene modifier
		bool addObject(InstanceVirtual* obj);
		bool removeObject(InstanceVirtual* obj);
		void updateObject(InstanceVirtual* obj);
		//

		//  Set/get functions
		void reserveInstanceTrack(const unsigned int& count);
		void setCameraAttributes(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& vertical, const glm::vec3& left, const float& verticalAngle, const float& horizontalAngle);
		void setViewDistance(float distance, int depth);
		void setWorldSize(glm::vec3 size);
		void setWorldPosition(glm::vec3 position);

		void getInstanceList(std::vector<std::pair<int, InstanceVirtual*> >& list, const Grain& grain = COARSE);
		void getInstanceOnRay(std::vector<std::pair<float, InstanceVirtual*> >& list, const Grain& grain = COARSE, const float& maxDistance = std::numeric_limits<float>::max());
		std::vector<float> getMaxViewDistanceStack();
		unsigned int getNumberInstanceStored() const;
		glm::vec3 getWorldSize() const;
		glm::vec3 getWorldPosition() const;
		//

		//	scene query
		void queryNodeBox(std::vector<NodeVirtual*>& nodeList, const glm::vec3& aabbMin, const glm::vec3& aabbMax);
		void queryInstanceBox(std::vector<InstanceVirtual*>& instanceList, const glm::vec3& center, const glm::vec3& halfSize);
		//

	private:
		//	Miscellaneous
		struct InstanceTrack
		{
			glm::vec3 position;
			NodeVirtual* owner;
		};
		//



		//  Default
		SceneManager();		//!< Default constructor.
		~SceneManager();	//!< Default destructor.
		//

		//	Node tests
		int isInFrustrum(const NodeVirtual* const node, const glm::vec3& camP, const glm::vec3& camD, const glm::vec3& camV, const glm::vec3& camL, const float& camVa, const float& camHa) const;
		float isOnRay(const NodeVirtual* const node, const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& directionV, const glm::vec3& directionL) const;
		
		int8_t isCollidingBox(const NodeVirtual* const node, const glm::vec3& center, const glm::vec3& halfSize) const;

		void queryNodeBoxRecursive(std::vector<NodeVirtual*>& nodeList, const glm::vec3& center, const glm::vec3& halfSize, NodeVirtual* const node, const bool& inside);
		//

		//  Attributes
		std::vector<NodeVirtual*> world;
		std::unordered_map<uint32_t, InstanceTrack> instanceTracking;
		std::vector<float> viewMaxDistance;

		glm::vec3 camPosition;				//!< Camera position
		glm::vec3 camDirection;				//!< Camera forward vector
		glm::vec3 camVertical;				//!< Camera relative vertical vector
		glm::vec3 camLeft;					//!< Camera left vector
		float camVerticalAngle;				//!< Frustrum angle vertical
		float camHorizontalAngle;			//!< Frustrum angle horizontal
		//
};
