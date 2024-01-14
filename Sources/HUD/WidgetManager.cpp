#include "WidgetManager.h"
#include "Loader/WidgetSaver.h"
#include <Utiles/Parser/Writer.h>
#include <Utiles/Parser/Reader.h>
#include <HUD/Loader/WidgetLoader.h>
#include <Utiles/ConsoleColor.h>

//  Default
WidgetManager::WidgetManager() : widgetDrawn(0), trianglesDrawn(0), pickingRay(0.f), lastViewportRatio(1.f) {}
WidgetManager::~WidgetManager()
{
	hudList.clear();
	for (std::set<WidgetVirtual*>::iterator it = widgetList.begin(); it != widgetList.end(); ++it)
		delete *it;
}
//


//	Public functions
void WidgetManager::loadHud(const std::string& hudName)
{
	if (hudName == "default")
	{
		if (hudList.find("debug") == hudList.end())
			generateDebugHud();
		if (hudList.find("help") == hudList.end())
			generateHelpHud();
		if (hudList.find("rendering") == hudList.end())
			generateRenderingHud();
	}
	else
	{
		//	skip loading if already present in list
		if (hudList.find(hudName) != hudList.end())
			return;

		//	load and parse file into variant structure
		Variant v; Variant* tmpvariant;
		std::string fullpath = ResourceManager::getInstance()->getRepository() + "GUI/" + hudName + ".gui";
		try
		{
			Reader::parseFile(v, fullpath);
			Variant::MapType::iterator it = v.getMap().begin();
			tmpvariant = &(it->second);
		}
		catch (const std::exception&)
		{
			std::cerr << "WidgetManager : Fail to open or parse file (variant loading fail) :" << std::endl;
			std::cerr << "               " << fullpath << std::endl;
			return;
		}
		Variant& hudMap = *tmpvariant;
		if (hudMap.getType() != Variant::VariantType::MAP)
		{
			std::cerr << "WidgetManager : base variant is not a map :" << std::endl;
			std::cerr << "               " << fullpath << std::endl;
			return;
		}

		// iterate over layers
		std::string errorHeader = "WidgetManager : Error parsing " + hudName + ".gui";
		const std::string errorIndent = "               ";
		const std::string discardedMsg = "-> layer has been discarded";
		std::vector<Layer*> layers;
		bool errorOccur = false;
		int fatalErrorCount = 0;
		int lastFatalError = 0;

		for (Variant::MapType::iterator it = hudMap.getMap().begin(); it != hudMap.getMap().end(); it++)
		{
			if (errorHeader.empty())
			{
				errorOccur = true;
				if (lastFatalError)
					errorHeader = errorIndent + discardedMsg + ", parsing continue";
				else
					errorHeader = errorIndent + "-> layer has warning, parsing continue";
			}

			int errorCount = 0;
			std::map<std::string, WidgetVirtual*> associations;
			Layer* layer = WidgetLoader::deserialize(it->second, it->first, associations, errorHeader, errorIndent, errorCount);
			fatalErrorCount += errorCount;
			lastFatalError = errorCount;

			if (layer)
			{
				std::vector<WidgetVirtual*>& children = layer->getChildrenList();
				for (unsigned int i = 0; i < children.size(); i++)
					addWidget(children[i]);

				addLayer(layer);
				layers.push_back(layer);

				if (activeHud == hudName)
				{
					layer->setTargetPosition(layer->getScreenPosition());
					layer->setPosition(layer->getScreenPosition());
				}
				else
				{
					vec4f direction = layer->getScreenPosition();
					if (direction == vec4f::zero)
						direction = vec4f(0.f, 0.f, 1.f, 0.f);
					else
						direction.normalize();
					layer->setTargetPosition(layer->getScreenPosition() + 0.1f * direction);
					layer->setPosition(layer->getScreenPosition() + 0.1f * direction);
				}

				for (std::map<std::string, WidgetVirtual*>::iterator it = associations.begin(); it != associations.end(); ++it)
					addAssociation(it->second, it->first);
			}
		}
		if (!layers.empty())
		{
			hudList[hudName] = layers;
		}
		if (errorOccur)
		{
			std::cerr << ConsoleColor::getColorString(ConsoleColor::Color::RED) << errorIndent << (lastFatalError ? discardedMsg : "-> end") << std::flush;
			std::cerr << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
		}
	}

	for (auto it = hudList.begin(); it != hudList.end(); ++it)
	{
		if (it->first == activeHud)
			continue;
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			vec4f direction = it->second[i]->getScreenPosition();
			if (direction == vec4f::zero)
				direction = vec4f(0.f, 0.f, 1.f, 0.f);
			else
				direction.normalize();
			it->second[i]->setTargetPosition(it->second[i]->getScreenPosition() + 0.1f * direction);
			it->second[i]->setPosition(it->second[i]->getScreenPosition() + 0.1f * direction);
		}
	}
}
void WidgetManager::deleteHud(const std::string& hudName)
{
	std::map<std::string, std::vector<Layer*>>::iterator it = hudList.find(hudName);
	if (it == hudList.end())
		return;

	/*if (hudName == activeHud)
		setActiveHUD("");*/

	for (unsigned int i = 0; i < it->second.size(); i++)
	{
		Layer* layer = it->second[i];
		removeLayer(layer);
		std::vector<WidgetVirtual*>& children = layer->getChildrenList();
		for (unsigned int j = 0; j < children.size(); j++)
		{
			removeWidget(children[j]);
			delete children[j];
		}
		delete layer;
	}
	hudList.erase(it);
}
void WidgetManager::update(const float& elapsedTime, const bool& clickButtonPressed)
{
	//	picking
	std::map<WidgetVirtual*, Layer*> tmpParentList;
	std::set<WidgetVirtual*> tmpWidgetList;
	if (pickingRay != vec4f::zero)
	{
		//	draw all widget in hudList
		for (std::map<std::string, std::vector<Layer*> >::iterator it = hudList.begin(); it != hudList.end(); ++it)
			for (unsigned int i = 0; i < it->second.size(); i++)
				if (it->second[i]->isVisible())		// layer visible
				{
					mat4f model = pickingBase * it->second[i]->getModelMatrix();
					std::vector<WidgetVirtual*>& list = it->second[i]->getChildrenList();
					for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
					{
						if ((*it2)->isVisible())	// widget visible
						{
							if ((*it2)->intersect(mat4f::translate(model, (*it2)->getPosition()), pickingRay))
							{
								tmpWidgetList.insert(*it2);
								tmpParentList[*it2] = it->second[i];
							}
						}
					}
				}
	}

	//	trigger new hover widget and widget leaving hover list
	for (std::set<WidgetVirtual*>::iterator it = tmpWidgetList.begin(); it != tmpWidgetList.end(); ++it)
	{
		if (hoverWidgetList.find(*it) == hoverWidgetList.end())
		{
			(*it)->setState(WidgetVirtual::State::HOVER);
		}
	}
	for (std::set<WidgetVirtual*>::iterator it = hoverWidgetList.begin(); it != hoverWidgetList.end(); ++it)
	{
		//	widget not hover anymore and not active anymore
		if (tmpWidgetList.find(*it) == tmpWidgetList.end() && std::find(activeWidgetList.begin(), activeWidgetList.end(), *it) == activeWidgetList.end())
			(*it)->setState(WidgetVirtual::State::DEFAULT);
	}
	hoverWidgetList.swap(tmpWidgetList);

	//	widget hover and a click occur -> add widget to active list if possible
	if (clickButtonPressed)
		for (std::set<WidgetVirtual*>::iterator it = hoverWidgetList.begin(); it != hoverWidgetList.end(); ++it)
		{
			mat4f model = pickingBase * tmpParentList[*it]->getModelMatrix();
			if((*it)->mouseEvent(mat4f::translate(model, (*it)->getPosition()), pickingRay, tmpParentList[*it]->getSize(), clickButtonPressed))
			{
				activeWidgetList.insert(*it);
				activeWidgetParentList[*it] = tmpParentList[*it];
			}
		}
	
	//	special update on active widget
	std::set<WidgetVirtual*> inactiveWidgetList;
	for (std::set<WidgetVirtual*>::iterator it = activeWidgetList.begin(); it != activeWidgetList.end(); ++it)
	{
		if(!(*it)->mouseEvent(mat4f::translate(pickingBase * activeWidgetParentList[*it]->getModelMatrix(),
			(*it)->getPosition()), pickingRay, activeWidgetParentList[*it]->getSize(), clickButtonPressed))
		{
			if (hoverWidgetList.find(*it) == hoverWidgetList.end())
				(*it)->setState(WidgetVirtual::State::DEFAULT);
			else
				(*it)->setState(WidgetVirtual::State::HOVER);
			inactiveWidgetList.insert(*it);
		}
	}
	for (std::set<WidgetVirtual*>::iterator it = inactiveWidgetList.begin(); it != inactiveWidgetList.end(); ++it)
	{
		activeWidgetList.erase(*it);
		activeWidgetParentList.erase(*it);
	}

	//	update all layers and widgets
	for (std::set<Layer*>::iterator it = layerList.begin(); it != layerList.end(); ++it)
		(*it)->update(elapsedTime);
	for (std::set<WidgetVirtual*>::iterator it = widgetList.begin(); it != widgetList.end(); ++it)
		(*it)->update(elapsedTime);
}


void WidgetManager::addAssociation(WidgetVirtual* widget, const std::string& associationName)
{
	associationsNameToWidget[associationName] = widget;

	std::map<WidgetVirtual*, std::vector<std::string>>::iterator it = associationsWidgetToName.find(widget);
	if (it == associationsWidgetToName.end())
	{
		std::vector<std::string> names;
		names.push_back(associationName);
		associationsWidgetToName[widget] = names;
	}
	else
		it->second.push_back(associationName);
}
void WidgetManager::removeAssociation(WidgetVirtual* widget, const std::string& associationName)
{
	associationsNameToWidget.erase(associationName);

	std::map<WidgetVirtual*, std::vector<std::string>>::iterator it = associationsWidgetToName.find(widget);
	if (it != associationsWidgetToName.end())
	{
		std::vector<std::string> newList;
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			if (it->second[i] != associationName)
				newList.push_back(it->second[i]);
		}
		it->second.swap(newList);
	}
}
void WidgetManager::setBoolean(const std::string& associationName, const bool& b)
{
	try
	{
		WidgetVirtual* w = associationsNameToWidget.at(associationName);
		if (w) w->setBoolean(b);
	}
	catch (const std::out_of_range &) {}
}
void WidgetManager::setString(const std::string& associationName, const std::string& s)
{
	try
	{
		WidgetVirtual* w = associationsNameToWidget.at(associationName);
		if (w) w->setString(s);
	}
	catch (const std::out_of_range &) {}
}
void WidgetManager::append(const std::string& associationName, const std::string& s)
{
	try
	{
		WidgetVirtual* w = associationsNameToWidget.at(associationName);
		if (w) w->append(s);
	}
	catch (const std::out_of_range &) {}
}
std::string WidgetManager::getString(const std::string& associationName)
{
	try
	{
		WidgetVirtual* w = associationsNameToWidget.at(associationName);
		if (w) return w->getString();
	}
	catch (const std::out_of_range &) {}
	return "";
}
bool WidgetManager::getBoolean(const std::string& associationName)
{
	try
	{
		WidgetVirtual* w = associationsNameToWidget.at(associationName);
		if (w) return w->getBoolean();
	}
	catch (const std::out_of_range &) {}
	return false;
}


void WidgetManager::addWidget(WidgetVirtual* w) { widgetList.insert(w); }
void WidgetManager::removeWidget(WidgetVirtual* w)
{
	widgetList.erase(w);
	hoverWidgetList.erase(w);
	activeWidgetList.erase(w);
	activeWidgetParentList.erase(w);

	std::map<WidgetVirtual*, std::vector<std::string>>::iterator it = associationsWidgetToName.find(w);
	if (it != associationsWidgetToName.end())
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
			associationsNameToWidget.erase(it->second[i]);
		associationsWidgetToName.erase(it);
	}
}
void WidgetManager::addLayer(Layer* l) { layerList.insert(l); }
void WidgetManager::removeLayer(Layer* l)
{
	layerList.erase(l);
	for (auto it = activeWidgetParentList.begin(); it != activeWidgetParentList.end(); )
	{
		if (it->second == l)
			it = activeWidgetParentList.erase(it);
		else ++it;
	}
}
//


//	Set / get functions
void WidgetManager::setInitialViewportRatio(float viewportRatio)
{
	lastViewportRatio = viewportRatio;
}

void WidgetManager::setActiveHUD(const std::string& name)
{
	//	activate new HUD layer
	std::map<std::string, std::vector<Layer*> >::iterator it = hudList.find(name);
	if (it != hudList.end())
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			it->second[i]->setVisibility(true);
			it->second[i]->setTargetPosition(it->second[i]->getScreenPosition());
		}
	}

	//	push away all widget from former HUD
	it = hudList.find(activeHud);
	if (it != hudList.end())
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			vec4f direction = it->second[i]->getScreenPosition();
			if (direction == vec4f::zero)
				direction = vec4f(0.f, 0.f, 1.f, 0.f);
			else
				direction.normalize();
			it->second[i]->setTargetPosition(it->second[i]->getScreenPosition() + 0.1f * direction);
		}
	}

	//	swap
	activeHud = name;
}
void WidgetManager::disableAllHUD()
{
	for (auto& it : hudList)
	{
		for (Layer* layer : it.second)
		{
			layer->setVisibility(false);

			vec4f direction = layer->getScreenPosition();
			if (direction == vec4f::zero)
				direction = vec4f(0.f, 0.f, 1.f, 0.f);
			else
				direction.normalize();

			vec4f p = layer->getScreenPosition() + 0.1f * direction;
			layer->setTargetPosition(p);
			layer->setPosition(p);
		}
	}
	activeHud = "";
}
void WidgetManager::setPickingParameters(const mat4f& base, const vec4f& ray)
{
	pickingRay = ray;
	pickingBase = base;
}


std::string WidgetManager::getActiveHUD() const { return activeHud; }
unsigned int WidgetManager::getNbDrawnWidgets() const { return widgetDrawn; }
unsigned int WidgetManager::getNbDrawnTriangles() const { return trianglesDrawn; }
bool WidgetManager::isUnderMouse() const { return !hoverWidgetList.empty(); }
const std::set<WidgetVirtual*>& WidgetManager::getActiveWidgets() const { return activeWidgetList; }
const std::vector<std::string>* WidgetManager::getWidgetAssociations(WidgetVirtual* widget)
{
	std::map<WidgetVirtual*, std::vector<std::string>>::iterator it = associationsWidgetToName.find(widget);
	if (it != associationsWidgetToName.end())
		return &(it->second);
	else return nullptr;
}
//


//	Callbacks functions
void WidgetManager::resizeCallback(GLFWwindow* window, int w, int h)
{
	float viewportRatio = (float) w / h;
	std::map<std::string, std::vector<Layer*> >& hudList = WidgetManager::getInstance()->hudList;
	for (std::map<std::string, std::vector<Layer*> >::iterator it = hudList.begin(); it != hudList.end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			Layer* l = it->second[i];
			vec4f p = l->getPosition();
			vec4f tp = l->getTargetPosition();
			vec4f sp = l->getScreenPosition();

			if (l->isResponsive())		// layer responsive
			{
				//	change position but keep ratio screen position (just on x)
				float ratio = (float) viewportRatio / WidgetManager::getInstance()->lastViewportRatio;
				l->setPosition(vec4f(p.x * ratio, p.y, p.z, 1.f));
				l->setTargetPosition(vec4f(tp.x * ratio, tp.y, tp.z, 1.f));
				l->setScreenPosition(vec4f(sp.x * ratio, sp.y, sp.z, 1.f));
			}
			else
			{
				//	change Layer position to be kept at this position relative to the border of screen
				float zscale = (float)tan(DEG2RAD * ANGLE_VERTICAL_HUD_PROJECTION * 0.5) * (p.y + DISTANCE_HUD_CAMERA);	//	compute relative scale on vertical axis (base because always 45 degres of view angle)
				float xscale = zscale * viewportRatio;														//	compute relative scale of x axis
				float lxscale = zscale * WidgetManager::getInstance()->lastViewportRatio;					//	compute previous relative scale of x axis
				if (p.x > 0)																				//	discriminate left or right border relative
					l->setPosition(vec4f(xscale - std::abs(p.x - lxscale), p.y, p.z, 1.f));					//	change position = border position -+ delta from border (delta = actual position - former scale)
				else l->setPosition(vec4f(-xscale + std::abs(-p.x - lxscale), p.y, p.z, 1.f));				//	change position 

				zscale = (float)tan(DEG2RAD * ANGLE_VERTICAL_HUD_PROJECTION * 0.5) * (tp.y + DISTANCE_HUD_CAMERA);
				xscale = zscale * viewportRatio;
				lxscale = zscale * WidgetManager::getInstance()->lastViewportRatio;
				if (tp.x > 0)
					l->setTargetPosition(vec4f(xscale - std::abs(tp.x - lxscale), tp.y, tp.z, 1.f));
				else l->setTargetPosition(vec4f(-xscale + std::abs(-tp.x - lxscale), tp.y, tp.z, 1.f));

				zscale = (float)tan(DEG2RAD * ANGLE_VERTICAL_HUD_PROJECTION * 0.5) * (l->getScreenPosition().y + DISTANCE_HUD_CAMERA);
				xscale = zscale * viewportRatio;
				lxscale = zscale * WidgetManager::getInstance()->lastViewportRatio;
				if (sp.x > 0)
					l->setScreenPosition(vec4f(xscale - std::abs(sp.x - lxscale), sp.y, sp.z, 1.f));
				else l->setScreenPosition(vec4f(-xscale + std::abs(-sp.x - lxscale), sp.y, sp.z, 1.f));
			}

			//	check all widgets attach to it
			std::vector<WidgetVirtual*>& list = it->second[i]->getChildrenList();
			for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
			{
				if ((*it2)->isResponsive())		// widget responsive
				{
					vec2f ds = (*it2)->getSize(WidgetVirtual::State::DEFAULT);
					vec2f as = (*it2)->getSize(WidgetVirtual::State::ACTIVE);
					vec2f hs = (*it2)->getSize(WidgetVirtual::State::HOVER);
					vec2f s = (*it2)->getSize(WidgetVirtual::State::CURRENT);

					float ratio = (float)viewportRatio / WidgetManager::getInstance()->lastViewportRatio;
					(*it2)->setSize(vec2f(ds.x * ratio, ds.y), WidgetVirtual::State::DEFAULT);
					(*it2)->setSize(vec2f(as.x * ratio, as.y),  WidgetVirtual::State::ACTIVE);
					(*it2)->setSize(vec2f(hs.x * ratio, hs.y),   WidgetVirtual::State::HOVER);
					(*it2)->setSize(vec2f(s.x * ratio, s.y), WidgetVirtual::State::CURRENT);
				}
			}
		}
	}
	WidgetManager::getInstance()->lastViewportRatio = viewportRatio;
}
//


//	Protected functions
void WidgetManager::generateDebugHud()
{
	const uint8_t visibleResponsive = (uint8_t)WidgetVirtual::OrphanFlags::VISIBLE | (uint8_t)WidgetVirtual::OrphanFlags::RESPONSIVE;

	//	top title
	WidgetBoard* board1 = new WidgetBoard(visibleResponsive);
		board1->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
		board1->setSize(vec2f(2.4f, 0.3f), WidgetVirtual::State::ALL);
		board1->initialize(0.02f, 0.1f, WidgetBoard::BOTTOM_LEFT | WidgetBoard::BOTTOM_RIGHT);
		board1->setColor(vec4f(0.5f, 0.f, 0.2f, 1.f), WidgetVirtual::State::ALL);
		board1->setColor(vec4f(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::State::HOVER);
		widgetList.insert(board1);
	WidgetLabel* label1 = new WidgetLabel(visibleResponsive);
		label1->setPosition(vec4f(0.f, 0.f, 0.f, 1.f), WidgetVirtual::State::ALL);
		label1->setSizeChar(0.15f);
		label1->setSize(vec2f(2.3f, 0.2f), WidgetVirtual::State::ALL);
		label1->setFont("Data Control");
		label1->initialize("Debug Hud", WidgetLabel::CLIPPING);
		widgetList.insert(label1);
	Layer* layer1 = new Layer();
		layer1->setSize(0.05f);
		layer1->setScreenPosition(vec4f(0.f, 0.003f, 0.054f, 1.f));
		layer1->setPosition(layer1->getScreenPosition());
		layer1->setTargetPosition(layer1->getScreenPosition());
		layer1->addChild(board1);
		layer1->addChild(label1);
		layerList.insert(layer1);

	//	runtime speed
	WidgetBoard* board2 = new WidgetBoard();
		board2->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
		board2->setSize(vec2f(0.8f, 0.5f), WidgetVirtual::State::ALL);
		board2->initialize(0.02f, 0.15f, WidgetBoard::BOTTOM_RIGHT);
		board2->setColor(vec4f(0.f, 0.f, 0.5f, 1.f), WidgetVirtual::State::ALL);
		board2->setColor(vec4f(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::State::HOVER);
		widgetList.insert(board2);
	WidgetLabel* label2 = new WidgetLabel();
		label2->setPosition(vec4f(0.f, 0.f, 0.f, 1.f), WidgetVirtual::State::ALL);
		label2->setSizeChar(0.07f);
		label2->setSize(vec2f(0.7f, 0.5f), WidgetVirtual::State::ALL);
		label2->setFont("Data Control");
		label2->initialize("FPS : na\n Avg : na\n\nTime : na ms\n Avg : na ms", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		widgetList.insert(label2);
		addAssociation(label2, "runtime speed");
	Layer* layer2 = new Layer(Layer::VISIBLE);
		layer2->setSize(0.05f);
		layer2->setScreenPosition(vec4f(-0.091f, 0.f, 0.049f, 1.f));
		layer2->setPosition(layer2->getScreenPosition());
		layer2->setTargetPosition(layer2->getScreenPosition());
		layer2->addChild(board2);
		layer2->addChild(label2);
		layerList.insert(layer2);

	//	Draw calls
	WidgetBoard* board3 = new WidgetBoard();
		board3->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
		board3->setSize(vec2f(0.8f, 0.5f), WidgetVirtual::State::ALL);
		board3->initialize(0.02f, 0.15f, WidgetBoard::BOTTOM_LEFT);
		board3->setColor(vec4f(0.8f, 0.f, 0.f, 1.f), WidgetVirtual::State::ALL);
		board3->setColor(vec4f(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::State::HOVER);
		widgetList.insert(board3);
	WidgetLabel* label3 = new WidgetLabel();
		label3->setPosition(vec4f(0.f, 0.f, 0.f, 1.f), WidgetVirtual::State::ALL);
		label3->setSizeChar(0.07f);
		label3->setSize(vec2f(0.7f, 0.5f), WidgetVirtual::State::ALL);
		label3->setFont("Data Control");
		label3->initialize("Instances :\n0\n\nTriangles :\n0", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		addAssociation(label3, "drawcalls");
		widgetList.insert(label3);
	Layer* layer3 = new Layer(Layer::VISIBLE);
		layer3->setSize(0.05f);
		layer3->setScreenPosition(vec4f(0.091f, 0.f, 0.049f, 1.f));
		layer3->setPosition(layer3->getScreenPosition());
		layer3->setTargetPosition(layer3->getScreenPosition());
		layer3->addChild(board3);
		layer3->addChild(label3);
		layerList.insert(layer3);

	//	console
	WidgetConsole* console = new WidgetConsole(visibleResponsive);
		console->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
		console->setSize(vec2f(2.1f, 0.5f), WidgetVirtual::State::ALL);
		console->setSizeChar(0.07f);
		console->setFont("Data Control");
		console->initialize(0.02f, 0.15f, WidgetBoard::TOP_RIGHT);
		console->setColor(vec4f(0.5f, 0.5f, 0.f, 1.f), WidgetVirtual::State::ALL);
		console->setColor(vec4f(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::State::HOVER);
		console->setColor(vec4f(0.2f, 0.2f, 0.2f, 1.f), WidgetVirtual::State::ACTIVE);
		addAssociation(console, "console");
		widgetList.insert(console);
	Layer* layer4 = new Layer();
		layer4->setSize(0.05f);
		layer4->setScreenPosition(vec4f(-0.055f, 0.f, -0.049f, 1.f));
		layer4->setPosition(layer4->getScreenPosition());
		layer4->setTargetPosition(layer4->getScreenPosition());
		layer4->addChild(console);
		layerList.insert(layer4);

	//	picking
	WidgetBoard* board5 = new WidgetBoard(visibleResponsive);
		board5->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
		board5->setSize(vec2f(2.1f, 0.5f), WidgetVirtual::State::ALL);
		board5->initialize(0.02f, 0.15f, WidgetBoard::TOP_LEFT);
		board5->setColor(vec4f(0.f, 0.5f, 0.f, 1.f), WidgetVirtual::State::ALL);
		board5->setColor(vec4f(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::State::HOVER);
		widgetList.insert(board5);
	WidgetLabel* label5 = new WidgetLabel(visibleResponsive);
		label5->setPosition(vec4f(0.01f, 0.f, -0.01f, 1.f), WidgetVirtual::State::ALL);
		label5->setSizeChar(0.07f);
		label5->setSize(vec2f(2.f, 0.35f), WidgetVirtual::State::ALL);
		label5->setFont("Data Control");
		label5->initialize("Distance : 50.06 m\nPosition : (923.1 , 584.6 , 1.2)\nInstance on ray : 5\nFirst instance pointed id : 23681\n  type : animated", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		addAssociation(label5, "interaction");
		widgetList.insert(label5);
	Layer* layer5 = new Layer();
		layer5->setSize(0.05f);
		layer5->setScreenPosition(vec4f(0.055f, 0.f, -0.049f, 1.f));
		layer5->setPosition(layer5->getScreenPosition());
		layer5->setTargetPosition(layer5->getScreenPosition());
		layer5->addChild(board5);
		layer5->addChild(label5);
		layerList.insert(layer5);
		
	//	hit cross
	WidgetImage* image = new WidgetImage("hitcross.png");
		image->setPosition(vec4f(0.f, 0.2f, 0.f, 1.f), WidgetVirtual::State::ALL);
		image->setSize(vec2f(0.1f, 0.1f), WidgetVirtual::State::ALL);
		image->initialize();
		widgetList.insert(image);
	Layer* layer6 = new Layer(Layer::VISIBLE | Layer::RESPONSIVE);
		layer6->setSize(0.05f);
		layer6->setScreenPosition(vec4f(0.f, 0.f, 0.f, 1.f));
		layer6->setPosition(layer6->getScreenPosition());
		layer6->setTargetPosition(layer6->getScreenPosition());
		layer6->addChild(image);
		layerList.insert(layer6);

	//	push HUD
	hudList["debug"].push_back(layer1);
	hudList["debug"].push_back(layer2);
	hudList["debug"].push_back(layer3);
	hudList["debug"].push_back(layer4);
	hudList["debug"].push_back(layer5);
	hudList["debug"].push_back(layer6);
}
void WidgetManager::generateHelpHud()
{
	const uint8_t visibleResponsive = (uint8_t)WidgetVirtual::OrphanFlags::VISIBLE | (uint8_t)WidgetVirtual::OrphanFlags::RESPONSIVE;

	//	top title
	WidgetBoard* board1 = new WidgetBoard(visibleResponsive);
		board1->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
		board1->setSize(vec2f(2.4f, 0.3f), WidgetVirtual::State::ALL);
		board1->initialize(0.02f, 0.1f, WidgetBoard::BOTTOM_LEFT | WidgetBoard::BOTTOM_RIGHT);
		board1->setColor(vec4f(0.5f, 0.f, 0.2f, 1.f), WidgetVirtual::State::ALL);
		board1->setColor(vec4f(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::State::HOVER);
		widgetList.insert(board1);
	WidgetLabel* label1 = new WidgetLabel(visibleResponsive);
		label1->setPosition(vec4f(0.f, 0.f, 0.f, 1.f), WidgetVirtual::State::ALL);
		label1->setSizeChar(0.15f);
		label1->setSize(vec2f(2.3f, 0.2f), WidgetVirtual::State::ALL);
		label1->setFont("Data Control");
		label1->initialize("Help / command", WidgetLabel::CLIPPING);
		widgetList.insert(label1);
	Layer* layer1 = new Layer();
		layer1->setSize(0.05f);
		layer1->setScreenPosition(vec4f(0.f, 0.003f, 0.054f, 1.f));
		layer1->setPosition(layer1->getScreenPosition());
		layer1->setTargetPosition(layer1->getScreenPosition());
		layer1->addChild(board1);
		layer1->addChild(label1);
		layerList.insert(layer1);

	//	central panel
	WidgetConsole* console = new WidgetConsole(visibleResponsive);
		console->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
		console->setSize(vec2f(2.6f, 2.f), WidgetVirtual::State::ALL);
		console->setSizeChar(0.07f);
		console->setMargin(0.15f);
		console->setFont("Data Control");
		console->initialize(0.02f, 0.15f, WidgetBoard::TOP_RIGHT | WidgetBoard::TOP_LEFT | WidgetBoard::BOTTOM_RIGHT | WidgetBoard::BOTTOM_LEFT);
		console->setColor(vec4f(0.f,  0.5f,  0.f, 1.f), WidgetVirtual::State::ALL);
		console->setColor(vec4f(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::State::HOVER);
		console->setColor(vec4f(0.2f, 0.2f, 0.2f, 1.f), WidgetVirtual::State::ACTIVE);
		widgetList.insert(console);
	Layer* layer2 = new Layer(Layer::VISIBLE | Layer::RESPONSIVE);
		layer2->setSize(0.05f);
		layer2->setScreenPosition(vec4f(0.f, 0.f, -0.01f, 1.f));
		layer2->setPosition(layer2->getScreenPosition());
		layer2->setTargetPosition(layer2->getScreenPosition());
		layer2->addChild(console);
		layerList.insert(layer2);

	//	push on HUD
	hudList["help"].push_back(layer1);
	hudList["help"].push_back(layer2);
}
void WidgetManager::generateRenderingHud()
{
	const uint8_t visibleResponsive = (uint8_t)WidgetVirtual::OrphanFlags::VISIBLE | (uint8_t)WidgetVirtual::OrphanFlags::RESPONSIVE;

	//	top title
	WidgetBoard* board1 = new WidgetBoard(visibleResponsive);
		board1->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
		board1->setSize(vec2f(2.4f, 0.3f), WidgetVirtual::State::ALL);
		board1->initialize(0.02f, 0.1f, WidgetBoard::BOTTOM_LEFT | WidgetBoard::BOTTOM_RIGHT);
		board1->setColor(vec4f(0.5f, 0.f, 0.2f, 1.f), WidgetVirtual::State::ALL);
		board1->setColor(vec4f(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::State::HOVER);
		widgetList.insert(board1);
	WidgetLabel* label1 = new WidgetLabel(visibleResponsive);
		label1->setPosition(vec4f(0.f, 0.f, 0.f, 1.f), WidgetVirtual::State::ALL);
		label1->setSizeChar(0.15f);
		label1->setSize(vec2f(2.3f, 0.2f), WidgetVirtual::State::ALL);
		label1->setFont("Data Control");
		label1->initialize("Rendering options", WidgetLabel::CLIPPING);
		widgetList.insert(label1);
	Layer* layer1 = new Layer();
		layer1->setSize(0.05f);
		layer1->setScreenPosition(vec4f(0.f, 0.003f, 0.054f, 1.f));
		layer1->setPosition(layer1->getScreenPosition());
		layer1->setTargetPosition(layer1->getScreenPosition());
		layer1->addChild(board1);
		layer1->addChild(label1);
		layerList.insert(layer1);

	//	panel left
	Layer* layer2;
	{
		WidgetBoard* board2 = new WidgetBoard(visibleResponsive);
			board2->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
			board2->setSize(vec2f(2.1f, 1.8f), WidgetVirtual::State::ALL);
			board2->initialize(0.02f, 0.15f, WidgetBoard::TOP_LEFT | WidgetBoard::BOTTOM_LEFT);
			board2->setColor(vec4f(0.f, 0.f, 0.5f, 1.f), WidgetVirtual::State::ALL);
			board2->setColor(vec4f(0.4f, 0.4f, 0.4f, 1.f), WidgetVirtual::State::HOVER);
			board2->setColor(vec4f(0.2f, 0.2f, 0.2f, 1.f), WidgetVirtual::State::ACTIVE);
			widgetList.insert(board2);

		WidgetRadioButton* button1 = new WidgetRadioButton(visibleResponsive);
			button1->setPosition(vec4f(0.f, 0.f, 0.7f, 1.f), WidgetVirtual::State::ALL);
			button1->setSizeChar(0.07f);
			button1->setSize(vec2f(2.f, 0.07f), WidgetVirtual::State::ALL);
			button1->setFont("Data Control");
			button1->setTextureOn("checkbox_checked.png");
			button1->setTextureOff("checkbox_unchecked.png");
			button1->setColor(vec4f(0.8f, 0.3f, 0.3f, 1.f), WidgetVirtual::State::HOVER);
			button1->setColor(vec4f(0.8f, 0.3f, 0.3f, 1.f), WidgetVirtual::State::ACTIVE);
			button1->initialize("bounding box rendering", WidgetLabel::CLIPPING | WidgetLabel::LEFT);
			addAssociation(button1, "BBrendering");
			widgetList.insert(button1);

		WidgetRadioButton* button2 = new WidgetRadioButton(visibleResponsive);
			button2->setPosition(vec4f(0.f, 0.f, 0.62f, 1.f), WidgetVirtual::State::ALL);
			button2->setSizeChar(0.07f);
			button2->setSize(vec2f(2.f, 0.07f), WidgetVirtual::State::ALL);
			button2->setFont("Data Control");
			button2->setTextureOn("checkbox_checked.png");
			button2->setTextureOff("checkbox_unchecked.png");
			button2->setColor(vec4f(0.8f, 0.3f, 0.3f, 1.f), WidgetVirtual::State::HOVER);
			button2->setColor(vec4f(0.8f, 0.3f, 0.3f, 1.f), WidgetVirtual::State::ACTIVE);
			button2->setBoolean(true);
			button2->initialize("draw bounding box on picked instance", WidgetLabel::CLIPPING | WidgetLabel::LEFT);
			addAssociation(button2, "BBpicking");
			widgetList.insert(button2);

		WidgetRadioButton* button3 = new WidgetRadioButton(visibleResponsive);
			button3->setPosition(vec4f(0.f, 0.f, 0.54f, 1.f), WidgetVirtual::State::ALL);
			button3->setSizeChar(0.07f);
			button3->setSize(vec2f(2.f, 0.07f), WidgetVirtual::State::ALL);
			button3->setFont("Data Control");
			button3->setTextureOn("checkbox_checked.png");
			button3->setTextureOff("checkbox_unchecked.png");
			button3->setColor(vec4f(0.8f, 0.3f, 0.3f, 1.f), WidgetVirtual::State::HOVER);
			button3->setColor(vec4f(0.8f, 0.3f, 0.3f, 1.f), WidgetVirtual::State::ACTIVE);
			button3->setBoolean(true);
			button3->initialize("synchronize render camera", WidgetLabel::CLIPPING | WidgetLabel::LEFT);
			addAssociation(button3, "syncCamera");
			widgetList.insert(button3);

		WidgetRadioButton* button4 = new WidgetRadioButton(visibleResponsive);
			button4->setPosition(vec4f(0.f, 0.f, 0.46f, 1.f), WidgetVirtual::State::ALL);
			button4->setSizeChar(0.07f);
			button4->setSize(vec2f(2.f, 0.07f), WidgetVirtual::State::ALL);
			button4->setFont("Data Control");
			button4->setTextureOn("checkbox_checked.png");
			button4->setTextureOff("checkbox_unchecked.png");
			button4->setColor(vec4f(0.8f, 0.3f, 0.3f, 1.f), WidgetVirtual::State::HOVER);
			button4->setColor(vec4f(0.8f, 0.3f, 0.3f, 1.f), WidgetVirtual::State::ACTIVE);
			button4->setBoolean(false);
			button4->initialize("render in wire frame mode", WidgetLabel::CLIPPING | WidgetLabel::LEFT);
			addAssociation(button4, "wireframe");
			widgetList.insert(button2);

		layer2 = new Layer();
			layer2->setSize(0.05f);
			layer2->setScreenPosition(vec4f(-0.055f, 0.f, -0.01f, 1.f));
			layer2->setPosition(layer2->getScreenPosition());
			layer2->setTargetPosition(layer2->getScreenPosition());
			layer2->addChild(board2);
			layer2->addChild(button1);
			layer2->addChild(button2);
			layer2->addChild(button3);
			layer2->addChild(button4);
			layerList.insert(layer2);
	}

	//	panel right
	Layer* layer3;
	{
		WidgetBoard* board2 = new WidgetBoard((uint8_t)WidgetVirtual::OrphanFlags::VISIBLE | (uint8_t)WidgetVirtual::OrphanFlags::RESPONSIVE);
			board2->setPosition(vec4f(0.f, 0.01f, 0.f, 1.f), WidgetVirtual::State::ALL);
			board2->setSize(vec2f(2.1f, 1.8f), WidgetVirtual::State::ALL);
			board2->initialize(0.02f, 0.15f, WidgetBoard::TOP_RIGHT | WidgetBoard::BOTTOM_RIGHT);
			board2->setColor(vec4f(0.f, 0.f, 0.5f, 1.f), WidgetVirtual::State::ALL);
			board2->setColor(vec4f(0.4f, 0.4f, 0.4f, 1.f), WidgetVirtual::State::HOVER);
			board2->setColor(vec4f(0.2f, 0.2f, 0.2f, 1.f), WidgetVirtual::State::ACTIVE);
			widgetList.insert(board2);
		layer3 = new Layer();
			layer3->setSize(0.05f);
			layer3->setScreenPosition(vec4f(0.055f, 0.f, -0.01f, 1.f));
			layer3->setPosition(layer3->getScreenPosition());
			layer3->setTargetPosition(layer3->getScreenPosition());
			layer3->addChild(board2);
			layerList.insert(layer3);
	}

	//	push HUD
	hudList["rendering"].push_back(layer1);
	hudList["rendering"].push_back(layer2);
	hudList["rendering"].push_back(layer3);
	
	/*return;

	//	save into file
	std::ofstream file(ResourceManager::getInstance()->getRepository() + "GUI/rendering.gui", std::ofstream::out);
	Writer writer(&file);
	file << std::fixed;
	file.precision(5);
	writer.setInlineArray(true);
	int startingUnknownIndex = 0;
	Variant hud; hud.createMap();

	for (unsigned int i = 0; i < hudList["rendering"].size(); ++i)
	{
		Variant tmp = WidgetSaver::serialize(hudList["rendering"][i], associations, startingUnknownIndex);
		hud.insert("layer_" + std::to_string(i + 1), tmp);
	}
	writer.write(hud);*/
}
//
