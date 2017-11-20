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
void WidgetManager::loadDebugHud()
{
	//	top title
	WidgetBoard* board1 = new WidgetBoard();
		board1->setPosition(glm::vec3(0.f, 0.01f, 0.f));
		board1->setSize(glm::vec2(2.8f, 0.3f));
		board1->initialize(0.02f, 0.1f);
		board1->setColor(glm::vec4(0.5f, 0.f, 0.2f, 1.f));
		widgetList.insert(board1);
	WidgetLabel* label1 = new WidgetLabel();
		label1->setPosition(glm::vec3(0.f, 0.f, 0.f));
		label1->setSizeChar(0.15f);
		label1->setSize(glm::vec2(2.7f, 0.2f));
		label1->setFont("Data Control");
		label1->initialize("Debug Hud", WidgetLabel::CLIPPING);
		widgetList.insert(label1);
	Layer* layer1 = new Layer();
		layer1->setSize(0.05f);
		layer1->setPosition(glm::vec3(0.f, 0.f, 0.054f));
		layer1->add(board1);
		layer1->add(label1);
		layerList.insert(layer1);

	//	runtime speed
	WidgetBoard* board2 = new WidgetBoard();
		board2->setPosition(glm::vec3(0.f, 0.01f, 0.f));
		board2->setSize(glm::vec2(0.8f, 0.5f));
		board2->initialize(0.02f, 0.15f, WidgetBoard::BOTTOM_RIGHT);
		board2->setColor(glm::vec4(0.f, 0.f, 0.5f, 1.f));
		widgetList.insert(board2);
	WidgetLabel* label2 = new WidgetLabel();
		label2->setPosition(glm::vec3(0.03f, 0.f, 0.f));
		label2->setSizeChar(0.07f);
		label2->setSize(glm::vec2(0.7f, 0.5f));
		label2->setFont("Data Control");
		label2->initialize("FPS : 60\n Avg : 60\n\nTime : 9ms\n Avg : 11ms", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		widgetList.insert(label2);
		addAssociation(label2, "runtime speed");
	Layer* layer2 = new Layer();
		layer2->setSize(0.05f);
		layer2->setPosition(glm::vec3(-0.091f, 0.f, 0.049f));
		layer2->add(board2);
		layer2->add(label2);
		layerList.insert(layer2);

	//	Draw calls
	WidgetBoard* board3 = new WidgetBoard();
		board3->setPosition(glm::vec3(0.f, 0.01f, 0.f));
		board3->setSize(glm::vec2(0.8f, 0.5f));
		board3->initialize(0.02f, 0.15f, WidgetBoard::BOTTOM_LEFT);
		board3->setColor(glm::vec4(0.8f, 0.f, 0.0f, 1.f));
		widgetList.insert(board3);
	WidgetLabel* label3 = new WidgetLabel();
		label3->setPosition(glm::vec3(0.025f, 0.f, 0.f));
		label3->setSizeChar(0.07f);
		label3->setSize(glm::vec2(0.7f, 0.5f));
		label3->setFont("Data Control");
		label3->initialize("Instances :\n80000\n\nTriangles :\n10000000", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		addAssociation(label3, "drawcalls");
		widgetList.insert(label3);
	Layer* layer3 = new Layer();
		layer3->setSize(0.05f);
		layer3->setPosition(glm::vec3(0.091f, 0.f, 0.049f));
		layer3->add(board3);
		layer3->add(label3);
		layerList.insert(layer3);

	//	console
	WidgetBoard* board4 = new WidgetBoard();
		board4->setPosition(glm::vec3(0.f, 0.01f, 0.f));
		board4->setSize(glm::vec2(2.1f, 0.5f));
		board4->initialize(0.02f, 0.1f, WidgetBoard::TOP_RIGHT);
		board4->setColor(glm::vec4(0.5f, 0.5f, 0.f, 1.f));
		widgetList.insert(board4);
	WidgetLabel* label4 = new WidgetLabel();
		label4->setPosition(glm::vec3(0.045f, 0.f, 0.f));
		label4->setSizeChar(0.055f);
		label4->setSize(glm::vec2(2.2f, 0.35f));
		label4->setFont("Data Control");
		label4->initialize("debug consol test 3000\nWARRNING : this game is too awesome\n   this could cause some crash !\npeasant says : hello\npeasant says : that's true this game is awesome\n\ndebug consol test 3000", WidgetLabel::LEFT | WidgetLabel::BOTTOM | WidgetLabel::CLIPPING);
		addAssociation(label4, "console");
		widgetList.insert(label4);
	Layer* layer4 = new Layer();
		layer4->setSize(0.05f);
		layer4->setPosition(glm::vec3(-0.055f, 0.f, -0.049f));
		layer4->add(board4);
		layer4->add(label4);
		layerList.insert(layer4);

	//	picking
	WidgetBoard* board5 = new WidgetBoard();
		board5->setPosition(glm::vec3(0.f, 0.01f, 0.f));
		board5->setSize(glm::vec2(2.1f, 0.5f));
		board5->initialize(0.02f, 0.1f, WidgetBoard::TOP_LEFT);
		board5->setColor(glm::vec4(0.f, 0.5f, 0.f, 1.f));
		widgetList.insert(board5);
	WidgetLabel* label5 = new WidgetLabel();
		label5->setPosition(glm::vec3(0.045f, 0.f, 0.f));
		label5->setSizeChar(0.07f);
		label5->setSize(glm::vec2(2.2f, 0.35f));
		label5->setFont("Data Control");
		label5->initialize("Distance : 50.06 m\nPosition : (923.1 , 584.6 , 1.2)\nInstance on ray : 5\nFirst instance pointed id : 23681\n  type : animated", WidgetLabel::LEFT | WidgetLabel::CLIPPING);
		addAssociation(label5, "interaction");
		widgetList.insert(label5);
	Layer* layer5 = new Layer();
		layer5->setSize(0.05f);
		layer5->setPosition(glm::vec3(0.055f, 0.f, -0.049f));
		layer5->add(board5);
		layer5->add(label5);
		layerList.insert(layer5);

	//	push on HUD
	hudList["debug"].push_back(layer1);
	hudList["debug"].push_back(layer2);
	hudList["debug"].push_back(layer3);
	hudList["debug"].push_back(layer4);
	hudList["debug"].push_back(layer5);
	activeHud = "debug";
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
void WidgetManager::update(const float& elapsedTime)
{
	//	picking

	//	update all layers and widgets
	for (std::set<Layer*>::iterator it = layerList.begin(); it != layerList.end(); ++it)
		(*it)->update(elapsedTime);
	for (std::set<WidgetVirtual*>::iterator it = widgetList.begin(); it != widgetList.end(); ++it)
		(*it)->update(elapsedTime);
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
void WidgetManager::setActiveHUD(const std::string& s)
{
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
