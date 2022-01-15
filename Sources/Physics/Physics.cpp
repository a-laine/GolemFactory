#include "Physics.h"

#include <set>

#include "RigidBody.h"
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Scene/SceneManager.h>
#include <World/World.h>
#include <Utiles/Debug.h>

#include <Utiles/Debug.h>
#include <Utiles/ImguiConfig.h>

#define APPROXIMATION_FACTOR 10.f
#define EPSILON 0.000001f
#define SUPERSAMPLING_DELTA 0.01f

#define SOLVER_MAX_ITERATIONS 5000
#define SOLVER_ITERATION_NORMAL_GAIN 0.8f
#define SOLVER_ITERATION_TANGENT_GAIN 0.5f
#define SOLVER_ITERATION_THRESHOLD 10E-06f

/*
	https://www.sidefx.com/docs/houdini/nodes/dop/rigidbodysolver.html
	https://digitalrune.github.io/DigitalRune-Documentation/html/138fc8fe-c536-40e0-af6b-0fb7e8eb9623.htm#Solutions
*/

#ifdef USE_IMGUI
	bool PhysicDebugWindowEnable = false;
#endif // USE_IMGUI



bool Physics::drawSweptBoxes = true;
bool Physics::drawCollisions = true;
bool Physics::drawClustersAABB = true;


//  Default
Physics::Physics() : gravity(0.f, 0.f, -9.81f), proximityTest(glm::vec3(0), glm::vec3(0)), defaultFriction(0.7f)
{}
Physics::~Physics()
{}
//

//	Set / get functions
void Physics::setGravity(const glm::vec3& g) { gravity = g; }
void Physics::setDefaultFriction(const float& f) { defaultFriction = f; };

glm::vec3 Physics::getGravity() const { return gravity; }
float Physics::getDefaultFriction() const { return defaultFriction; }

void Physics::addMovingEntity(Entity* e)
{
	if (e->getComponent<RigidBody>() && movingEntity.insert(e).second)
		e->getParentWorld()->getOwnership(e);
}
//


//	Public functions
void Physics::stepSimulation(const float& elapsedTime, SceneManager* scene)
{
	if (elapsedTime == 0.f)
		return;

	clusterFinder.clear();
	dynamicPairs.clear();
	dynamicCollisions.clear();
	staticCollisions.clear();
	clusters.clear();
	
	predictTransform(elapsedTime);
	computeBoundingShapesAndDetectPairs(elapsedTime, scene);
	computeClusters();
	for (unsigned int i = 0; i < clusters.size(); i++)
	{
		createConstraint(i, elapsedTime);
		solveConstraint(i, elapsedTime);

		for (unsigned int j = 0; j < clusters[i].dynamicEntities.size(); j++)
		{
			RigidBody* rigidbody = clusters[i].bodies[j];

			rigidbody->setPosition(rigidbody->getPosition() + elapsedTime * rigidbody->linearVelocity);
			glm::fquat dq = glm::fquat(0.f, rigidbody->angularVelocity.x, rigidbody->angularVelocity.y, rigidbody->angularVelocity.z);
			glm::fquat q = rigidbody->getOrientation() + 0.5f * elapsedTime * dq * rigidbody->getOrientation();
			rigidbody->setOrientation(glm::normalize(q));
			rigidbody->setExternalForces(glm::vec3(0.f));
			rigidbody->setExternalTorques(glm::vec3(0.f));

			rigidbody->linearVelocity *= 0.99f;
			rigidbody->angularVelocity *= 0.99f;

			scene->updateObject(clusters[i].dynamicEntities[j]);
		}
	}

	clearTempoaryStruct(scene);
}

void Physics::debugDraw()
{
	const glm::vec3 clustersOffset = glm::vec3(0.004f);
	const glm::vec3 sweptOffset = glm::vec3(0.004f);
	const float pointRadius = 0.01f;
	const float depthLength = 10.f;
	const float tangentLength = 0.3f;

	for (unsigned int i = 0; i < clusters.size(); i++)
	{
		const Cluster& cluster = clusters[i];

		if (drawClustersAABB)
		{
			Debug::getInstance()->color = Debug::magenta;
			for (const Cluster& cluster : clusters)
			{
				AxisAlignedBox box = cluster.dynamicEntities[0]->swept->getBox();
				for (int j = 1; j < cluster.dynamicEntities.size(); j++)
					box.add(cluster.dynamicEntities[j]->swept->getBox());
				Debug::getInstance()->drawLineCube(glm::mat4(1.f), box.min - clustersOffset, box.max + clustersOffset);
			}
		}

		if (drawSweptBoxes)
		{
			for (const Entity* entity : cluster.dynamicEntities)
			{
				Debug::getInstance()->color = Debug::yellow;
				Debug::getInstance()->drawLineCube(glm::mat4(1.f), entity->swept->getBox().min - sweptOffset, entity->swept->getBox().max + sweptOffset);
			}
		}

		if (drawCollisions)
		{
			for (const Constraint& constraint : cluster.constraints)
			{
				// point and collision frame
				Debug::color = Debug::red;
				Debug::drawSphere(constraint.worldPoint, pointRadius);
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint - (constraint.depth * depthLength) * constraint.axis[0]);
				Debug::color = Debug::green;
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint + tangentLength * constraint.axis[1]);
				Debug::color = Debug::blue;
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint + tangentLength * constraint.axis[2]);

				// closing velocity
				Debug::color = Debug::orange;
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint + constraint.computeClosingVelocity());

				// computed impulse
				Debug::color = Debug::yellow;
				glm::vec3 impulse = glm::vec3(0.f);
				for (int k = 0; k < constraint.axisCount; k++)
					impulse += constraint.accumulationLinear[k] * constraint.axis[k];
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint + impulse);
			}
		}
	}
}
void Physics::drawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("Physics");

	ImGui::Text("Options");
	ImGui::Checkbox("Draw sweept boxes", &drawSweptBoxes);
	ImGui::Checkbox("Draw collisions", &drawCollisions);
	ImGui::Checkbox("Draw clusters", &drawClustersAABB);

	ImGui::End();
#endif // USE_IMGUI
}
//


//	Pipeline
void Physics::predictTransform(const float& elapsedTime)
{
	for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end();)
	{
		Entity* entity = *it;
		RigidBody* rigidbody = entity->getComponent<RigidBody>();
		if (!rigidbody || rigidbody->getMass() == 0.f)
		{
			if (entity->swept)
			{
				delete entity->swept;
				entity->swept = nullptr;
			}
			entity->getParentWorld()->releaseOwnership(entity);
			it = movingEntity.erase(it);
		}
		else
		{
			rigidbody->beforeStepPosition = rigidbody->getPosition();
			rigidbody->beforeStepOrientation = rigidbody->getOrientation();

			rigidbody->linearAcceleration = gravity * rigidbody->gravityFactor;							// gravity
			//rigidbody->linearAcceleration -= rigidbody->drag * rigidbody->linearVelocity;				// air friction
			rigidbody->linearAcceleration += rigidbody->inverseMass * rigidbody->externalForces;		// other forces
			rigidbody->angularAcceleration = rigidbody->inverseInertia * rigidbody->externalTorques;	// other torques

			rigidbody->linearVelocity += elapsedTime * rigidbody->linearAcceleration;
			rigidbody->angularVelocity += elapsedTime * rigidbody->angularAcceleration;

			rigidbody->predictedPosition = rigidbody->getPosition() + elapsedTime * rigidbody->linearVelocity;
			glm::fquat dq = glm::fquat(0.f, rigidbody->angularVelocity.x, rigidbody->angularVelocity.y, rigidbody->angularVelocity.z);
			rigidbody->predictedOrientation = rigidbody->getOrientation() + 0.5f * elapsedTime * dq * rigidbody->getOrientation();
			rigidbody->predictedOrientation = glm::normalize(rigidbody->predictedOrientation);

			Swept* swept = entity->swept;
			if (!swept)
			{
				swept = new Swept(entity);
				entity->swept = swept;
			}
			else
				swept->init(*it);

			++it;
		}
	}
}
void Physics::computeBoundingShapesAndDetectPairs(const float& elapsedTime, SceneManager* scene)
{
	for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end(); ++it)
	{
		Entity* entity = *it;
		RigidBody* rigidbody = entity->getComponent<RigidBody>();
		Swept* swept = entity->swept;
		if (!swept)
			continue;

		sweptList.push_back(swept);
		const AxisAlignedBox& box = swept->getBox();

		proximityTest.result.clear();
		proximityTest.bbMin = box.min;
		proximityTest.bbMax = box.max;
		proximityList.result.clear();
		scene->getEntities(&proximityTest, &proximityList);

		bool collideOnDynamic = false;
		bool collideOnStatic = false;
		for (unsigned int i = 0; i < proximityList.result.size(); i++)
		{
			if (proximityList.result[i] == entity)
				continue;

			//	get shape of concurent entity
			Entity* other = proximityList.result[i];
			Shape* shape2 = nullptr;
			if (other->swept)
				shape2 = (Shape*)&other->swept->getBox();
			else
				shape2 = const_cast<Shape*>(other->getGlobalBoundingShape());
			
			//	test collision
			if (shape2 && Collision::collide(&swept->getBox(), shape2))
			{
				if (other->swept)
				{
					collideOnDynamic = true;
					dynamicPairs.insert(std::pair<Entity*, Entity*>(entity, other));
					dynamicCollisions[entity].push_back(other);
				}
				else
				{
					collideOnStatic = true;
					staticCollisions[entity].push_back(proximityList.result[i]);
				}
			}
		}

		if (!collideOnDynamic && !collideOnStatic)
		{
			// no collision detected, just move object to predicted pose
			entity->setTransformation(rigidbody->predictedPosition, entity->getScale(), rigidbody->predictedOrientation);
			scene->updateObject(entity);
		}
		else if (!collideOnDynamic && collideOnStatic)
		{
			// small cluster of one dynamic object
			clusters.push_back(Cluster());
			Cluster& cluster = clusters.back();
			cluster.dynamicEntities.push_back(entity);
			cluster.bodies.push_back(rigidbody);
			std::map<Entity*, std::vector<Entity*>>::iterator it = staticCollisions.find(entity);
			if (it != staticCollisions.end())
			{
				for (unsigned int k = 0; k < it->second.size(); k++)
					cluster.staticEntities.push_back(it->second[k]);
			}
		}
	}
}
void Physics::computeClusters()
{
	std::set<Entity*> nodes;
	for (auto it = dynamicPairs.begin(); it != dynamicPairs.end(); ++it)
		nodes.insert(it->first);

	clusterFinder.initialize(nodes);
	for (auto it = dynamicPairs.begin(); it != dynamicPairs.end(); ++it)
	{
		clusterFinder.addLink(it->first, it->second);
		if(it->second->getComponent<RigidBody>())
			clusterFinder.addLink(it->second, it->first);
	}

	std::vector<std::vector<Entity*>> clusterList = clusterFinder.getCluster();
	for (unsigned int i = 0; i < clusterList.size(); i++)
	{
		clusters.push_back(Cluster());
		Cluster& cluster = clusters.back();
		std::set<Entity*> staticEntities;

		for (unsigned int j = 0; j < clusterList[i].size(); j++)
		{
			Entity* entity = clusterList[i][j];
			cluster.dynamicEntities.emplace_back(entity);
			cluster.bodies.push_back(entity->getComponent<RigidBody>());

			std::map<Entity*, std::vector<Entity*>>::iterator it = staticCollisions.find(entity);
			if (it != staticCollisions.end())
			{
				for (unsigned int k = 0; k < it->second.size(); k++)
					staticEntities.insert(it->second[k]);
			}
		}
		if (cluster.dynamicEntities.empty())
			clusters.pop_back();
		else
		{
			for (Entity* entity : staticEntities)
				cluster.staticEntities.push_back(entity);
		}
	}
}
void Physics::createConstraint(const unsigned int& clusterIndex, const float& deltaTime)
{
	Cluster* cluster = &clusters[clusterIndex];

	for (unsigned int i = 0; i < cluster->bodies.size(); i++)
		cluster->dynamicEntities[i]->setTransformation(cluster->bodies[i]->predictedPosition, cluster->dynamicEntities[i]->getScale(), cluster->bodies[i]->predictedOrientation);

	CollisionReport report;
	report.computeManifoldContacts = true;

	for (unsigned int i = 0; i < cluster->dynamicEntities.size(); i++)
	{
		Entity* object1 = cluster->dynamicEntities[i];
		for (unsigned int j = i + 1; j < cluster->dynamicEntities.size(); j++)
		{
			Entity* object2 = cluster->dynamicEntities[j];
			if (Collision::collide(object1->getGlobalBoundingShape(), object2->getGlobalBoundingShape(), &report))
			{
				report.entity1 = object1;
				report.entity2 = object2;
				report.body1 = cluster->bodies[i];
				report.body2 = cluster->bodies[j];

				for (int k = 0; k < report.points.size(); k++)
				{
					if (report.depths[k] > 0.f)
					{
						Constraint constraint;
						constraint.createFromReport(report, k, deltaTime);
						cluster->constraints.push_back(constraint);
					}
				}

				report.clear();
			}
		}
	}

	for (unsigned int i = 0; i < cluster->dynamicEntities.size(); i++)
	{
		Entity* object1 = cluster->dynamicEntities[i];
		for (unsigned int j = 0; j < cluster->staticEntities.size(); j++)
		{
			Entity* object2 = cluster->staticEntities[j];
			if (Collision::collide(object1->getGlobalBoundingShape(), object2->getGlobalBoundingShape(), &report))
			{
				report.entity1 = object1;
				report.entity2 = object2;
				report.body1 = cluster->bodies[i];
				report.body2 = nullptr;

				for (int k = 0; k < report.points.size(); k++)
				{
					if (report.depths[k] > 0.f)
					{
						Constraint constraint;
						constraint.createFromReport(report, k, deltaTime);
						cluster->constraints.push_back(constraint);
					}
				}

				report.clear();
			}
		}
	}
}
void Physics::clearTempoaryStruct(SceneManager* scene)
{
	sweptList.clear();
}
//

//  Solveurs
void Physics::solveConstraint(const unsigned int& clusterIndex, const float& deltaTime)
 {
	Cluster* cluster = &clusters[clusterIndex];
	for (int i = 0; i < SOLVER_MAX_ITERATIONS; i++)
	{
		float maxImpulseCorrection = 0.f;
		for (unsigned int j = 0; j < cluster->constraints.size(); j++)
		{
			Constraint& constraint = cluster->constraints[j];
			glm::vec3 velocity = constraint.computeClosingVelocity();
			float error = constraint.targetLinearVelocity[0] - glm::dot(velocity, constraint.axis[0]);

			/*for (int k = 0; k < constraint.axisCount; k++)
			{
				float error = constraint.targetLinearVelocity[k] - glm::dot(velocity, constraint.axis[k]);
				if (std::abs(error) < 0.0001f)
					continue;

				float impulseLength = 0.8f * error / constraint.velocityChangePerAxis[k];
				float totalImpulse = constraint.accumulationLinear[k] + impulseLength;
				totalImpulse = glm::clamp(totalImpulse, constraint.accumulationLinearMin[k], constraint.accumulationLinearMax[k]);
				impulseLength = totalImpulse - constraint.accumulationLinear[k];
				if (std::abs(impulseLength) < 0.0001f)
					continue;

				constraint.accumulationLinear[k] = totalImpulse;
				if (k == 0 && constraint.frictionLimit)
				{
					float limit = constraint.friction * std::abs(totalImpulse);
					constraint.accumulationLinearMin[1] = constraint.accumulationLinearMin[2] = -limit;
					constraint.accumulationLinearMax[1] = constraint.accumulationLinearMax[2] = limit;
				}

				constraint.body1->linearVelocity += (impulseLength * constraint.body1->inverseMass) * constraint.axis[k];
				constraint.body1->angularVelocity += impulseLength * constraint.rotationPerUnitImpulse1[k];
				if (constraint.body2)
				{
					constraint.body2->linearVelocity -= (impulseLength * constraint.body2->inverseMass) * constraint.axis[k];
					constraint.body2->angularVelocity -= impulseLength * constraint.rotationPerUnitImpulse2[k];
				}

				maxImpulseCorrection = std::max(maxImpulseCorrection, std::abs(impulseLength));
			}*/

			//if (std::abs(error) > SOLVER_ITERATION_THRESHOLD)
			{
				float impulseLength = SOLVER_ITERATION_NORMAL_GAIN * error / constraint.velocityChangePerAxis[0];
				float totalImpulse = constraint.accumulationLinear[0] + impulseLength;
				totalImpulse = glm::clamp(totalImpulse, constraint.accumulationLinearMin[0], constraint.accumulationLinearMax[0]);
				impulseLength = totalImpulse - constraint.accumulationLinear[0];
				constraint.accumulationLinear[0] = totalImpulse;

				constraint.body1->linearVelocity += (impulseLength * constraint.body1->inverseMass) * constraint.axis[0];
				constraint.body1->angularVelocity += impulseLength * constraint.rotationPerUnitImpulse1[0];
				if (constraint.body2)
				{
					constraint.body2->linearVelocity -= (impulseLength * constraint.body2->inverseMass) * constraint.axis[0];
					constraint.body2->angularVelocity -= impulseLength * constraint.rotationPerUnitImpulse2[0];
				}

				maxImpulseCorrection = std::max(maxImpulseCorrection, std::abs(impulseLength));
			}

			error = constraint.targetLinearVelocity[1] - glm::dot(velocity, constraint.axis[1]);
			float impulseLength1 = SOLVER_ITERATION_TANGENT_GAIN * error / constraint.velocityChangePerAxis[1];

			error = constraint.targetLinearVelocity[2] - glm::dot(velocity, constraint.axis[2]);
			float impulseLength2 = SOLVER_ITERATION_TANGENT_GAIN * error / constraint.velocityChangePerAxis[2];

			if (constraint.frictionLimit)
			{
				float totalImpulse1 = constraint.accumulationLinear[1] + impulseLength1;
				float totalImpulse2 = constraint.accumulationLinear[2] + impulseLength2;

				float limit = constraint.friction * std::abs(constraint.accumulationLinear[0]);
				float totalImpulseMag2 = totalImpulse1 * totalImpulse1 + totalImpulse2 * totalImpulse2;
				if (totalImpulseMag2 > limit * limit)
				{
					float f = limit / sqrtf(totalImpulseMag2);
					totalImpulse1 *= f;
					totalImpulse2 *= f;
				}

				impulseLength1 = totalImpulse1 - constraint.accumulationLinear[1];
				constraint.accumulationLinear[1] = totalImpulse1;
				impulseLength2 = totalImpulse2 - constraint.accumulationLinear[2];
				constraint.accumulationLinear[2] = totalImpulse2;
			}
			else
			{
				float totalImpulse1 = constraint.accumulationLinear[1] + impulseLength1;
				totalImpulse1 = glm::clamp(totalImpulse1, constraint.accumulationLinearMin[1], constraint.accumulationLinearMax[1]);
				impulseLength1 = totalImpulse1 - constraint.accumulationLinear[1];
				constraint.accumulationLinear[1] = totalImpulse1;

				float totalImpulse2 = constraint.accumulationLinear[2] + impulseLength2;
				totalImpulse2 = glm::clamp(totalImpulse1, constraint.accumulationLinearMin[2], constraint.accumulationLinearMax[2]);
				impulseLength2 = totalImpulse2 - constraint.accumulationLinear[2];
				constraint.accumulationLinear[2] = totalImpulse1;
			}

			if (std::abs(impulseLength1) > SOLVER_ITERATION_THRESHOLD)
			{
				constraint.body1->linearVelocity += (impulseLength1 * constraint.body1->inverseMass) * constraint.axis[1];
				constraint.body1->angularVelocity += impulseLength1 * constraint.rotationPerUnitImpulse1[1];
				if (constraint.body2)
				{
					constraint.body2->linearVelocity -= (impulseLength1 * constraint.body2->inverseMass) * constraint.axis[1];
					constraint.body2->angularVelocity -= impulseLength1 * constraint.rotationPerUnitImpulse2[1];
				}

				maxImpulseCorrection = std::max(maxImpulseCorrection, std::abs(impulseLength1));
			}

			if (std::abs(impulseLength2) > SOLVER_ITERATION_THRESHOLD)
			{
				constraint.body1->linearVelocity += (impulseLength2 * constraint.body1->inverseMass) * constraint.axis[2];
				constraint.body1->angularVelocity += impulseLength2 * constraint.rotationPerUnitImpulse1[2];
				if (constraint.body2)
				{
					constraint.body2->linearVelocity -= (impulseLength2 * constraint.body2->inverseMass) * constraint.axis[2];
					constraint.body2->angularVelocity -= impulseLength2 * constraint.rotationPerUnitImpulse2[2];
				}

				maxImpulseCorrection = std::max(maxImpulseCorrection, std::abs(impulseLength2));
			}
		}

		if (maxImpulseCorrection < SOLVER_ITERATION_THRESHOLD)
			break;
	}
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
/*void Physics::createReportConstraints(Cluster& cluster, CollisionReport& report)
{
	for (int k = 0; k < report.points.size(); k++)
	{
		Constraint constraint;
		constraint.createFromReport(report, k);
		cluster.constraints.push_back(constraint);
	}
}*/
//



//	Private internal class
void Physics::EntityGraph::clear()
{
	graph.clear();
}
void Physics::EntityGraph::initialize(const std::set<Entity*>& nodes)
{
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