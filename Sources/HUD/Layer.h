#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "WidgetVirtual.h"

class Layer
{
	friend class WidgetSaver;

	public:
		//  Miscellaneous
		enum Flags
		{
			VISIBLE = 1 << 0,
			RESPONSIVE = 1 << 1
		};
		//

		//  Default
		Layer(const uint8_t& config = VISIBLE | RESPONSIVE);
		virtual ~Layer();
		//

		//	Public functions
		virtual void update(const float& elapseTime);
		//

		//  Set/get functions
		void setPosition(const glm::vec3& p);
		void setScreenPosition(const glm::vec3& p);
		void setTargetPosition(const glm::vec3& p);
		void setSize(const float& s);
		void setOrientation(const float& yaw, const float& pitch, const float& roll);
		void setVisibility(const bool& visible);
		void setResponsive(const bool& responsive);
		void setConfiguration(const uint8_t& config);

		bool isVisible() const;
		bool isResponsive() const;
		float getSize() const;
		glm::mat4 getModelMatrix() const;
		glm::vec3 getPosition() const;
		glm::vec3 getScreenPosition() const;
		glm::vec3 getTargetPosition() const;
		std::vector<WidgetVirtual*>& getChildrenList();
		//

		//	Hierarchy modifiers
		virtual void addChild(WidgetVirtual* w);
		virtual bool removeChild(WidgetVirtual* w);
		//

	protected:
		//  Attributes
		uint8_t configuration;
		glm::vec3 position;
		glm::vec3 screenPosition;
		glm::vec3 targetPosition;
		float size;
		glm::vec3 eulerAngle;
		std::vector<WidgetVirtual*> children;
		//
};
