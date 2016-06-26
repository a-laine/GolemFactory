#pragma once

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Instances/InstanceManager.h"


class NodeVirtual
{
	friend class SceneManager;

	public:
		//  Miscellaneous
		enum NodeDivisionFlags
		{
			X = 0x0000FF,	dX = 0,
			Y = 0x00FF00,	dY = 8,
			Z = 0xFF0000,	dZ = 16
		};
		//

		//	Default
		NodeVirtual(NodeVirtual* p = nullptr, unsigned int d = 0x000000);
		virtual ~NodeVirtual();
		//

		//	Public functions
		int getNbFils(int level = -1);
		bool isLastBranch();

		virtual void addObject(InstanceVirtual* obj);
		virtual bool removeObject(InstanceVirtual* obj);
		//

		//Set/Get functions
		void setPosition(glm::vec3 p);
		void setPosition(float x, float y, float z);
		void setSize(glm::vec3 s);
		void setSize(float x, float y, float z);

		glm::vec3 getPosition();
		glm::vec3 getSize();
		//

	protected:
		//	Protected functions
		uint8_t getLevel() const;
		int getChildrenKey(glm::vec3 p) const;
		//

		//Attributes
		NodeVirtual* parent;
		std::vector<NodeVirtual*> children;
		glm::vec3 position, size;

		std::vector<InstanceVirtual*> instanceList;
		unsigned int division;
		//
};


