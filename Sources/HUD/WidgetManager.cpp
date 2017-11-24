#include "WidgetManager.h"


//  Default
WidgetManager::WidgetManager() : widgetDrawn(0), trianglesDrawn(0), pickingRay(0.f) {}
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
	loadDebugHud();
}
void WidgetManager::draw(Shader* s, const glm::mat4& base, const float* view, const float* projection)
{
	//	change opengl states
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	uint8_t stencilMask = 0x00;
	widgetDrawn = 0;
	trianglesDrawn = 0;

	//	draw all widget in hudList
	for (std::map<std::string, std::vector<Layer*> >::iterator it = hudList.begin(); it != hudList.end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			if (it->second[i]->isVisible())		// layer visible
			{
				glm::mat4 model = base * it->second[i]->getModelMatrix();
				std::vector<WidgetVirtual*>& list = it->second[i]->getWidgetList();
				for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
				{
					if ((*it2)->isVisible())	// widget visible
					{
						//	Get shader
						Shader* shader;
						if (!s) shader = (*it2)->getShader();
						if (!shader) return;
						shader->enable();

						//	Enable mvp matrix
						int loc = shader->getUniformLocation("model");
						if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, &glm::translate(model, (*it2)->getPosition())[0][0]);
						loc = shader->getUniformLocation("view");
						if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, view);
						loc = shader->getUniformLocation("projection");
						if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, projection);

						//	Draw
						(*it2)->draw(shader, stencilMask);
						widgetDrawn++;
						trianglesDrawn += (*it2)->getNumberFaces();
					}
				}
			}
		}
	}
	glDisable(GL_BLEND);
}
void WidgetManager::update(const float& elapsedTime, const bool& clickButtonPressed)
{
	//	picking
	glm::vec3 intersection;
	std::set<WidgetVirtual*> tmpWidgetList;
	if (pickingRay != glm::vec3(0.f))
	{
		//	draw all widget in hudList
		for (std::map<std::string, std::vector<Layer*> >::iterator it = hudList.begin(); it != hudList.end(); ++it)
			for (unsigned int i = 0; i < it->second.size(); i++)
				if (it->second[i]->isVisible())		// layer visible
				{
					glm::mat4 model = pickingBase * it->second[i]->getModelMatrix();
					std::vector<WidgetVirtual*>& list = it->second[i]->getWidgetList();
					for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
					{
						if ((*it2)->isVisible())	// widget visible
						{
							if ((*it2)->intersect(glm::translate(model, (*it2)->getPosition()), pickingRay, pickingOrigin, intersection))
								tmpWidgetList.insert(*it2);
						}
					}
				}
	}

	//	trigger new hover widget and widget leaving hover list
	for (std::set<WidgetVirtual*>::iterator it = tmpWidgetList.begin(); it != tmpWidgetList.end(); ++it)
	{
		if (hoverWidgetList.find(*it) == hoverWidgetList.end())
		{
			(*it)->setState(WidgetVirtual::HOVER);
		}
	}
	for (std::set<WidgetVirtual*>::iterator it = hoverWidgetList.begin(); it != hoverWidgetList.end(); ++it)
	{
		//	widget not hover anymore and not active anymore
		if (tmpWidgetList.find(*it) == tmpWidgetList.end() && std::find(activeWidgetList.begin(), activeWidgetList.end(), *it) == activeWidgetList.end())
			(*it)->setState(WidgetVirtual::DEFAULT);
	}
	hoverWidgetList.swap(tmpWidgetList);

	//	widget hover and a click occur -> add widget to active list if possible
	if (clickButtonPressed)
		for (std::set<WidgetVirtual*>::iterator it = hoverWidgetList.begin(); it != hoverWidgetList.end(); ++it)
		{
			if((*it)->mouseEvent(intersection, clickButtonPressed))
				activeWidgetList.insert(activeWidgetList.end(), *it);
		}
	
	//	special update on active widget
	for (std::list<WidgetVirtual*>::iterator it = activeWidgetList.begin(); it != activeWidgetList.end();)
	{
		if((*it)->mouseEvent(intersection, clickButtonPressed)) ++it;
		else 
		{
			if (hoverWidgetList.find(*it) == hoverWidgetList.end()) (*it)->setState(WidgetVirtual::DEFAULT);
			else (*it)->setState(WidgetVirtual::HOVER);
			it = activeWidgetList.erase(it);
		}
	}

	//	update all layers and widgets
	for (std::set<Layer*>::iterator it = layerList.begin(); it != layerList.end(); ++it)
		(*it)->update(elapsedTime);
	for (std::set<WidgetVirtual*>::iterator it = widgetList.begin(); it != widgetList.end(); ++it)
		(*it)->update(elapsedTime);

	//	end
	lastClickButtonState = clickButtonPressed;
}


void WidgetManager::addAssociation(WidgetVirtual* w, const std::string& associationName)
{
	associations[associationName] = w;
}
void  WidgetManager::setString(const std::string& associationName, const std::string& s)
{
	try
	{
		WidgetVirtual* w = associations.at(associationName);
		if (w) w->setString(s);
	}
	catch (const std::out_of_range &) {}
}
std::string  WidgetManager::getString(const std::string& associationName)
{
	try
	{
		WidgetVirtual* w = associations.at(associationName);
		if (w) return w->getString();
	}
	catch (const std::out_of_range &) {}
}
std::stringstream*  WidgetManager::getStream(const std::string& associationName)
{
	try
	{
		WidgetVirtual* w = associations.at(associationName);
		if (w) return w->getStream();
	}
	catch (const std::out_of_range &) {}
}


void WidgetManager::addWidget(WidgetVirtual* w) { widgetList.insert(w); }
void WidgetManager::removeWidget(WidgetVirtual* w)
{
	widgetList.erase(w);
	for (auto it = associations.begin(); it != associations.end(); )
	{
		if (it->second == w)
			it = associations.erase(it);
		else ++it;
	}
}
void WidgetManager::addLayer(Layer* l) { layerList.insert(l); }
void WidgetManager::removeLayer(Layer* l) { layerList.erase(l); }
//

//	Set / get functions
void WidgetManager::setInitialWindowSize(const int& width, const int& height)
{
	lastWidth = width;
	lastHeight = height;
}
void WidgetManager::setActiveHUD(const std::string& s)
{
	//	activate new HUD layer
	std::map<std::string, std::vector<Layer*> >::iterator it = hudList.find(s);
	if (it != hudList.end())
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			it->second[i]->setVisibility(true);
			it->second[i]->setTargetPosition(it->second[i]->getScreenPosition());
		}
	}

	//	push all widget from former HUD
	it = hudList.find(activeHud);
	if (it != hudList.end())
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			glm::vec3 direction = it->second[i]->getScreenPosition();
			if (direction == glm::vec3(0.f)) direction = glm::vec3(0.f, 0.f, 1.f);
			it->second[i]->setTargetPosition(it->second[i]->getScreenPosition() + 0.1f * glm::normalize(direction));
		}
	}

	//	swap
	activeHud = s;
}
void WidgetManager::setPickingParameters(const glm::mat4& base, const glm::vec3& ray, const glm::vec3& origin)
{
	pickingRay = ray;
	pickingBase = base;
	pickingOrigin = origin;
}


std::string WidgetManager::getActiveHUD() const { return activeHud; }
unsigned int WidgetManager::getNbDrawnWidgets() const { return widgetDrawn; }
unsigned int WidgetManager::getNbDrawnTriangles() const { return trianglesDrawn; }
//

//	callbacks functions
void WidgetManager::resizeCallback(int w, int h)
{
	std::map<std::string, std::vector<Layer*> >& hudList = WidgetManager::getInstance()->hudList;
	for (std::map<std::string, std::vector<Layer*> >::iterator it = hudList.begin(); it != hudList.end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			Layer* l = it->second[i];
			if (l->isResponsive())		// layer responsive
			{
				//	change position but keep ratio screen position (just on x)
				float ratio = (float) WidgetManager::getInstance()->lastHeight * w / h / WidgetManager::getInstance()->lastWidth;
				l->setPosition(glm::vec3(l->getPosition().x * ratio, l->getPosition().y, l->getPosition().z));
				l->setTargetPosition(glm::vec3(l->getTargetPosition().x * ratio, l->getTargetPosition().y, l->getTargetPosition().z));
				l->setScreenPosition(glm::vec3(l->getScreenPosition().x * ratio, l->getScreenPosition().y, l->getScreenPosition().z));
			}
			else
			{
				//	change Layer position to be kept at this position relative to the border of screen

				float zscale = tan(glm::radians(ANGLE_VERTICAL_HUD_PROJECTION / 2.f)) * (l->getPosition().y + DISTANCE_HUD_CAMERA);													//	compute relative scale on vertical axis (base because always 45 degres of view angle)
				float xscale = zscale * w / h;																									//	compute relative scale of x axis
				float lxscale = zscale * WidgetManager::getInstance()->lastWidth / WidgetManager::getInstance()->lastHeight;					//	compute previous relative scale of x axis
				if (l->getPosition().x > 0)																										//	discriminate left or right border relative
					l->setPosition(glm::vec3(xscale - std::abs(l->getPosition().x - lxscale), l->getPosition().y, l->getPosition().z));			//	change position = border position -+ delta from border (delta = actual position - former scale)
				else l->setPosition(glm::vec3(-xscale + std::abs(-l->getPosition().x - lxscale), l->getPosition().y, l->getPosition().z));		//	change position 

				zscale = tan(glm::radians(ANGLE_VERTICAL_HUD_PROJECTION / 2.f)) * (l->getTargetPosition().y + DISTANCE_HUD_CAMERA);
				xscale = zscale * w / h;
				lxscale = zscale * WidgetManager::getInstance()->lastWidth / WidgetManager::getInstance()->lastHeight;
				if (l->getTargetPosition().x > 0)
					l->setTargetPosition(glm::vec3(xscale - std::abs(l->getTargetPosition().x - lxscale), l->getTargetPosition().y, l->getTargetPosition().z));
				else l->setTargetPosition(glm::vec3(-xscale + std::abs(-l->getTargetPosition().x - lxscale), l->getTargetPosition().y, l->getTargetPosition().z));

				zscale = tan(glm::radians(ANGLE_VERTICAL_HUD_PROJECTION / 2.f)) * (l->getScreenPosition().y + DISTANCE_HUD_CAMERA);
				xscale = zscale * w / h;
				lxscale = zscale * WidgetManager::getInstance()->lastWidth / WidgetManager::getInstance()->lastHeight;
				if (l->getScreenPosition().x > 0)
					l->setScreenPosition(glm::vec3(xscale - std::abs(l->getScreenPosition().x - lxscale), l->getScreenPosition().y, l->getScreenPosition().z));
				else l->setScreenPosition(glm::vec3(-xscale + std::abs(-l->getScreenPosition().x - lxscale), l->getScreenPosition().y, l->getScreenPosition().z));
			}

			//	check all widgets attach to it
			std::vector<WidgetVirtual*>& list = it->second[i]->getWidgetList();
			for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
			{
				if ((*it2)->isResponsive())		// widget responsive
				{
					float ratio = (float)WidgetManager::getInstance()->lastHeight * w / h / WidgetManager::getInstance()->lastWidth;
					(*it2)->setSize(glm::vec2((*it2)->getSize(WidgetVirtual::DEFAULT).x * ratio, (*it2)->getSize(WidgetVirtual::DEFAULT).y), WidgetVirtual::DEFAULT);
					(*it2)->setSize(glm::vec2((*it2)->getSize(WidgetVirtual::ACTIVE).x * ratio,  (*it2)->getSize(WidgetVirtual::ACTIVE).y),  WidgetVirtual::ACTIVE);
					(*it2)->setSize(glm::vec2((*it2)->getSize(WidgetVirtual::HOVER).x * ratio,   (*it2)->getSize(WidgetVirtual::HOVER).y),   WidgetVirtual::HOVER);
					(*it2)->setSize(glm::vec2((*it2)->getSize(WidgetVirtual::CURRENT).x * ratio, (*it2)->getSize(WidgetVirtual::CURRENT).y), WidgetVirtual::CURRENT);
				}
			}
		}
	}
	WidgetManager::getInstance()->lastWidth = w;
	WidgetManager::getInstance()->lastHeight = h;
}
//

//	Protected functions
void WidgetManager::loadDebugHud()
{
	//	top title
	WidgetBoard* board1 = new WidgetBoard(WidgetVirtual::VISIBLE | WidgetVirtual::RESPONSIVE);
		board1->setPosition(glm::vec3(0.f, 0.01f, 0.f), WidgetVirtual::ALL);
		board1->setSize(glm::vec2(2.4f, 0.3f), WidgetVirtual::ALL);
		board1->initialize(0.02f, 0.1f, WidgetBoard::BOTTOM_LEFT | WidgetBoard::BOTTOM_RIGHT);
		board1->setColor(glm::vec4(0.5f, 0.f, 0.2f, 1.f), WidgetVirtual::ALL);
		board1->setColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::HOVER);
		widgetList.insert(board1);
	WidgetLabel* label1 = new WidgetLabel(WidgetVirtual::VISIBLE | WidgetVirtual::RESPONSIVE);
		label1->setPosition(glm::vec3(0.f, 0.f, 0.f), WidgetVirtual::ALL);
		label1->setSizeChar(0.15f);
		label1->setSize(glm::vec2(2.3f, 0.2f), WidgetVirtual::ALL);
		label1->setFont("Data Control");
		label1->initialize("Debug Hud", WidgetLabel::CLIPPING);
		widgetList.insert(label1);
	Layer* layer1 = new Layer();
		layer1->setSize(0.05f);
		layer1->setScreenPosition(glm::vec3(0.f, 0.003f, 0.054f));
		layer1->setPosition(layer1->getScreenPosition());
		layer1->setTargetPosition(layer1->getScreenPosition());
		layer1->add(board1);
		layer1->add(label1);
		layerList.insert(layer1);

	//	runtime speed
	WidgetBoard* board2 = new WidgetBoard();
		board2->setPosition(glm::vec3(0.f, 0.01f, 0.f), WidgetVirtual::ALL);
		board2->setSize(glm::vec2(0.8f, 0.5f), WidgetVirtual::ALL);
		board2->initialize(0.02f, 0.15f, WidgetBoard::BOTTOM_RIGHT);
		board2->setColor(glm::vec4(0.f, 0.f, 0.5f, 1.f), WidgetVirtual::ALL);
		board2->setColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::HOVER);
		widgetList.insert(board2);
	WidgetLabel* label2 = new WidgetLabel();
		label2->setPosition(glm::vec3(0.03f, 0.f, 0.f), WidgetVirtual::ALL);
		label2->setSizeChar(0.07f);
		label2->setSize(glm::vec2(0.7f, 0.5f), WidgetVirtual::ALL);
		label2->setFont("Data Control");
		label2->initialize("FPS : 60\n Avg : 60\n\nTime : 9ms\n Avg : 11ms", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		widgetList.insert(label2);
		addAssociation(label2, "runtime speed");
	Layer* layer2 = new Layer(Layer::VISIBLE);
		layer2->setSize(0.05f);
		layer2->setScreenPosition(glm::vec3(-0.091f, 0.f, 0.049f));
		layer2->setPosition(layer2->getScreenPosition());
		layer2->setTargetPosition(layer2->getScreenPosition());
		layer2->add(board2);
		layer2->add(label2);
		layerList.insert(layer2);

	//	Draw calls
	WidgetBoard* board3 = new WidgetBoard();
		board3->setPosition(glm::vec3(0.f, 0.01f, 0.f), WidgetVirtual::ALL);
		board3->setSize(glm::vec2(0.8f, 0.5f), WidgetVirtual::ALL);
		board3->initialize(0.02f, 0.15f, WidgetBoard::BOTTOM_LEFT);
		board3->setColor(glm::vec4(0.8f, 0.f, 0.f, 1.f), WidgetVirtual::ALL);
		board3->setColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::HOVER);
		widgetList.insert(board3);
	WidgetLabel* label3 = new WidgetLabel();
		label3->setPosition(glm::vec3(0.025f, 0.f, 0.f), WidgetVirtual::ALL);
		label3->setSizeChar(0.07f);
		label3->setSize(glm::vec2(0.7f, 0.5f), WidgetVirtual::ALL);
		label3->setFont("Data Control");
		label3->initialize("Instances :\n80000\n\nTriangles :\n10000000", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		addAssociation(label3, "drawcalls");
		widgetList.insert(label3);
	Layer* layer3 = new Layer(Layer::VISIBLE);
		layer3->setSize(0.05f);
		layer3->setScreenPosition(glm::vec3(0.091f, 0.f, 0.049f));
		layer3->setPosition(layer3->getScreenPosition());
		layer3->setTargetPosition(layer3->getScreenPosition());
		layer3->add(board3);
		layer3->add(label3);
		layerList.insert(layer3);

	//	console
	WidgetBoard* board4 = new WidgetBoard(WidgetVirtual::VISIBLE | WidgetVirtual::RESPONSIVE);
		board4->setPosition(glm::vec3(0.f, 0.01f, 0.f), WidgetVirtual::ALL);
		board4->setSize(glm::vec2(2.1f, 0.5f), WidgetVirtual::ALL);
		board4->initialize(0.02f, 0.1f, WidgetBoard::TOP_RIGHT);
		board4->setColor(glm::vec4(0.5f, 0.5f, 0.f, 1.f), WidgetVirtual::ALL);
		board4->setColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::HOVER);
		widgetList.insert(board4);
	WidgetLabel* label4 = new WidgetLabel(WidgetVirtual::VISIBLE | WidgetVirtual::RESPONSIVE);
		label4->setPosition(glm::vec3(0.0f, 0.f, 0.f), WidgetVirtual::ALL);
		label4->setSizeChar(0.055f);
		label4->setSize(glm::vec2(2.f, 0.35f), WidgetVirtual::ALL);
		label4->setFont("Data Control");
		label4->initialize("debug consol test 3000\nWARRNING : this game is too awesome\n   this could cause some crash !\npeasant says : hello\npeasant says : that's true this game is awesome\n\ndebug consol test 3000", WidgetLabel::LEFT | WidgetLabel::BOTTOM | WidgetLabel::CLIPPING);
		addAssociation(label4, "console");
		widgetList.insert(label4);
	Layer* layer4 = new Layer();
		layer4->setSize(0.05f);
		layer4->setScreenPosition(glm::vec3(-0.055f, 0.f, -0.049f));
		layer4->setPosition(layer4->getScreenPosition());
		layer4->setTargetPosition(layer4->getScreenPosition());
		layer4->add(board4);
		layer4->add(label4);
		layerList.insert(layer4);

	//	picking
	WidgetBoard* board5 = new WidgetBoard(WidgetVirtual::VISIBLE | WidgetVirtual::RESPONSIVE);
		board5->setPosition(glm::vec3(0.f, 0.01f, 0.f), WidgetVirtual::ALL);
		board5->setSize(glm::vec2(2.1f, 0.5f), WidgetVirtual::ALL);
		board5->initialize(0.02f, 0.1f, WidgetBoard::TOP_LEFT);
		board5->setColor(glm::vec4(0.f, 0.5f, 0.f, 1.f), WidgetVirtual::ALL);
		board5->setColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::HOVER);
		widgetList.insert(board5);
	WidgetLabel* label5 = new WidgetLabel(WidgetVirtual::VISIBLE | WidgetVirtual::RESPONSIVE);
		label5->setPosition(glm::vec3(0.0f, 0.f, 0.f), WidgetVirtual::ALL);
		label5->setSizeChar(0.07f);
		label5->setSize(glm::vec2(2.f, 0.35f), WidgetVirtual::ALL);
		label5->setFont("Data Control");
		label5->initialize("Distance : 50.06 m\nPosition : (923.1 , 584.6 , 1.2)\nInstance on ray : 5\nFirst instance pointed id : 23681\n  type : animated", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		addAssociation(label5, "interaction");
		widgetList.insert(label5);
	Layer* layer5 = new Layer();
		layer5->setSize(0.05f);
		layer5->setScreenPosition(glm::vec3(0.055f, 0.f, -0.049f));
		layer5->setPosition(layer5->getScreenPosition());
		layer5->setTargetPosition(layer5->getScreenPosition());
		layer5->add(board5);
		layer5->add(label5);
		layerList.insert(layer5);

	//	test
	WidgetConsole* console = new WidgetConsole();
		console->setPosition(glm::vec3(0.f, 0.01f, 0.f), WidgetVirtual::ALL);
		console->setSize(glm::vec2(2.1f, 0.5f), WidgetVirtual::ALL);
		console->initialize(0.02f, 0.1f, WidgetBoard::TOP_RIGHT);
		console->setColor(glm::vec4(0.5f, 0.5f, 0.f, 1.f), WidgetVirtual::ALL);
		console->setColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.f), WidgetVirtual::HOVER);
		widgetList.insert(console);
	Layer* layer6 = new Layer(Layer::VISIBLE);
		layer6->setSize(0.05f);
		layer6->setScreenPosition(glm::vec3(0.f, 0.f, 0.f));
		layer6->setPosition(layer6->getScreenPosition());
		layer6->setTargetPosition(layer6->getScreenPosition());
		layer6->add(console);
		layerList.insert(layer6);

	//	push on HUD
	hudList["debug"].push_back(layer1);
	hudList["debug"].push_back(layer2);
	hudList["debug"].push_back(layer3);
	hudList["debug"].push_back(layer4);
	hudList["debug"].push_back(layer5);
	hudList["debug"].push_back(layer6);
	activeHud = "debug";
}
//
