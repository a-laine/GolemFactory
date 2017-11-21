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
		void loadHud(const std::string& hudName);

		void addAssociation(WidgetVirtual* w, const std::string& associationName);
		void setString(const std::string& associationName, const std::string& s);
		std::string getString(const std::string& associationName);
		std::stringstream* getStream(const std::string& associationName);

		void addWidget(WidgetVirtual* w);
		void addLayer(Layer* l);
		void removeWidget(WidgetVirtual* w);
		void removeLayer(Layer* l);
		//

		//	Set / get functions
		void setActiveHUD(const std::string& s);
		void setPickingParameters(const glm::mat4& base, const glm::vec3& ray, const glm::vec3& origin);

		std::string getActiveHUD() const;
		unsigned int getNbDrawnWidgets() const;
		unsigned int getNbDrawnTriangles() const;
		//

	private:
		//  Default
		WidgetManager();
		~WidgetManager();
		//

		//	Protected functions
		void loadDebugHud();
		//

		//	Attributes
		std::set<Layer*> layerList;
		std::set<WidgetVirtual*> widgetList;
		std::map<std::string, std::vector<Layer*> > hudList;
		std::string activeHud;
		std::map<std::string, WidgetVirtual*> associations;

		unsigned int widgetDrawn, trianglesDrawn;

		glm::vec3 pickingRay;
		glm::vec3 pickingOrigin;
		glm::mat4 pickingBase;
		std::set<WidgetVirtual*> hoverWidgetList;
		//
};
