#pragma once

#include <iostream>
#include <vector>


/*#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>*/

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
		void setPosition(const vec4f& p);
		void setScreenPosition(const vec4f& p);
		void setTargetPosition(const vec4f& p);
		void setSize(const float& s);
		void setOrientation(const float& yaw, const float& pitch, const float& roll);
		void setVisibility(const bool& visible);
		void setResponsive(const bool& responsive);
		void setConfiguration(const uint8_t& config);

		bool isVisible() const;
		bool isResponsive() const;
		float getSize() const;
		mat4f getModelMatrix() const;
		vec4f getPosition() const;
		vec4f getScreenPosition() const;
		vec4f getTargetPosition() const;
		std::vector<WidgetVirtual*>& getChildrenList();
		//

		//	Hierarchy modifiers
		virtual void addChild(WidgetVirtual* w);
		virtual bool removeChild(WidgetVirtual* w);
		//

	protected:
		//  Attributes
		uint8_t configuration;
		vec4f position;
		vec4f screenPosition;
		vec4f targetPosition;
		float size;
		vec3f eulerAngle;
		std::vector<WidgetVirtual*> children;
		//
};
