#pragma once

#include <iostream>
#include <set>
#include <list>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <Utiles/Mutex.h>
#include <Utiles/Singleton.h>
#include <Events/EventHandler.h>

#include "Layer.h"
#include "WidgetImage.h"
#include "WidgetBoard.h"
#include "WidgetLabel.h"
#include "WidgetConsole.h"
#include "WidgetRadioButton.h"


#define ANGLE_VERTICAL_HUD_PROJECTION 45.f
#define DISTANCE_HUD_CAMERA 0.15f


class WidgetManager : public Singleton<WidgetManager>
{
	friend class Singleton<WidgetManager>;
	friend class Renderer;

	public:
		//	Callback
		static void resizeCallback(int w, int h);
		//

		//	Public functions
		void update(const float& elapsedTime, const bool& clickButtonPressed);
		void loadHud(const std::string& hudName);

		void addAssociation(WidgetVirtual* w, const std::string& associationName);
		void setBoolean(const std::string& associationName, const bool& b);
		void setString(const std::string& associationName, const std::string& s);
		void append(const std::string& associationName, const std::string& s);
		std::string getString(const std::string& associationName);
		bool getBoolean(const std::string& associationName);

		void addWidget(WidgetVirtual* w);
		void addLayer(Layer* l);
		void removeWidget(WidgetVirtual* w);
		void removeLayer(Layer* l);
		//

		//	Set / get functions
		void setInitialViewportRatio(float viewportRatio);
		void setActiveHUD(const std::string& s);
		void setPickingParameters(const glm::mat4& base, const glm::vec3& ray);// , const glm::vec3& origin);

		std::string getActiveHUD() const;
		unsigned int getNbDrawnWidgets() const;
		unsigned int getNbDrawnTriangles() const;
		bool isUnderMouse() const;
		//

	private:
		//  Default
		WidgetManager();
		~WidgetManager();
		//

		//	Protected functions
		void generateDebugHud();
		void generateHelpHud();
		void generateRenderingHud();

		bool tryExtractVector(const Variant& variant, glm::vec3& vector, std::string& error) const;
		//

		//	Attributes
		std::set<Layer*> layerList;
		std::set<WidgetVirtual*> widgetList;
		std::map<std::string, std::vector<Layer*> > hudList;
		std::string activeHud;
		std::map<std::string, WidgetVirtual*> associations;

		unsigned int widgetDrawn, trianglesDrawn;

		glm::vec3 pickingRay;
		glm::mat4 pickingBase;
		std::set<WidgetVirtual*> hoverWidgetList;
		std::list<WidgetVirtual*> activeWidgetList;
		std::map<WidgetVirtual*, Layer*> activeWidgetParentList;

		float lastViewportRatio;
		//
};
