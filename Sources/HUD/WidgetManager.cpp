#include "WidgetManager.h"


//  Default
WidgetManager::WidgetManager() {}
WidgetManager::~WidgetManager()
{
	hudList.clear();
	for (std::set<WidgetVirtual*>::iterator it = widgetList.begin(); it != widgetList.end(); ++it)
		delete *it;
}
//

//	Public functions
void WidgetManager::loadHud(const std::string& fileName)
{
	//	dummy
	WidgetBoard* board = new WidgetBoard();
		board->setPosition(glm::vec3(0.f, 0.1f, 0.f));
		board->setSize(glm::vec2(2.f, 1.f));
		board->initialize(0.02f, 0.1f, WidgetBoard::TOP_LEFT | WidgetBoard::TOP_RIGHT | WidgetBoard::BOTTOM_RIGHT | WidgetBoard::BOTTOM_LEFT);
		board->setColor(glm::vec4(0.5f, 0.f, 0.2f, 1.f));
		widgetList.insert(board);
	WidgetLabel* label = new WidgetLabel();
		label->setSize(glm::vec2(1.85f, 0.8f));
		label->setFont("Data Control");
		label->initialize("Hi friend !", WidgetLabel::CLIPPING);
		widgetList.insert(label);
	Layer* layer = new Layer();
		layer->setSize(0.05f);
		layer->setPosition(glm::vec3(0.f, 0.f, 0.f));
		layer->add(board);
		layer->add(label);
		layerList.insert(layer);
	hudList["default"].push_back(layer);
	activeHud = "default";
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


void WidgetManager::addWidget(WidgetVirtual* w) { widgetList.insert(w); }
void WidgetManager::removeWidget(WidgetVirtual* w) { widgetList.erase(w); }
void WidgetManager::addLayer(Layer* l) { layerList.insert(l); }
void WidgetManager::removeLayer(Layer* l) { layerList.erase(l); }
//

//	Set / get functions
void WidgetManager::setActiveHUD(const std::string& s)
{
	activeHud = s;
}

std::string WidgetManager::getActiveHUD() const { return activeHud; }
//
