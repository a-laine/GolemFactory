#pragma once

#include <iostream>

#include <GL/glew.h>

#include "Utiles/Mutex.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Utiles/Singleton.h"

#include "HUD/Layer.h"
#include "HUD/WidgetImage.h"
#include "HUD/WidgetBoard.h"
#include "HUD/WidgetLabel.h"


class WidgetManager : public Singleton<WidgetManager>
{
	friend class Singleton<WidgetManager>;

	public:
		//	Public functions
		void draw(Shader* s, const glm::mat4& base, const float* view, const float* projection);
		void update(const float& elapsedTime);
		void loadHud(const std::string& fileName);

		void addWidget(WidgetVirtual* w);
		void removeWidget(WidgetVirtual* w);
		void addLayer(Layer* l);
		void removeLayer(Layer* l);

		//

		//	Set / get functions
		void setActiveHUD(const std::string& s);

		std::string getActiveHUD() const;
		//

	private:
		//  Default
		WidgetManager();
		~WidgetManager();
		//

		//	Protected functions

		//

		//	Attributes
		std::set<Layer*> layerList;
		std::set<WidgetVirtual*> widgetList;
		std::map<std::string, std::vector<Layer*> > hudList;
		std::string activeHud;
		//
};
