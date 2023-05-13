#include "Layer.h"

#define SPEED 0.003f

//  Default
Layer::Layer(const uint8_t& config) : configuration(config), size(1.f) {}
Layer::~Layer() {}
//

//	Public functions
void Layer::update(const float& elapseTime)
{
	vec4f direction = targetPosition - position;
	if (direction.getNorm() > SPEED)
		position += SPEED * direction.getNormal();
	else position = targetPosition;

	if ((position - targetPosition).getNorm() < SPEED && targetPosition != screenPosition)
		configuration &= ~VISIBLE;
}
//

//  Set/get functions
void Layer::setPosition(const vec4f& p) { position = p; }
void Layer::setScreenPosition(const vec4f& p) { screenPosition = p; }
void Layer::setTargetPosition(const vec4f& p) { targetPosition = p; }
void Layer::setSize(const float& s) { size = s; }
void Layer::setOrientation(const float& yaw, const float& pitch, const float& roll)
{
	eulerAngle.x = yaw;
	eulerAngle.y = pitch;
	eulerAngle.z = roll;
}
void Layer::setVisibility(const bool& visible)
{
	if (visible) configuration |= VISIBLE;
	else configuration &= ~VISIBLE;
}
void Layer::setResponsive(const bool& responsive)
{
	if (responsive) configuration |= RESPONSIVE;
	else configuration &= ~RESPONSIVE;
}
void Layer::setConfiguration(const uint8_t& config)
{
	configuration = config;
}



bool Layer::isVisible() const { return (configuration & VISIBLE) != 0; }
bool Layer::isResponsive() const { return (configuration & RESPONSIVE) != 0; }
float Layer::getSize() const { return size; }
mat4f Layer::getModelMatrix() const
{
	mat4f model = mat4f::rotate(mat4f::identity, vec3f(0, 0, PI));			//	begin to rotate 180 degres to have x axis in good direction (left to right)
	model = mat4f::translate(model, position);										//	go to position
	model = mat4f::rotate(model, eulerAngle);	//	add orientation
	model = mat4f::scale(model, vec4f(size, size, size, 1.f));				//	change scale
	return model;
}
vec4f Layer::getPosition() const { return position; }
vec4f Layer::getScreenPosition() const { return screenPosition; }
vec4f Layer::getTargetPosition() const { return targetPosition; }
std::vector<WidgetVirtual*>& Layer::getChildrenList() { return children; }
//

//	Hierarchy modifiers
void Layer::addChild(WidgetVirtual* w) { children.push_back(w); }
bool Layer::removeChild(WidgetVirtual* w)
{
	std::vector<WidgetVirtual*>::iterator it = std::find(children.begin(), children.end(), w);
	if (it != children.end())
	{
		children.erase(it);
		return true;
	}
	else return false;
}
//
