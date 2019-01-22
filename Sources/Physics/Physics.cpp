#include "Physics.h"

#include <set>

#include "RigidBody.h"
#include "Renderer/DrawableComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Scene/SceneManager.h"
#include "World/World.h"


//  Default
Physics::Physics() : gravity(0.f, 0.f, -9.81f)
{}
Physics::~Physics()
{}
//

//	Set / get functions
void Physics::setGravity(const glm::vec3& g) { gravity = g; }


glm::vec3 Physics::getGravity() const { return gravity; }


void Physics::addMovingEntity(Entity* e)
{
	if (e->getComponent<RigidBody>() && movingEntity.insert(e).second)
		e->getParentWorld()->getOwnership(e);
}
//


//	Public functions
void Physics::stepSimulation(const float& elapsedTime, SceneManager* s)
{
	proximityList.clear();
	collisionList.clear();
	collidingPairs.clear();

	//std::cout << "moving object count : " << movingEntity.size() << std::endl;

	applyForces(elapsedTime);
	predictTransform(elapsedTime);
	computeBoundingShapes(elapsedTime);
	detectPairs(elapsedTime);
	computeContacts(elapsedTime);
	solveConstraints(elapsedTime);
	integratePositions(elapsedTime);

	//std::cout << std::endl;
}
/*
void Physics::moveEntity(Entity* entity, const float& elapsedTime, const glm::vec3& delta)
{
	proximityList.clear();
	collisionList.clear();
	if (!entity || !world) return;

	//	broad phase
	detectCollision(entity, elapsedTime, delta);
	Shape prediction = entity->getShape();
	prediction.transform(delta, glm::vec3(1.f), glm::fquat());

	for (unsigned int i = 0; i < proximityList.getObjectInBox().size(); i++)
	{
		Entity* candidate = proximityList.getObjectInBox()[i];
		if (candidate == entity) continue;
		
		//	middle phase
		if (!Collision::collide(candidate->getShape(), prediction))
			continue;

		if (extractIsAnimatable(entity) && extractIsAnimatable(candidate))
		{
			collisionList.push_back(Intersection::intersect(candidate->getShape(), prediction));
			continue;
		}
		if (!extractMesh(candidate)) continue;

		//	narrow phase
		if(extractIsAnimatable(candidate))
			narrowPhase(candidate, entity, candidate->getShape());
		else narrowPhase(entity, candidate, prediction);
	}

	//	test if grounded
	bool grounded = false;
	for (unsigned int i = 0; i < collisionList.size(); i++)
	{
		if (collisionList[i].normal.z > 0.5f)
			grounded = true;
	}
	std::cout << (grounded ? "grounded " : ". ") << std::endl;
}

//


//	Protected functions
void Physics::detectCollision(Entity* entity, const float& elapsedTime, const glm::vec3& delta)
{
	glm::vec3 querySize = glm::vec3(entity->getShape().toSphere().radius) + 0.5f * glm::abs(delta);
	glm::vec3 queryCenter = entity->getPosition() + 0.5f * delta;
	DefaultSceneManagerBoxTest queryBox(queryCenter - querySize, queryCenter + querySize);
	world->getSceneManager().getObjects(proximityList, queryBox);
}
void Physics::narrowPhase(Entity* entity1, Entity* entity2, Shape prediction1)
{
	//	prepare aliases
	bool animatable = extractIsAnimatable(entity1);

	Mesh* mesh1 = extractMesh(entity1);
	const std::vector<glm::vec3>& vertices1 = *mesh1->getVertices();
	const std::vector<unsigned short>& faces1 = *mesh1->getFaces();
	glm::mat4 model1 = entity1->getMatrix();
	

	Mesh* mesh2 = extractMesh(entity2);
	const std::vector<glm::vec3>& vertices2 = *mesh2->getVertices();
	const std::vector<unsigned short>& faces2 = *mesh2->getFaces();
	glm::mat4 model2 = entity2->getMatrix();

	//	narrow phase in action, and store result in list
	if (animatable)
	{
		for (unsigned int j = 0; j < faces2.size(); j += 3)
		{
			glm::vec3 p12 = glm::vec3(model2 * glm::vec4(vertices2[faces2[j]], 1.f));
			glm::vec3 p22 = glm::vec3(model2 * glm::vec4(vertices2[faces2[j + 1]], 1.f));
			glm::vec3 p32 = glm::vec3(model2 * glm::vec4(vertices2[faces2[j + 2]], 1.f));
			Triangle triangle2 = Triangle(p12, p22, p32);

			if (Collision::collide(triangle2, entity1->getShape()))
				collisionList.push_back(Intersection::intersect(triangle2, prediction1));
		}
	}
	else
	{
		for (unsigned int i = 0; i < faces1.size(); i += 3)
		{
			glm::vec3 p11 = glm::vec3(model1 * glm::vec4(vertices1[faces1[i]], 1.f));
			glm::vec3 p21 = glm::vec3(model1 * glm::vec4(vertices1[faces1[i + 1]], 1.f));
			glm::vec3 p31 = glm::vec3(model1 * glm::vec4(vertices1[faces1[i + 2]], 1.f));
			Triangle triangle1 = Triangle(p11, p21, p31);

			for (unsigned int j = 0; j < faces2.size(); j += 3)
			{
				glm::vec3 p12 = glm::vec3(model2 * glm::vec4(vertices2[faces2[j]], 1.f));
				glm::vec3 p22 = glm::vec3(model2 * glm::vec4(vertices2[faces2[j + 1]], 1.f));
				glm::vec3 p32 = glm::vec3(model2 * glm::vec4(vertices2[faces2[j + 2]], 1.f));
				Triangle triangle2 = Triangle(p12, p22, p32);

				if (Collision::collide(triangle1, triangle2))
					collisionList.push_back(Intersection::intersect(triangle1, triangle2));
			}
		}
	}
}
*/
//


//	Pipeline
void Physics::applyForces(const float& elapsedTime)
{
	for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end(); it++)
	{
		RigidBody* rigidbody = (*it)->getComponent<RigidBody>();
		if (!rigidbody) it = std::prev(movingEntity.erase(it));
		else if (rigidbody->getMass() == 0.f) continue;
		else
		{
			//	compute accelerations
			rigidbody->acceleration = rigidbody->getMass() * gravity * rigidbody->getGravityFactor();
			for (unsigned int i = 0; i < rigidbody->forces.size(); i++)
				rigidbody->acceleration += rigidbody->forces[i];
			rigidbody->acceleration /= rigidbody->getMass();

			rigidbody->angularAcceleration = glm::vec3(0.f);
			for (unsigned int i = 0; i < rigidbody->torques.size(); i++)
				rigidbody->angularAcceleration += rigidbody->torques[i];
			rigidbody->angularAcceleration = glm::inverse(rigidbody->inertia) * rigidbody->angularAcceleration;

			//	integrate
			rigidbody->velocity += elapsedTime * rigidbody->acceleration;
			rigidbody->angularVelocity += elapsedTime * rigidbody->angularAcceleration;

			//	clear
			rigidbody->forces.clear();
			rigidbody->torques.clear();
		}
	}
}
void Physics::predictTransform(const float& elapsedTime)
{
	// save
	// rigidbody->rotation from axis&angle (vector & magnitude) to quaternion
	// set entity position & rotation
}
void Physics::computeBoundingShapes(const float& elapsedTime)
{

}
void Physics::detectPairs(const float& elapsedTime)
{
	
}
void Physics::computeContacts(const float& elapsedTime)
{

}
void Physics::solveConstraints(const float& elapsedTime)
{

}
void Physics::integratePositions(const float& elapsedTime)
{

}
//



//	Usefull functions
Mesh* Physics::extractMesh(Entity* entity) const
{
	DrawableComponent* drawableComponent = entity->getComponent<DrawableComponent>();
	if (!drawableComponent || !drawableComponent->isValid())
		return nullptr;
	return drawableComponent->getMesh();
}
bool Physics::extractIsAnimatable(Entity* entity) const
{
	DrawableComponent* drawableComponent = entity->getComponent<DrawableComponent>();
	if (!drawableComponent || !drawableComponent->isValid())
		return false;

	SkeletonComponent* skeletonComponent = entity->getComponent<SkeletonComponent>();
	if (!skeletonComponent || !skeletonComponent->isValid())
		return false;

	return true;
}
//
