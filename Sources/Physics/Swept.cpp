#include "Swept.h"
#include "RigidBody.h"

#include <glm/gtx/component_wise.hpp>

//	Default
Swept::Swept(Entity* e) : entity(e), dp(0.f), dq()
{
	RigidBody* rigidbody = entity->getComponent<RigidBody>();
	dp = rigidbody->getDeltaPosition();
	dq = rigidbody->getDeltaRotation();

	auto start = entity->getGlobalBoundingShape()->toAxisAlignedBox();
	auto end = *static_cast<AxisAlignedBox*>(start.duplicate());
	end.transform(dp, glm::vec3(1.f), dq);

	glm::vec3 min = glm::vec3(glm::min(start.min.x, end.min.x), glm::min(start.min.y, end.min.y), glm::min(start.min.z, end.min.z));
	glm::vec3 max = glm::vec3(glm::max(start.max.x, end.max.x), glm::max(start.max.y, end.max.y), glm::max(start.max.z, end.max.z));
	box = AxisAlignedBox(min, max);
}
//

//	Set / get functions
const AxisAlignedBox& Swept::getBox() const { return box; }
//

//	Default
PhysicsArtefacts::PhysicsArtefacts(Entity* e) : type(PhysicsArtefacts::ENTITY)
{
	data.entity = e;
}
PhysicsArtefacts::PhysicsArtefacts(Swept* s) : type(PhysicsArtefacts::SWEPT)
{
	data.swept = s;
}
//