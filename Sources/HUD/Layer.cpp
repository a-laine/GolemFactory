#include "Layer.h"

#define SPEED 0.003f

//  Default
Layer::Layer(const uint8_t& config) : configuration(config), size(1.f) {}
Layer::~Layer() {}
//

//	Public functions
void Layer::update(const float& elapseTime)
{
	glm::vec3 direction = targetPosition - position;
	if (glm::length(direction) > SPEED)
		position += SPEED * glm::normalize(direction);
	else position = targetPosition;

	if (glm::length(position - targetPosition) < SPEED && targetPosition != sreenPosition)
		configuration &= ~VISIBLE;
}
//

//  Set/get functions
void Layer::setPosition(const glm::vec3& p) { position = p; }
void Layer::setSreenPosition(const glm::vec3& p) { sreenPosition = p; }
void Layer::setTargetPosition(const glm::vec3& p) { targetPosition = p; }
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


bool Layer::isVisible() const { return (configuration & VISIBLE) != 0; }
glm::mat4 Layer::getModelMatrix() const
{
	glm::mat4 model = glm::rotate(glm::pi<float>(), glm::vec3(0, 0, 1));			//	begin to rotate 180 degres to have x axis in good direction (left to right)
	model = glm::translate(model, position);										//	go to position
	model = model * glm::eulerAngleYXZ(eulerAngle.y, eulerAngle.x, eulerAngle.z);	//	add orientation
	model = glm::scale(model, glm::vec3(size, size, size));							//	change scale
	return model;
}
glm::vec3 Layer::getSreenPosition() const { return sreenPosition; }
//
