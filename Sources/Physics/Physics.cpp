#include "Physics.h"

#include <set>

#include "RigidBody.h"
#include "Renderer/DrawableComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Scene/SceneManager.h"
#include "World/World.h"

#include "Utiles/Debug.h"

#define APPROXIMATION_FACTOR 10.f


/*
	https://www.sidefx.com/docs/houdini/nodes/dop/rigidbodysolver.html
	https://digitalrune.github.io/DigitalRune-Documentation/html/138fc8fe-c536-40e0-af6b-0fb7e8eb9623.htm#Solutions
*/

//	Debug
void DrawShape(Shape* s)
{
	Debug::color = Debug::white;
	switch (s->type)
	{
		case Shape::SPHERE:
			{
				Sphere* a = reinterpret_cast<Sphere*>(&s);
				Debug::drawSphere(a->center,1.1*a->radius);
				std::cout << "SPHERE " << a->center.x << " " << a->center.y << " " << a->center.z << " " << 1.1*a->radius << std::endl;
			}
			break;
		default:
			std::cout << "UNKNOWN" << std::endl;
			break;
	}
}
//

//  Default
Physics::Physics() : gravity(0.f, 0.f, -9.81f), proximityTest(glm::vec3(0), glm::vec3(0))
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
	clusterFinder.clear();
	collisionList.clear();
	collidingPairs.clear();
	clusters.clear();
	
	predictTransform(elapsedTime);
	computeBoundingShapesAndDetectPairs(elapsedTime, s);
	computeClusters();
	for (unsigned int i = 0; i < clusters.size(); i++)
	{
		std::cout << "cluster " << i << std::endl;
		switch (getSolverType(clusters[i].first))
		{
			case RigidBody::DISCRETE:
				discreteSolver(clusters[i]);
				break;
			case RigidBody::CONTINUOUS:
				continuousSolver(clusters[i]);
				break;
			case RigidBody::SUPERSAMPLING:
				supersamplingSolver(clusters[i]);
				break;
			default:
				break;
		}
	}
	//std::cout << std::endl;

	clearTempoaryStruct(s);
}
/*
void Physics::moveEntity(Entity* entity, const float& elapsedTime, const glm::vec3& delta)
{
	proximityList.clear();
	collisionList.clear();
	if (!entity || !world) return;

	//	broad phase
	detectCollision(entity, elapsedTime, delta);
	Shape prediction = *entity->getGlobalBoundingShape();
	prediction.transform(delta, glm::vec3(1.f), glm::fquat());

	for (unsigned int i = 0; i < proximityList.getObjectInBox().size(); i++)
	{
		Entity* candidate = proximityList.getObjectInBox()[i];
		if (candidate == entity) continue;
		
		//	middle phase
		if (!Collision::collide(*candidate->getGlobalBoundingShape(), prediction))
			continue;

		if (extractIsAnimatable(entity) && extractIsAnimatable(candidate))
		{
			collisionList.push_back(Intersection::intersect(*candidate->getGlobalBoundingShape(), prediction));
			continue;
		}
		if (!extractMesh(candidate)) continue;

		//	narrow phase
		if(extractIsAnimatable(candidate))
			narrowPhase(candidate, entity, *candidate->getGlobalBoundingShape());
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
	glm::vec3 querySize = glm::vec3(entity->getGlobalBoundingShape()->toSphere().radius) + 0.5f * glm::abs(delta);
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

			if (Collision::collide(triangle2, *entity1->getGlobalBoundingShape()))
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
void Physics::predictTransform(const float& elapsedTime)
{
	for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end();)
	{
		/*Sphere* a = reinterpret_cast<Sphere*>((Sphere*)(*it)->getGlobalBoundingShape());
		std::cout << "predictTransform " << a->center.x << " " << a->center.y << " " << a->center.z << " " << 1.1*a->radius << std::endl;*/

		RigidBody* rigidbody = (*it)->getComponent<RigidBody>();
		if (!rigidbody)
		{
			if ((*it)->swept)
			{
				delete (*it)->swept;
				(*it)->swept = nullptr;
			}
			it = movingEntity.erase(it);
		}
		else if (rigidbody->getMass() == 0.f)  
		{
			if ((*it)->swept)
			{
				delete (*it)->swept;
				(*it)->swept = nullptr;
			}
			it = movingEntity.erase(it);
		}
		else
		{
			//	linear acceleration
			rigidbody->acceleration = rigidbody->mass * gravity * rigidbody->gravityFactor;
			for (unsigned int i = 0; i < rigidbody->forces.size(); i++)
				rigidbody->acceleration += rigidbody->forces[i];
			rigidbody->acceleration *= rigidbody->inverseMass;

			//	linear velocity
			rigidbody->velocity += elapsedTime * rigidbody->acceleration;

			//	angular acceleration
			rigidbody->angularAcceleration = glm::vec3(0.f);
			for (unsigned int i = 0; i < rigidbody->torques.size(); i++)
				rigidbody->angularAcceleration += rigidbody->torques[i];
			rigidbody->angularAcceleration = rigidbody->inverseInertia * rigidbody->angularAcceleration;

			//	angular velocity
			rigidbody->angularVelocity += elapsedTime * rigidbody->angularAcceleration;

			//	predict position
			rigidbody->deltaPosition = rigidbody->velocity * elapsedTime;
			rigidbody->predictPosition = (*it)->getPosition() + rigidbody->deltaPosition;

			//	predict orientation
			rigidbody->deltaRotation = 0.5f * elapsedTime * glm::fquat(0.f, rigidbody->angularVelocity.x, rigidbody->angularVelocity.y, rigidbody->angularVelocity.z) * (*it)->getOrientation();
			rigidbody->predictRotation = (*it)->getOrientation() +rigidbody->deltaRotation;
			
			//	clear
			rigidbody->forces.clear();
			rigidbody->torques.clear();

			//	check new static
			if(rigidbody->isResting())
			{
				if ((*it)->swept)
				{
					delete (*it)->swept;
					(*it)->swept = nullptr;
				}
				it = movingEntity.erase(it);
			}
			else ++it;
		}
	}
}
void Physics::computeBoundingShapesAndDetectPairs(const float& elapsedTime, SceneManager* scene)
{
	for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end(); ++it)
	{
		//DrawShape((Shape*)(*it)->getLocalBoundingShape());

		Swept* swept = (*it)->swept;
		if (!swept)
		{
			swept = new Swept(*it);
			(*it)->swept = swept;
		}
		else swept->init(*it);
		sweptList.push_back(swept);
		
		/*scene->removeObject(*it);
		scene->addObject(*it);*/
	
		auto box = swept->getBox();

		Debug::color = Debug::black; 
		Debug::drawWiredCube(glm::translate(0.5f * (box.min + box.max)), 0.5f * (box.max - box.min));
		Debug::color = Debug::magenta;
		Debug::drawLine((*it)->getPosition(), (*it)->getPosition() + (*it)->getComponent<RigidBody>()->velocity);

		proximityTest.result.clear();
		proximityTest.bbMin = box.min;
		proximityTest.bbMax = box.max;
		proximityList.result.clear();
		scene->getEntities(&proximityTest, &proximityList);

		bool collision = false;
		for (unsigned int i = 0; i < proximityList.result.size(); i++)
		{
			//	get shape of concurent entity
			Shape* shape2 = nullptr;
			if (proximityList.result[i]->swept)
			{
				if (proximityList.result[i]->swept == swept)
					continue;
				shape2 = (Shape*)&proximityList.result[i]->swept->getBox();
			}
			else shape2 = const_cast<Shape*>(proximityList.result[i]->getGlobalBoundingShape());
			
			//	test collision
			if (shape2 && Collision::collide(swept->getBox(), *shape2))
			{
				collision = true;
				collidingPairs.insert(std::pair<Entity*, Entity*>(*it, proximityList.result[i]));
			}
		}

		if (!collision)
		{
			integratePosition(*it, elapsedTime);
			scene->updateObject(*it);
		}

		/*Sphere* a = reinterpret_cast<Sphere*>((Sphere*)(*it)->getGlobalBoundingShape());
		std::cout << "BB and pairs " << a->center.x << " " << a->center.y << " " << a->center.z << " " << 1.1*a->radius << std::endl;*/
	}
}
void Physics::computeClusters()
{
	std::set<Entity*> nodes;
	for (auto it = collidingPairs.begin(); it != collidingPairs.end(); ++it)
	{
		nodes.insert(it->first);
		if (it->second->getComponent<RigidBody>())
			nodes.insert(it->second);
	}
	clusterFinder.initialize(nodes);
	for (auto it = collidingPairs.begin(); it != collidingPairs.end(); ++it)
	{
		clusterFinder.addLink(it->first, it->second);
		if(it->second->getComponent<RigidBody>())
			clusterFinder.addLink(it->second, it->first);
	}
	auto c = clusterFinder.getCluster();
	for (unsigned int i = 0; i < c.size(); i++)
	{
		std::vector<Entity*> dynamicEntities;
		std::vector<Entity*> staticEntities;
		for (unsigned int j = 0; j < c[i].size(); j++)
		{
			if (c[i][j]->swept) dynamicEntities.emplace_back(c[i][j]);
			else staticEntities.emplace_back(c[i][j]);
		}
		if (dynamicEntities.empty() || dynamicEntities.size() + staticEntities.size() == 1)
			continue;

		clusters.emplace_back(std::make_pair(dynamicEntities, staticEntities));
	}
}

void Physics::integratePosition(Entity* entity, const float& elapsedTime)
{
	RigidBody* rigidbody = entity->getComponent<RigidBody>();
	entity->setTransformation(rigidbody->predictPosition, entity->getScale(), glm::normalize(rigidbody->predictRotation));
}
void Physics::clearTempoaryStruct(SceneManager* scene)
{
	for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end(); ++it)
	{
		scene->removeObject(*it);
		scene->addObject(*it);
	}
	sweptList.clear();
}
//

//  Solveurs
void Physics::discreteSolver(const std::pair<std::vector<Entity*>, std::vector<Entity*> >& cluster)
{
	for (unsigned int i = 0; i < cluster.first.size(); i++)
	{
		/*Sphere* a = reinterpret_cast<Sphere*>((Sphere*)cluster.first[i]->getGlobalBoundingShape());
		std::cout << "discreteSolver " << a->center.x << " " << a->center.y << " " << a->center.z << " " << 1.1*a->radius << std::endl;*/
		//DrawShape((Shape*)cluster.first[i]->getLocalBoundingShape());
		//std::cout << "   " << (unsigned long long)cluster.first[i] << std::endl;
		//RigidBody* rigidbody = cluster.first[i]->getComponent<RigidBody>();
		/*Shape* end = cluster.first[i]->getGlobalBoundingShape()->duplicate();
		end->transform(rigidbody->getDeltaPosition(), glm::vec3(1.f), rigidbody->getDeltaRotation());*/
		//DrawShape((Shape*)cluster.first[i]->getGlobalBoundingShape());

		//Debug::color = Debug::magenta;
		//Debug::drawLine(cluster.first[i]->getPosition(), cluster.first[i]->getPosition() + rigidbody->velocity);

		/*for (unsigned int j = i + 1; j < cluster.first.size(); j++)
		{
			if (Collision::collide(*end, *cluster.first[j]->getGlobalBoundingShape()))
			{
				Intersection::Contact contact = Intersection::intersect(*end, *cluster.first[j]->getGlobalBoundingShape());
				Debug::color = Debug::magenta;
				Debug::drawLine(contact.contactPointA, contact.contactPointB);
			}
		}*/
		/*for (unsigned int j = i + 1; j < cluster.first.size(); j++)
			//Debug::drawLine(cluster.first[i]->getPosition(), cluster.first[j]->getPosition());
			DrawShape((Shape*)cluster.first[i]->getGlobalBoundingShape());*/
		//cluster.first[i]->setTransformation(rigidbody->predictPosition, cluster.first[i]->getScale(), glm::normalize(rigidbody->predictRotation));
		//scene->updateObject(*it);
	}
}
void Physics::continuousSolver(const std::pair<std::vector<Entity*>, std::vector<Entity*> >& cluster)
{

}
void Physics::supersamplingSolver(const std::pair<std::vector<Entity*>, std::vector<Entity*> >& cluster)
{

}
//


//	Usefull functions
RigidBody::SolverType Physics::getSolverType(const std::vector<Entity*>& cluster)
{
	RigidBody::SolverType solver = RigidBody::DISCRETE;
	for (unsigned int i = 0; i < cluster.size(); i++)
	{
		RigidBody* rigidbody = cluster[i]->getComponent<RigidBody>();
		if (solver < rigidbody->solver)
			solver = rigidbody->solver;
	}
	return solver;
}
/*Mesh* Physics::extractMesh(Entity* entity) const
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
}*/
//



//	Private internal class
void Physics::EntityGraph::clear()
{
	nodes.clear();
	graph.clear();
}
void Physics::EntityGraph::initialize(const std::set<Entity*>& n)
{
	nodes = n;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
		graph[*it] = std::pair<std::set<Entity*>, bool>(std::set<Entity*>(), false);
}
void Physics::EntityGraph::addLink(const Entity* n1, const Entity* n2)
{
	if (n1 != n2)
		graph[const_cast<Entity*>(n1)].first.insert(const_cast<Entity*>(n2));
}
std::vector<std::vector<Entity*> > Physics::EntityGraph::getCluster()
{
	std::vector<std::vector<Entity*> > result;

	for (auto it = graph.begin(); it != graph.end(); ++it)
	{
		if (!it->second.second)
		{
			std::vector<Entity*> cluster;
			getNeighbours(it->first, cluster);
			result.push_back(cluster);
		}
	}
	return result;
}

void Physics::EntityGraph::getNeighbours(Entity* node, std::vector<Entity*>& result)
{
	auto itnode = graph.find(node);
	if (itnode != graph.end() && !itnode->second.second)
	{
		graph[node].second = true;
		result.push_back(node);
		for (auto it = graph[node].first.begin(); it != graph[node].first.end(); ++it)
			getNeighbours(*it, result);
	}
	else if(itnode == graph.end())
		result.push_back(node);
}
//