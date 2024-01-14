#include "Physics.h"
#include "Collision.h"

#include <set>

#include "RigidBody.h"
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Scene/SceneManager.h>
#include <World/World.h>
#include <Utiles/Debug.h>

#include <Utiles/Debug.h>
#include <Utiles/ImguiConfig.h>
#include <Physics/Shapes/Collider.h>
#include <Renderer/CameraComponent.h>
#include <Utiles/ProfilerConfig.h>

//#define APPROXIMATION_FACTOR 10.f
//#define SUPERSAMPLING_DELTA 0.01f
#define SOLVER_ITERATION_THRESHOLD 10E-06f


/*
	https://www.sidefx.com/docs/houdini/nodes/dop/rigidbodysolver.html
	https://digitalrune.github.io/DigitalRune-Documentation/html/138fc8fe-c536-40e0-af6b-0fb7e8eb9623.htm#Solutions
*/

#ifdef USE_IMGUI
	bool PhysicDebugWindowEnable = false;
#endif // USE_IMGUI



bool Physics::drawSweptBoxes = false;
bool Physics::drawCollisions = false;
bool Physics::drawClustersAABB = false;


//  Default
Physics::Physics() : gravity(0.f, -9.81f, 0.f, 0.f), proximityTest(vec4f(0), vec4f(0)), defaultFriction(0.7f)
{
	Collision::DispatchMatrixInit();
}
Physics::~Physics()
{}
//

//	Set / get functions
void Physics::setGravity(const vec4f& g) { gravity = g; }
void Physics::setDefaultFriction(const float& f) { defaultFriction = f; };

vec4f Physics::getGravity() const { return gravity; }
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
	SCOPED_CPU_MARKER("Physics Update");
	if (elapsedTime == 0.f)
		return;

	clusterFinder.clear();
	dynamicPairs.clear();
	dynamicCollisions.clear();
	staticCollisions.clear();
	clusters.clear();
	
	predictTransform(elapsedTime);
	computeBoundingShapesAndDetectPairs(elapsedTime, scene);
	computeDynamicClusters();

	for (unsigned int i = 0; i < clusters.size(); i++)
	{
		createConstraint(i, elapsedTime);
		solveConstraint(i, elapsedTime);

		for (unsigned int j = 0; j < clusters[i].dynamicEntities.size(); j++)
		{
			RigidBody* rigidbody = clusters[i].dynamicEntities[j];

			//rigidbody->setPosition(rigidbody->getPosition() + elapsedTime * rigidbody->linearVelocity);
			//glm::fquat dq = glm::fquat(0.f, rigidbody->angularVelocity.x, rigidbody->angularVelocity.y, rigidbody->angularVelocity.z);
			//glm::fquat q = rigidbody->getOrientation() + 0.5f * elapsedTime * dq * rigidbody->getOrientation();
			//rigidbody->setOrientation(glm::normalize(q));
			rigidbody->setExternalForces(vec4f(0.f));
			rigidbody->setExternalTorques(vec4f(0.f));

			rigidbody->linearVelocity *= 1.f - rigidbody->damping;
			rigidbody->angularVelocity *= 1.f - rigidbody->damping;

			scene->updateObject(clusters[i].dynamicEntities[j]->getParentEntity());
		}
	}

	clearTempoaryStruct(scene);
}

bool Physics::collisionTest(const Shape& _shape, SceneManager* scene, uint64_t flags, uint64_t noFlags, CollisionReport* _report)
{
	SCOPED_CPU_MARKER("Physics collisionTest");

	auto aabb = _shape.toAxisAlignedBox();
	proximityTest.result.clear();
	proximityTest.bbMin = aabb.min;
	proximityTest.bbMax = aabb.max;
	proximityList.result.clear();
	proximityList.m_flags = flags;
	proximityList.m_exclusionFlags = noFlags;
	scene->getEntities(&proximityTest, &proximityList);

	CollisionReport bestReport;
	float maxDepth = -1.f;

	for (unsigned int i = 0; i < proximityList.result.size(); i++)
	{
		Entity* entity = proximityList.result[i];

		auto colliderVisitor = [&](Component* componentCollider)
		{
			const Collider* collider = static_cast<const Collider*>(componentCollider);
			if (collider)
			{
				Shape* tmp = collider->m_shape->duplicate();
				tmp->transform(entity->getWorldPosition(), vec4f(entity->getWorldScale()), entity->getWorldOrientation());

				if (Collision::collide(&_shape, tmp, _report))
				{
					if (_report)
					{
						float d = -1.f;
						for (int i = 0; i < _report->depths.size(); i++)
							d = std::max(_report->depths[i], d);
						if (d > maxDepth)
						{
							maxDepth = d;
							bestReport = *_report;
							bestReport.shape1 = (Shape*)&_shape;
							bestReport.shape2 = _report->shape2;
							bestReport.entity2 = entity;
						}
						_report->clear();
					}
					else
					{
						maxDepth = 1.f;
						return true;
					}
				}
				delete tmp;
			}
			return false;
		};

		entity->componentsVisitor(Collider::getStaticClassID(), colliderVisitor);
	}
	return maxDepth >= 0.f;
}
//


//	Pipeline
void Physics::predictTransform(const float& elapsedTime)
{
	SCOPED_CPU_MARKER("PredictTransform");

	for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end();)
	{
		Entity* entity = *it;
		RigidBody* rigidbody = entity->getComponent<RigidBody>();
		if (!rigidbody || rigidbody->getMass() == 0.f)
		{
			entity->getParentWorld()->releaseOwnership(entity);
			it = movingEntity.erase(it);
		}
		else
		{
			if (rigidbody->getType() == RigidBody::RigidBodyType::DYNAMIC)
			{
				rigidbody->previousPosition = rigidbody->getPosition();
				rigidbody->previousOrientation = rigidbody->getOrientation();

				rigidbody->linearAcceleration = gravity * rigidbody->gravityFactor;							// gravity
				rigidbody->linearAcceleration += rigidbody->inverseMass * rigidbody->externalForces;		// other forces
				rigidbody->angularAcceleration = rigidbody->inverseInertia * rigidbody->externalTorques;	// other torques

				rigidbody->linearVelocity += elapsedTime * rigidbody->linearAcceleration;
				rigidbody->angularVelocity += elapsedTime * rigidbody->angularAcceleration;

				vec4f newPosition = rigidbody->previousPosition + elapsedTime * rigidbody->linearVelocity;
				quatf dq = quatf(0.f, rigidbody->angularVelocity.x, rigidbody->angularVelocity.y, rigidbody->angularVelocity.z);
				quatf newOrientation = rigidbody->getOrientation() + (0.5f * elapsedTime) * dq * rigidbody->getOrientation();
				newOrientation.normalize();

				rigidbody->sweptBox = entity->m_worldBoundingBox;
				AxisAlignedBox end = entity->m_localBoundingBox;
				end.transform(newPosition, vec4f(entity->getWorldScale()), newOrientation);
				rigidbody->sweptBox.add(end);

				entity->setWorldTransformation(newPosition, entity->getWorldScale(), newOrientation);
			}
			else if (rigidbody->getType() == RigidBody::RigidBodyType::KINEMATICS)
			{
				rigidbody->previousPosition = rigidbody->getPosition();
				rigidbody->previousOrientation = rigidbody->getOrientation();
				rigidbody->linearAcceleration = gravity * rigidbody->gravityFactor;
				rigidbody->linearVelocity += elapsedTime * rigidbody->linearAcceleration;
				rigidbody->angularVelocity = vec4f::zero;
				rigidbody->angularAcceleration = vec4f::zero;

				vec4f newPosition = rigidbody->previousPosition + elapsedTime * rigidbody->linearVelocity;
				rigidbody->sweptBox = entity->m_worldBoundingBox;
				AxisAlignedBox end = entity->m_localBoundingBox;
				end.transform(newPosition, vec4f(entity->getWorldScale()), rigidbody->previousOrientation);
				rigidbody->sweptBox.add(end);

				entity->setWorldPosition(newPosition);
			}

			++it;
		}
	}
}
void Physics::computeBoundingShapesAndDetectPairs(const float& elapsedTime, SceneManager* scene)
{
	SCOPED_CPU_MARKER("Update shapes and detect pairs");

	for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end(); ++it)
	{
		Entity* entity = *it;
		RigidBody* rigidbody = entity->getComponent<RigidBody>();
		const AxisAlignedBox& box = rigidbody->sweptBox;

		proximityTest.result.clear();
		proximityTest.bbMin = box.min;
		proximityTest.bbMax = box.max;
		proximityList.result.clear();
		proximityList.m_flags = (uint64_t)Entity::Flags::Fl_Collision;
		scene->getEntities(&proximityTest, &proximityList);

		bool collideOnDynamic = false;
		bool collideOnStatic = false;
		for (unsigned int i = 0; i < proximityList.result.size(); i++)
		{
			if (proximityList.result[i] == entity)
				continue;

			//	get shape of concurent entity
			Entity* other = proximityList.result[i];
			RigidBody* body2 = (other->getFlags() & (uint64_t)Entity::Flags::Fl_Physics) ? other->getComponent<RigidBody>() : nullptr;
			AxisAlignedBox* box2 = nullptr;
			if (body2)
				box2 = &body2->sweptBox;
			else
				box2 = &other->m_worldBoundingBox;
			
			//	test collision
			if (Collision::collide_AxisAlignedBoxvsAxisAlignedBox(box.min, box.max, box2->min, box2->max))
			{
				if (body2)
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
			//entity->setTransformation(rigidbody->predictedPosition, entity->getScale(), rigidbody->predictedOrientation);
			scene->updateObject(entity);
		}
		else if (!collideOnDynamic && collideOnStatic)
		{
			// small cluster of one dynamic object
			clusters.push_back(Cluster());
			Cluster& cluster = clusters.back();
			cluster.constraints.clear();
			cluster.dynamicEntities.clear();
			cluster.staticEntities.clear();
			cluster.dynamicEntities.push_back(rigidbody);
			rigidbody->computeWorldShapes();

			std::map<Entity*, std::vector<Entity*>>::iterator it = staticCollisions.find(entity);
			if (it != staticCollisions.end())
			{
				for (unsigned int k = 0; k < it->second.size(); k++)
					cluster.staticEntities.push_back(it->second[k]);
			}
		}
	}
}
void Physics::computeDynamicClusters()
{
	SCOPED_CPU_MARKER("Create clusters");

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
			RigidBody* body = entity->getComponent<RigidBody>();
			cluster.dynamicEntities.emplace_back(body);
			body->computeWorldShapes();

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
	SCOPED_CPU_MARKER("CreateConstraint");

	Cluster* cluster = &clusters[clusterIndex];

	CollisionReport report;
	report.computeManifoldContacts = true;
	std::vector<Component*> entityColliders;

	for (unsigned int i = 0; i < cluster->dynamicEntities.size(); i++)
	{
		RigidBody* body1 = cluster->dynamicEntities[i];

		for (unsigned int j = i + 1; j < cluster->dynamicEntities.size(); j++)
		{
			RigidBody* body2 = cluster->dynamicEntities[j];

			for (unsigned int k = 0; k < body1->worldShapes.size(); k++)
				for (unsigned int l = 0; l < body2->worldShapes.size(); l++)
				{
					if (Collision::collide(body1->worldShapes[k], body2->worldShapes[l], &report))
					{
						report.entity1 = body1->getParentEntity();
						report.entity2 = body2->getParentEntity();
						report.body1 = body1;
						report.body2 = body2;

						for (int k = 0; k < report.points.size(); k++)
						{
							if (report.depths[k] > 0.f)
							{
								Constraint constraint;
								constraint.createFromReport(report, k, deltaTime);
								cluster->constraints.push_back(constraint);
							}
						}
					}
					report.clear();
				}
		}

		for (unsigned int j = 0; j < cluster->staticEntities.size(); j++)
		{
			Entity* entity2 = cluster->staticEntities[j];
			entityColliders.clear();
			entity2->getAllComponents<Collider>(entityColliders);


			auto colliderVisitor = [&](Component* componentCollider)
			{
				const Collider* collider = static_cast<const Collider*>(componentCollider);
				if (collider)
				{
					Shape* tmp = collider->m_shape->duplicate();
					tmp->transform(entity2->getWorldPosition(), vec4f(entity2->getWorldScale()), entity2->getWorldOrientation());

					for (unsigned int k = 0; k < body1->worldShapes.size(); k++)
					{
						if (Collision::collide(body1->worldShapes[k], tmp, &report))
						{
							report.entity1 = body1->getParentEntity();
							report.entity2 = entity2;
							report.body1 = body1;
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
						}
						report.clear();
					}
						

					delete tmp;
				}
				return false;
			};

			entity2->componentsVisitor(Collider::getStaticClassID(), colliderVisitor);
		}
	}
}
void Physics::clearTempoaryStruct(SceneManager* scene)
{
	//sweptList.clear();
}
//

//  Solveurs
int g_maxIterationCount = 200;
float g_contactNormalRelaxation = 0.8f;
float g_contactTangentRelaxation = 0.5f;
void Physics::solveConstraint(const unsigned int& clusterIndex, const float& deltaTime)
 {
	SCOPED_CPU_MARKER("SolveConstraint");

	Cluster* cluster = &clusters[clusterIndex];
	for (int i = 0; i < g_maxIterationCount; i++)
	{
		float maxImpulseCorrection = 0.f;
		for (unsigned int j = 0; j < cluster->constraints.size(); j++)
		{
			Constraint& constraint = cluster->constraints[j];
			vec4f velocity = constraint.computeClosingVelocity();
			float error = constraint.targetLinearVelocity[0] - vec4f::dot(velocity, constraint.axis[0]);

			if (std::abs(error) > SOLVER_ITERATION_THRESHOLD)
			{
				float impulseLength = g_contactNormalRelaxation * error / constraint.velocityChangePerAxis[0];
				float totalImpulse = constraint.accumulationLinear[0] + impulseLength;
				totalImpulse = clamp(totalImpulse, constraint.accumulationLinearMin[0], constraint.accumulationLinearMax[0]);
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

			error = constraint.targetLinearVelocity[1] - vec4f::dot(velocity, constraint.axis[1]);
			float impulseLength1 = g_contactTangentRelaxation * error / constraint.velocityChangePerAxis[1];

			error = constraint.targetLinearVelocity[2] - vec4f::dot(velocity, constraint.axis[2]);
			float impulseLength2 = g_contactTangentRelaxation * error / constraint.velocityChangePerAxis[2];

			if (constraint.frictionLimit)
			{
				float totalImpulse1 = constraint.accumulationLinear[1] + impulseLength1;
				float totalImpulse2 = constraint.accumulationLinear[2] + impulseLength2;

				float limit = constraint.friction * constraint.accumulationLinear[0];
				float totalImpulseMag2 = totalImpulse1 * totalImpulse1 + totalImpulse2 * totalImpulse2;
				if (totalImpulseMag2 > limit * limit)
				{
					float f = limit > 0.f ? limit / sqrtf(totalImpulseMag2) : 0.f;
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
				totalImpulse1 = clamp(totalImpulse1, constraint.accumulationLinearMin[1], constraint.accumulationLinearMax[1]);
				impulseLength1 = totalImpulse1 - constraint.accumulationLinear[1];
				constraint.accumulationLinear[1] = totalImpulse1;

				float totalImpulse2 = constraint.accumulationLinear[2] + impulseLength2;
				totalImpulse2 = clamp(totalImpulse2, constraint.accumulationLinearMin[2], constraint.accumulationLinearMax[2]);
				impulseLength2 = totalImpulse2 - constraint.accumulationLinear[2];
				constraint.accumulationLinear[2] = totalImpulse2;
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

	for (int i = 0; i < g_maxIterationCount; i++)
	{
		float maxCorrection = 0.f;
		for (unsigned int j = 0; j < cluster->constraints.size(); j++)
		{
			Constraint& constraint = cluster->constraints[j];
			vec4f pos1 = constraint.body1->getPosition();
			vec4f p1 = pos1 + constraint.body1->getOrientation() * constraint.localPoint1;
			vec4f p2 = constraint.worldPoint;
			if (constraint.body2)
				p2 = constraint.body2->getPosition() + constraint.body2->getOrientation() * constraint.localPoint2;
			float depth = constraint.depth + vec4f::dot(p2 - p1, constraint.axis[0]);
			if (std::abs(depth) < SOLVER_ITERATION_THRESHOLD)
				continue;

			float invMassSum = constraint.body1->inverseMass;
			if (constraint.body2)
				invMassSum += constraint.body2->inverseMass;
			if (invMassSum < 10E-06f)
				continue;

			float correction = depth * g_contactNormalRelaxation / invMassSum;
			maxCorrection = std::max(maxCorrection, correction);
			float slack = correction * constraint.body1->inverseMass;
			constraint.body1->setPosition(pos1 + slack * constraint.axis[0]);
			if (constraint.body2)
			{
				slack = correction * constraint.body2->inverseMass;
				constraint.body1->setPosition(constraint.body2->getPosition() - slack * constraint.axis[0]);
			}
		}

		if (maxCorrection < SOLVER_ITERATION_THRESHOLD)
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

// Debug
void Physics::debugDraw()
{
	SCOPED_CPU_MARKER("Physics Debug Draw");

#ifdef USE_IMGUI
	if (!PhysicDebugWindowEnable)
		return;
#endif

	const vec4f clustersOffset = vec4f(0.004f);
	const vec4f sweptOffset = vec4f(0.004f);
	const float pointRadius = 0.01f;
	const float depthLength = 10.f;
	const float tangentLength = 0.3f;

	for (unsigned int i = 0; i < clusters.size(); i++)
	{
		const Cluster& cluster = clusters[i];

		if (drawClustersAABB)
		{
			Debug::color = Debug::magenta;
			AxisAlignedBox box = cluster.dynamicEntities[0]->getParentEntity()->m_worldBoundingBox;
			for (int j = 1; j < cluster.dynamicEntities.size(); j++)
				box.add(cluster.dynamicEntities[j]->getParentEntity()->m_worldBoundingBox);
			Debug::drawLineCube(mat4f::identity, box.min - clustersOffset, box.max + clustersOffset);
		}

		if (drawSweptBoxes)
		{
			for (const RigidBody* dynamicBody : cluster.dynamicEntities)
			{
				Debug::color = Debug::yellow;
				Debug::drawLineCube(mat4f::identity, dynamicBody->sweptBox.min - sweptOffset, dynamicBody->sweptBox.max + sweptOffset);
			}
		}

		if (drawCollisions)
		{
			std::vector<Debug::Vertex> debugCache;
			debugCache.reserve(cluster.constraints.size() * 10);
			Debug::color = Debug::red;
			for (const Constraint& constraint : cluster.constraints)
			{
				Debug::drawPoint(constraint.worldPoint);
				// point and collision frame
				/*Debug::color = Debug::red;
				//Debug::drawSphere(constraint.worldPoint, pointRadius);
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint - (constraint.depth * depthLength) * constraint.axis[0]);
				Debug::color = Debug::green;
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint + tangentLength * constraint.axis[1]);
				Debug::color = Debug::blue;
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint + tangentLength * constraint.axis[2]);

				// closing velocity
				Debug::color = Debug::orange;
				Debug::drawLine(constraint.worldPoint, constraint.worldPoint + constraint.computeClosingVelocity());*/

				// computed impulse
				//Debug::color = Debug::yellow;
				vec4f impulse = vec4f(0.f);
				for (int k = 0; k < constraint.axisCount; k++)
					impulse += constraint.accumulationLinear[k] * constraint.axis[k];
				//Debug::drawLine(constraint.worldPoint, constraint.worldPoint + impulse);


				debugCache.push_back({ constraint.worldPoint,  Debug::red });
				debugCache.push_back({ constraint.worldPoint - (constraint.depth * depthLength) * constraint.axis[0],  Debug::red });
				debugCache.push_back({ constraint.worldPoint,  Debug::green });
				debugCache.push_back({ constraint.worldPoint + tangentLength * constraint.axis[1],  Debug::green });
				debugCache.push_back({ constraint.worldPoint,  Debug::blue });
				debugCache.push_back({ constraint.worldPoint + tangentLength * constraint.axis[2],  Debug::blue });
				debugCache.push_back({ constraint.worldPoint,  Debug::orange });
				debugCache.push_back({ constraint.worldPoint + constraint.computeClosingVelocity(),  Debug::orange });
				debugCache.push_back({ constraint.worldPoint,  Debug::orange });
				debugCache.push_back({ constraint.worldPoint + impulse,  Debug::yellow });
			}
			Debug::drawMultiplePrimitive(debugCache.data(), (unsigned int)debugCache.size(), mat4f::identity, GL_LINES);
		}
	}

#ifdef USE_IMGUI
	if (m_drawCollidersAround && PhysicDebugWindowEnable)
	{
		m_collidersQuery.getResult().clear();
		auto& list = m_colliderColector.getResult();
		Debug::setDepthTest(m_enableZtest);
		Debug::setFaceCulling(true);
		Debug::setBlending(false);
		
		std::vector<Component*> entityColliders;
		for (int i = 0; i < list.size(); i++)
		{
			if (list[i] == mainCameraEntity)
				continue;

			entityColliders.clear();
			list[i]->getAllComponents<Collider>(entityColliders);
			for (int j = 0; j < entityColliders.size(); j++)
			{
				Collider* collider = static_cast<Collider*>(entityColliders[j]);
				collider->drawDebug(vec4f(0, 1, 0, 1), m_drawCollidersWired);
			}
		}
	}
#endif // USE_IMGUI
}

#ifdef USE_IMGUI
unsigned int g_GeneratedEntitiesIdCount = 0;
Entity* g_GeneratedObjContainer = nullptr;
#endif
void Physics::drawImGui(World& world)
{
#ifdef USE_IMGUI
	SCOPED_CPU_MARKER("Physics");

	ImGui::Begin("Physics");
	ImGui::PushID(this);

	CameraComponent* mainCamera = world.getMainCamera();
	ImVec4 titleColor = ImVec4(1, 1, 0, 1);

	ImGui::TextColored(titleColor, "Infos");
	ImGui::Text("Object count : %d", movingEntity.size());
	ImGui::Text("Cluster count : %d", clusters.size());
	ImGui::Spacing();

	ImGui::TextColored(titleColor, "Options");
	ImGui::SliderInt("Solver max iteration", &g_maxIterationCount, 5, 1000, "%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Contact normal relaxation", &g_contactNormalRelaxation, 0.1f, 0.99f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Contact tangent relaxation", &g_contactTangentRelaxation, 0.1f, 0.99f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::Checkbox("Draw sweept boxes", &drawSweptBoxes);
	ImGui::Checkbox("Draw collisions", &drawCollisions);
	ImGui::Checkbox("Draw clusters", &drawClustersAABB);
	ImGui::Checkbox("Draw colliders around", &m_drawCollidersAround);
	ImGui::SliderFloat("Draw colliders query size", &m_drawCollidersQuerySize, 5.f, 300.f);
	ImGui::Checkbox("Draw colliders as wired", &m_drawCollidersWired);
	ImGui::Checkbox("Z test", &m_enableZtest);

	if (ImGui::Button("One frame update"))
	{
		stepSimulation(0.016f, &world.getSceneManager());
	}

	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::TextColored(titleColor, "Object thrower");

	ImGui::Combo("Shape code", &m_shapeCode, "Sphere\0Box\0\0");
	ImGui::DragFloat("Velocity", &m_velocity, 0.01f, 0.f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat3("Size", &m_size[0], 0.01f, 0.00001f, 10.f);
	if (ImGui::Button("Throw object !") && mainCamera)
	{
		std::string type;
		switch (m_shapeCode)
		{
			case 1: type = "cube"; break;
			default: type = "sphere"; break;
		}

		if (!g_GeneratedObjContainer)
		{
			g_GeneratedObjContainer = world.getEntityFactory().createEntity();
			g_GeneratedObjContainer->setName("PhysicsObjectsGeneratedContainer");
			g_GeneratedObjContainer->setLocalTransformation(vec4f::zero, 1.f, quatf::identity);
			world.getSceneManager().addToRootList(g_GeneratedObjContainer);
		}

		world.getEntityFactory().createObject(type, [&](Entity* object)
			{
				float radius = m_size.x;
				object->setName("Throwed " + type + " (" + std::to_string(g_GeneratedEntitiesIdCount++) + ")");
				g_GeneratedObjContainer->addChild(object);
				object->setLocalTransformation(mainCamera->getParentEntity()->getWorldPosition() + mainCamera->getForward(), radius, quatf::identity);

				RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
				rb->setMass(radius * radius * radius);
				rb->setBouncyness(0.01f);
				rb->setFriction(0.2f);
				rb->setDamping(0.001f);
				rb->setGravityFactor(1.f);
				rb->setLinearVelocity(m_velocity * mainCamera->getForward());
				object->addComponent(rb);
			});
	}

	ImGui::PopID();
	ImGui::End();

	if (m_drawCollidersAround)
	{
		m_collidersQuery.getResult().clear();
		m_colliderColector.getResult().clear();
		m_colliderColector.m_flags = (uint64_t)Entity::Flags::Fl_Collision;
		m_colliderColector.m_exclusionFlags = 0;

		if (mainCamera)
		{
			mainCameraEntity = mainCamera->getParentEntity();
			vec4f center = mainCameraEntity->getWorldPosition();
			vec4f hsize = vec4f(m_drawCollidersQuerySize);
			m_collidersQuery.bbMin = center - hsize;
			m_collidersQuery.bbMax = center + hsize;
		}


		world.getSceneManager().getEntities(&m_collidersQuery, &m_colliderColector);
	}
#endif // USE_IMGUI
}
//