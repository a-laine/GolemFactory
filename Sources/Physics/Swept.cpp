#include "Swept.h"
#include "RigidBody.h"

#include <glm/gtx/component_wise.hpp>

//	Default
Swept::Swept(Entity* e) : entity(e)
{
	init(e);
}
//

//	Set / get functions
const AxisAlignedBox& Swept::getBox() const { return box; }
glm::vec3 Swept::getPosition() const { return entity->getPosition(); };
glm::vec3 Swept::getSize() const { return box.max - box.min; };
//

//	Public functions
void Swept::init(Entity* _entity)
{
	entity = _entity;
	if (!_entity) 
		return;

	RigidBody* rigidbody = entity->getComponent<RigidBody>();
	auto start = entity->getGlobalBoundingShape()->toAxisAlignedBox();
	auto end = entity->getLocalBoundingShape()->toAxisAlignedBox();
	end.transform(rigidbody->predictedPosition, glm::vec3(1.f), rigidbody->predictedOrientation);
	box = AxisAlignedBox(glm::min(start.min, end.min), glm::max(start.max, end.max));
}
//
