#include "WidgetConsole.h"

//	same as widget label
#define LINE_OFFSET		0.5f
#define TEX_OFFSET		0.00586f

//  Default
WidgetConsole::WidgetConsole(const uint8_t& config, const std::string& shaderName) : 
	WidgetBoard(config | NEED_UPDATE, shaderName)
{
	type = CONSOLE;
	font = nullptr;
}
WidgetConsole::~WidgetConsole()
{
	ResourceManager::getInstance()->release(font);
}
//



//	Public functions
void WidgetConsole::update(const float& elapseTime)
{
	State s = (State)(configuration & STATE_MASK);
	colors[CURRENT] = 0.9f * colors[CURRENT] + 0.1f * colors[s];
	positions[CURRENT] = positions[s];
	sizes[CURRENT] = sizes[s];
	lastConfiguration = configuration;

	//	update buffers if needed
	if (configuration & RESPONSIVE)
	{
		updateCooldown += elapseTime;
		if (configuration & NEED_UPDATE && updateCooldown > 500.f)
		{
			updateBuffers();
			updateVBOs();
			configuration &= ~NEED_UPDATE;
			updateCooldown = 0.f;
		}
	}
}
void WidgetConsole::initialize(const float& bThickness, const float& bWidth, const uint8_t& corner)
{
	colors[CURRENT] = colors[(State)(configuration & STATE_MASK)];
	positions[CURRENT] = positions[(State)(configuration & STATE_MASK)];
	sizes[CURRENT] = sizes[(State)(configuration & STATE_MASK)];
	lastConfiguration = configuration;

	borderThickness = bThickness;
	borderWidth = bWidth;
	cornerConfiguration = corner;

	batchList.push_back(DrawBatch());
	batchList.push_back(DrawBatch());
	updateBuffers(true);

	if (configuration & RESPONSIVE) initializeVBOs(GL_DYNAMIC_DRAW);
	else initializeVBOs(GL_STATIC_DRAW);
	initializeVAOs();
}
void WidgetConsole::draw(Shader* s, uint8_t& stencilMask)
{
	WidgetBoard::draw(s, stencilMask);
}
bool WidgetConsole::intersect(const glm::mat4& base, const glm::vec3& ray, const glm::vec3 origin, glm::vec3& result)
{
	return WidgetVirtual::intersect(base, ray, origin, result);
}
//



//	Set / get functions
void WidgetConsole::setFont(const std::string& fontName)
{
	ResourceManager::getInstance()->release(font);
	font = ResourceManager::getInstance()->getFont(fontName);
	configuration |= NEED_UPDATE;
}
void WidgetConsole::setSizeChar(const float& f)
{
	sizeChar = f;
	configuration |= NEED_UPDATE;
}


std::stringstream* WidgetConsole::getStream() { return &text; }
Font* WidgetConsole::getFont() const { return font; }
float WidgetConsole::getSizeChar() const { return sizeChar; }
//

//	Protected functions
void WidgetConsole::updateBuffers(const bool& firstInit)
{
	WidgetBoard::updateBuffers(firstInit);
}
//