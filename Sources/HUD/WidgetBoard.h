#pragma once


#include <vector>

#include "WidgetVirtual.h"


class WidgetBoard : public WidgetVirtual
{
	friend class WidgetSaver;

	public:
		//  Miscellaneous
		enum CornerConfig
		{
			TOP_RIGHT = 1 << 0,
			TOP_LEFT = 1 << 1,
			BOTTOM_RIGHT = 1 << 2,
			BOTTOM_LEFT = 1 << 3
		};
		enum class BoardFlags : uint8_t
		{
			CAN_BE_ACTIVATED = (uint8_t)WidgetVirtual::OrphanFlags::SPECIAL
		};
		//

		//  Default
		WidgetBoard(const uint8_t& config = (uint8_t)WidgetVirtual::OrphanFlags::VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetBoard();
		//

		//  Public functions
		virtual void initialize(const float& bThickness, const float& bWidth, const uint8_t& corner = 0x00);
		virtual void draw(Shader* s, uint8_t& stencilMask, const mat4f& model) override;
		virtual void update(const float& elapseTime) override;
		virtual bool mouseEvent(const mat4f& base, const vec4f& ray, const float& parentscale, const bool& clicked) override;
		//

	protected:
		//	Protected functions
		virtual void updateBuffers(const bool& firstInit = false);
		void updateVBOs();
		//

		//	Attributes
		uint8_t cornerConfiguration;
		float borderWidth;
		float borderThickness;
		float updateCooldown;
		//bool activated;
		//
};