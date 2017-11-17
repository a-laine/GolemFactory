#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "WidgetVirtual.h"
#include "WidgetContainer.h"

class Layer : public WidgetContainer
{
	public:
		//  Miscellaneous
		enum Flags
		{
			VISIBLE = 1 << 0
		};
		//

		//  Default
		Layer(const uint8_t& config = VISIBLE);
		virtual ~Layer();
		//

		//	Public functions
		virtual void update(const float& elapseTime);
		//

		//  Set/get functions
		void setPosition(const glm::vec3& p);
		void setSize(const float& s);
		void setOrientation(const float& yaw, const float& pitch, const float& roll);
		void setVisibility(const bool& visible);

		bool isVisible() const;
		glm::mat4 getModelMatrix() const;
		//

	protected:
		//  Attributes
		uint8_t configuration;
		glm::vec3 position;
		float size;
		glm::vec3 eulerAngle;

		std::vector<WidgetVirtual*> widgetList;
		//
};
