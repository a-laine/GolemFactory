#pragma once


#include "WidgetVirtual.h"

class WidgetImage : public WidgetVirtual
{
	public:
		//  Default
		WidgetImage(const std::string& textureName= "default", const uint8_t& config = VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetImage();
		//

		//	Public functions
		virtual void initialize();



		//for test burn after copy elswhere !!!!!
			bool mouseEvent(const glm::vec3& eventLocation, const bool& clicked);





		//
};