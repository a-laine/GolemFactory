#include "Physics.h"
#include "Collision.h"

#include <set>

#include "RigidBody.h"
//#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Scene/SceneManager.h>
#include <World/World.h>
#include <EntityComponent/ComponentUpdater.h>
#include <Utiles/Debug.h>

#include <Utiles/Debug.h>
#include <Utiles/ImguiConfig.h>
#include <Physics/Shapes/Collider.h>
#include <Renderer/CameraComponent.h>
#include <Utiles/ProfilerConfig.h>
#include <Terrain/TerrainAreaDrawableComponent.h>
#include <Utiles/JobSystem.h>
#include <Utiles/MixedArray.h>

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

thread_local BoxSceneQuerry g_proximityTest;
thread_local VirtualEntityCollector g_proximityList;

//  Default
Physics::Physics() : gravity(0.f, -9.81f, 0.f, 0.f), defaultFriction(0.7f)
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
	RigidBody* rb = e->getComponent<RigidBody>();
	if (rb && movingEntity.insert(e).second)
	{
		e->getParentWorld()->getOwnership(e);
		rb->m_indexInList = m_physicObjList.add(rb);
	}
}
void Physics::removeMovingEntity(Entity* e)
{
	const auto& it = movingEntity.find(e);
	if (it != movingEntity.end())
	{
		e->getParentWorld()->releaseOwnership(e);

		RigidBody* rb = e->getComponent<RigidBody>();
		m_physicObjList.remove(rb->m_indexInList);
	}
}
Physics::Cluster* Physics::getCLuster(int clusterIndex)
{
	return &m_clusters[clusterIndex];
}
//


//	Public functions
void Physics::stepSimulation2(const float& elapsedTime, SceneManager* scene)
{
	SCOPED_CPU_MARKER("Physics Update");
	if (elapsedTime == 0.f)
		return;

	// prepare
	clusterFinder.clear();
	dynamicPairs.clear();
	dynamicCollisions.clear();
	m_clusters.clear();
	m_physicObjList.clear();
	MixedArray<int, 256> validClusterIndices;

	{
		SCOPED_CPU_MARKER("PredictTransform");
		for (std::set<Entity*>::iterator it = movingEntity.begin(); it != movingEntity.end();)
		{
			Entity* entity = *it;
			RigidBody* rigidbody = entity->getComponent<RigidBody>();
			if (rigidbody)
			{
				rigidbody->m_clusterIndex = -1;
				rigidbody->m_indexInList = -1;
			}

			if (!rigidbody || rigidbody->getMass() == 0.f)
			{
				entity->getParentWorld()->releaseOwnership(entity);
				it = movingEntity.erase(it);
			}
			else
			{
				rigidbody->m_position = rigidbody->getPosition();
				rigidbody->m_orientation = rigidbody->getOrientation();

				Sphere sphstart = entity->m_worldBoundingBox.toSphere();
				sphstart.radius += std::max(0.1f, 0.5f * elapsedTime * rigidbody->m_linearVelocity.getNorm());
				Sphere sphend = sphstart;
				sphend.center += elapsedTime * rigidbody->m_linearVelocity;
				rigidbody->m_sweptBox = sphstart.toAxisAlignedBox();
				rigidbody->m_sweptBox.add(sphend.toAxisAlignedBox());
				rigidbody->m_indexInList = m_physicObjList.add(rigidbody);

				++it;
			}
		}
	}

	{
		SCOPED_CPU_MARKER("ComputeClusters");
		for (int i = 0; i < m_physicObjList.size(); i++)
		{
			RigidBody* rbA = m_physicObjList[i];
			rbA->computeWorldShapes();
			const AxisAlignedBox& boxA = rbA->m_sweptBox;

			for (int j = i + 1; j < m_physicObjList.size(); j++)
			{
				RigidBody* rbB = m_physicObjList[j];
				const AxisAlignedBox& boxB = rbB->m_sweptBox;

				if (!Collision::collide_AxisAlignedBoxvsAxisAlignedBox(boxA.min, boxA.max, boxB.min, boxB.max))
					continue;

				// create new cluster
				if (rbA->m_clusterIndex < 0 && rbB->m_clusterIndex < 0)
				{
					rbA->m_clusterIndex = m_clusters.add(Cluster());
					rbB->m_clusterIndex = rbA->m_clusterIndex;
					Cluster& cluster = m_clusters[rbA->m_clusterIndex];
					cluster.constraints.clear();
					cluster.dynamicEntities.clear();
					cluster.cache.clear();
					cluster.dynamicEntities.push_back(rbA);
					cluster.dynamicEntities.push_back(rbB);
					cluster.cache.m_aabb = boxA;
					cluster.cache.m_aabb.add(boxB);
				}

				// insert A to B
				else if (rbA->m_clusterIndex < 0)
				{
					rbA->m_clusterIndex = rbB->m_clusterIndex;
					Cluster& cluster = m_clusters[rbB->m_clusterIndex];
					cluster.dynamicEntities.push_back(rbA);
					cluster.cache.m_aabb.add(boxA);
				}

				// insert B to A
				else if (rbB->m_clusterIndex < 0)
				{
					rbB->m_clusterIndex = rbA->m_clusterIndex;
					Cluster& cluster = m_clusters[rbA->m_clusterIndex];
					cluster.dynamicEntities.push_back(rbB);
					cluster.cache.m_aabb.add(boxB);
				}

				// merge A to B or B to A
				else if (rbA->m_clusterIndex != rbB->m_clusterIndex)
				{
					bool targetA = rbA->m_clusterIndex < rbB->m_clusterIndex;
					int clusterTargetIndex = targetA ? rbA->m_clusterIndex : rbB->m_clusterIndex;
					int clusterOtherIndex = targetA ? rbB->m_clusterIndex : rbA->m_clusterIndex;
					Cluster& clusterTarget = m_clusters[clusterTargetIndex];
					Cluster& clusterOther = m_clusters[clusterOtherIndex];

					for (RigidBody* rb : clusterOther.dynamicEntities)
					{
						rb->m_clusterIndex = clusterTargetIndex;
						clusterTarget.dynamicEntities.push_back(rb);
					}
					clusterOther.dynamicEntities.clear();
					clusterTarget.cache.m_aabb.add(clusterOther.cache.m_aabb);
					m_clusters.remove(clusterOtherIndex);
				}
			}

			// object is solo
			if (rbA->m_clusterIndex < 0)
			{
				rbA->m_clusterIndex = m_clusters.add(Cluster());
				Cluster& cluster = m_clusters[rbA->m_clusterIndex];
				cluster.constraints.clear();
				cluster.dynamicEntities.clear();
				cluster.cache.clear();
				cluster.dynamicEntities.push_back(rbA);
				cluster.cache.m_aabb = boxA;
			}
		}

		for (int i = 0; i < m_clusters.range(); i++)
		{
			if (!m_clusters.isValid(i))
				continue;

			validClusterIndices.push_back(i);
		}
	}


	if (validClusterIndices.size() > 0)
	{
		SCOPED_CPU_MARKER("MultijobUpdate");
		/*
		Job2 updateLodJob(Job2::JobPriority::HIGH, [&jobLodDatas](int _jobId, void* _data) {
				SCOPED_CPU_MARKER("TerrainArea::updateLodJob");
				TerrainArea* area = jobLodDatas[_jobId].m_area;
				int targetLod = jobLodDatas[_jobId].m_targetLod;
				area->setLod(targetLod);
				Entity* entity = area->m_entity;
				if (entity)
				{
					TerrainAreaDrawableComponent* drawable = entity->getComponent<TerrainAreaDrawableComponent>();
					drawable->setMesh(area->getTerrain()->m_clipmapMeshes[targetLod]);
					drawable->updateData(area->m_tiles[targetLod]);
				}
			});
		JobSystem::getInstance()->dispatchJob(updateLodJob, jobLodDatas.size(), 1);
		updateLodJob.waitCompletion(true);
		
		*/

		Job2 MultijobUpdate(Job2::HIGH, [this, elapsedTime, scene, &validClusterIndices](int _jobId, void* _data) 
			{
				SCOPED_CPU_MARKER("ClusterUpdate");
				int clusterIndex = validClusterIndices[_jobId];
				Cluster& cluster = m_clusters[clusterIndex];

				getCollisionCache(scene, cluster.cache, (uint64_t)Entity::Flags::Fl_Collision, (uint64_t)Entity::Flags::Fl_Physics);

				float oneStepDt = 0.005f;//in sec
				float advanceTime = 0.f;
				int substepCount = 0;
				while (substepCount < 10 && advanceTime < elapsedTime)
				{
					float dt = std::min(oneStepDt, elapsedTime - advanceTime);

					predictTransform2(clusterIndex, dt);
					createConstraint(clusterIndex, dt);
					solveConstraint(clusterIndex, dt);

					for (int i = 0; i < cluster.dynamicEntities.size(); i++)
					{
						RigidBody* rigidbody = cluster.dynamicEntities[i];
						for (int j = 0; j < rigidbody->m_fixedUpdateSuscribers.size(); j++)
						{
							rigidbody->m_fixedUpdateSuscribers[j].m_callback(Component::UpdatePass::ePhysics, dt);
						}
					}

					advanceTime += dt;
					substepCount++;
				}
			});
		JobSystem::getInstance()->dispatchJob(MultijobUpdate, validClusterIndices.size(), 1);
		MultijobUpdate.waitCompletion(true);
	}

	{
		SCOPED_CPU_MARKER("SceneUpdate");
		for (int i = 0; i < m_physicObjList.size(); i++)
		{
			RigidBody* rigidbody = m_physicObjList[i];
			rigidbody->setExternalForces(vec4f(0.f));
			rigidbody->setExternalTorques(vec4f(0.f));
			rigidbody->setPosition(rigidbody->m_position);
			rigidbody->setOrientation(rigidbody->m_orientation);

			if (!scene->isTracked(rigidbody->getParentEntity()))
				scene->addObject(rigidbody->getParentEntity(), 2);
			else
				scene->updateObject(rigidbody->getParentEntity());
		}
	}
}
void Physics::stepSimulation(const float& elapsedTime, SceneManager* scene)
{
	SCOPED_CPU_MARKER("Physics Update");
	if (elapsedTime == 0.f)
		return;

	clusterFinder.clear();
	dynamicPairs.clear();
	dynamicCollisions.clear();
	m_clusters.clear();
	
	predictTransform(elapsedTime);
	computeBoundingPairsClusters2(elapsedTime, scene);
	//computeBoundingShapesAndDetectPairs(elapsedTime, scene);
	//computeDynamicClusters(scene);

	for (int i = 0; i < m_clusters.range(); i++)
	{
		if (!m_clusters.isValid(i))
			continue;

		getCollisionCache(scene, m_clusters[i].cache, (uint64_t)Entity::Flags::Fl_Collision, (uint64_t)Entity::Flags::Fl_Physics);

		createConstraint(i, elapsedTime);
		solveConstraint(i, elapsedTime);

		for (unsigned int j = 0; j < m_clusters[i].dynamicEntities.size(); j++)
		{
			RigidBody* rigidbody = m_clusters[i].dynamicEntities[j];
			rigidbody->setExternalForces(vec4f(0.f));
			rigidbody->setExternalTorques(vec4f(0.f));

			rigidbody->m_linearVelocity *= 1.f - rigidbody->m_damping;
			rigidbody->m_angularVelocity *= 1.f - rigidbody->m_damping;

			if (!scene->isTracked(rigidbody->getParentEntity()))
				scene->addObject(rigidbody->getParentEntity(), 2);
			else
				scene->updateObject(rigidbody->getParentEntity());
		}
	}

	//clearTempoaryStruct(scene);
}

/*bool Physics::collisionTest(const Shape& _shape, SceneManager* scene, uint64_t flags, uint64_t noFlags, CollisionReport* _report)
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
}*/

bool Physics::raycast(Segment ray, SceneManager* scene, uint64_t flags, uint64_t noFlags, bool skipTriggers, RaycastReport* _report)
{
	SCOPED_CPU_MARKER("Physics::raycast");

	auto aabb = ray.toAxisAlignedBox();
	g_proximityTest.result.clear();
	g_proximityTest.bbMin = aabb.min;
	g_proximityTest.bbMax = aabb.max;
	g_proximityList.result.clear();
	g_proximityList.m_flags = flags;
	g_proximityList.m_exclusionFlags = noFlags;
	scene->getEntities(&g_proximityTest, &g_proximityList);

	constexpr float maxValue = 1E12f;
	float minDistance = maxValue;
	for (unsigned int i = 0; i < g_proximityList.result.size(); i++)
	{
		Entity* entity = g_proximityList.result[i];
		if (!Collision::raycast(&ray, &entity->m_worldBoundingBox))
			continue;

		Segment localRay;
		localRay.p1 = entity->getInverseWorldTransformMatrix() * ray.p1;
		localRay.p2 = entity->getInverseWorldTransformMatrix() * ray.p2;
		RaycastReport bestEntityReport, tmpReport;
		float bestEntityMinDistance = maxValue;


		auto colliderVisitor = [&](Component* componentCollider)
		{
			const Collider* collider = static_cast<const Collider*>(componentCollider);
			if (!collider)
				return false;
			if (collider->m_isTrigger && skipTriggers)
				return false;

			if (collider->m_shape)
			{
				if (Collision::raycast(&localRay, collider->m_shape, _report ? &tmpReport : nullptr))
				{
					if (_report)
					{
						if (tmpReport.m_distance < bestEntityMinDistance)
						{
							bestEntityMinDistance = tmpReport.m_distance;
							bestEntityReport = tmpReport;
						}
					}
					else
					{
						minDistance = 0.f;
						return true;
					}
					tmpReport.clear();
				}
			}
			return false;
		};
		entity->componentsVisitor(Collider::getStaticClassID(), colliderVisitor);

		if (_report && bestEntityMinDistance < maxValue)
		{
			vec4f localRayDirection = (localRay.p2 - localRay.p1).getNormal();
			vec4f localClosest = localRay.p1 + bestEntityReport.m_distance * localRayDirection;
			localClosest.w = 1;
			vec4f worldClosest = entity->getWorldTransformMatrix() * localClosest;
			worldClosest.w = 1;
			float distance = (worldClosest - ray.p1).getNorm();

			if (distance < minDistance)
			{
				minDistance = distance;
				*_report = bestEntityReport;
				_report->m_intersection = worldClosest;
				_report->m_distance = distance;
				_report->m_normal = entity->getWorldTransformMatrix() * bestEntityReport.m_normal;
				_report->m_normal.w = 0;
				_report->m_normal.normalize();
			}
		}
		else break;
	}
	return minDistance < maxValue;
}

bool Physics::raycastInCache(const CollisionCache& cache, Segment ray, SceneManager* scene, uint64_t flags, uint64_t noFlags, RaycastReport* _report)
{
	ray.computeDirection();

	SCOPED_CPU_MARKER("Physics::raycastInCache");
	if (!Collision::raycast(&ray, &cache.m_aabb, nullptr))
		return false;

	bool hadCollision = false;
	RaycastReport tmpReport;
	for (unsigned int i = 0; i < cache.m_elements.size(); i++)
	{
		if (Collision::raycast(&ray, cache.m_elements[i].m_shape, _report ? &tmpReport : nullptr))
		{
			hadCollision = true;
			if (_report)
			{
				if (tmpReport.m_distance < _report->m_distance)
				{
					*_report = tmpReport;
					_report->m_entity = cache.m_elements[i].m_entity;
				}
			}
			else return true;
			tmpReport.clear();
		}
	}
	return hadCollision;
}

bool Physics::collisionInCache(const CollisionCache& cache, Shape* shape, SceneManager* scene, uint64_t flags, uint64_t noFlags, CollisionReport* _report)
{
	SCOPED_CPU_MARKER("Physics::collisionInCache");
	if (!Collision::collide(shape, &cache.m_aabb, nullptr))
		return false;

	bool hadCollision = false;
	CollisionReport tmpReport;
	tmpReport.computeManifoldContacts = _report ? _report->computeManifoldContacts : false;
	float maxDepthComputed = -1.f;
	for (unsigned int i = 0; i < cache.m_elements.size(); i++)
	{
		if (Collision::collide(shape, cache.m_elements[i].m_shape, _report ? &tmpReport : nullptr))
		{
			hadCollision = true;
			if (_report)
			{
				float maxDepth = tmpReport.depths[0];
				if (tmpReport.computeManifoldContacts)
				{
					for (int j = 1; j < tmpReport.depths.size(); j++)
						maxDepth = std::max(maxDepth, tmpReport.depths[0]);
				}

				if (maxDepth > maxDepthComputed)
				{
					maxDepthComputed = maxDepth;
					*_report = tmpReport;
					_report->entity1 = cache.m_elements[i].m_entity;
					_report->entity2 = nullptr;
				}
			}
			else return true;
			tmpReport.clear();
		}
	}
	return hadCollision;
}

void Physics::getCollisionCache(SceneManager* scene, CollisionCache& cache, uint64_t flags, uint64_t noFlags)
{
	SCOPED_CPU_MARKER("Physics::getCollisionCache");

	cache.clear();

	/*cache.m_triangles.clear();	cache.m_triangles.reserve(128);
	cache.m_spheres.clear();	cache.m_spheres.reserve(16);
	cache.m_boxes.clear();		cache.m_boxes.reserve(16);
	cache.m_capsules.clear();	cache.m_capsules.reserve(16);
	cache.m_hulls.clear();		cache.m_hulls.reserve(4);
	cache.m_elements.clear();	cache.m_triangles.reserve(32);*/

	//proximityTest.result.clear();
	/*proximityTest.bbMin = cache.m_aabb.min;
	proximityTest.bbMax = cache.m_aabb.max;
	proximityList.result.clear();
	proximityList.m_flags = flags;
	proximityList.m_exclusionFlags = noFlags;*/

	g_proximityTest.result.clear();
	g_proximityTest.bbMin = cache.m_aabb.min;
	g_proximityTest.bbMax = cache.m_aabb.max;
	g_proximityList.result.clear();
	g_proximityList.m_flags = flags;
	g_proximityList.m_exclusionFlags = noFlags;

	scene->getEntities(&g_proximityTest, &g_proximityList);

	for (unsigned int i = 0; i < g_proximityList.result.size(); i++)
	{
		Entity* entity = g_proximityList.result[i];
		if (!Collision::collide(&cache.m_aabb, &entity->m_worldBoundingBox))
			continue;

		OrientedBox box;
		box.base = entity->getInverseWorldTransformMatrix();
		box.min = cache.m_aabb.min;
		box.max = cache.m_aabb.max;
		AxisAlignedBox localaabb = box.toAxisAlignedBox();

		auto colliderVisitor = [&](Component* componentCollider)
			{
				const Collider* collider = static_cast<const Collider*>(componentCollider);
				if (!collider)
					return false;
				if (collider->m_isTrigger)
					return false;

				if (collider->m_shape)
				{
					switch (collider->m_shape->type)
					{
						case Shape::ShapeType::SPHERE:
							{
								Sphere shape = *((Sphere*)collider->m_shape);
								shape.transform(entity->getWorldPosition(), entity->getWorldScale(), entity->getWorldOrientation());
								if (Collision::collide(&cache.m_aabb, &shape, nullptr))
								{
									CollisionCache::Element element;
									element.m_entity = entity;
									element.m_shape = &cache.m_spheres[cache.m_spheres.add(shape)];
									cache.m_elements.push_back(element);
								}
							}
							break;

						case Shape::ShapeType::ORIENTED_BOX:
							{
								OrientedBox shape = *((OrientedBox*)collider->m_shape);
								shape.transform(entity->getWorldPosition(), entity->getWorldScale(), entity->getWorldOrientation());
								if (Collision::collide(&cache.m_aabb, &shape, nullptr))
								{
									CollisionCache::Element element;
									element.m_entity = entity;
									element.m_shape = &cache.m_boxes[cache.m_boxes.add(shape)];
									cache.m_elements.push_back(element);
								}
							}
							break;

						case Shape::ShapeType::AXIS_ALIGNED_BOX:
							{
								AxisAlignedBox* localshape = (AxisAlignedBox*)collider->m_shape;
								OrientedBox shape;
								shape.min = localshape->min;
								shape.max = localshape->max;
								shape.base = entity->getWorldTransformMatrix();

								if (Collision::collide(&cache.m_aabb, &shape, nullptr))
								{
									CollisionCache::Element element;
									element.m_entity = entity;
									element.m_shape = &cache.m_boxes[cache.m_boxes.add(shape)];
									cache.m_elements.push_back(element);
								}
							}
							break;

						case Shape::ShapeType::CAPSULE:
							{
								Capsule shape = *((Capsule*)collider->m_shape);
								shape.transform(entity->getWorldPosition(), entity->getWorldScale(), entity->getWorldOrientation());
								if (Collision::collide(&cache.m_aabb, &shape, nullptr))
								{
									CollisionCache::Element element;
									element.m_entity = entity;
									element.m_shape = &cache.m_capsules[cache.m_capsules.add(shape)];
									cache.m_elements.push_back(element);
								}
							}
							break;

						/*case Shape::ShapeType::HULL:
							{
								Hull shape = *((Hull*)collider->m_shape);
								shape.transform(entity->getWorldPosition(), entity->getWorldScale(), entity->getWorldOrientation());
								if (Collision::collide(&cache.m_aabb, &shape, nullptr))
								{
									CollisionCache::Element element;
									element.m_entity = entity;
									element.m_shape = &cache.m_hulls[cache.m_hulls.add(shape)];
									cache.m_elements.push_back(element);
								}
							}
							break;*/


						case Shape::ShapeType::TRIANGLE:
							{
							Triangle shape = *((Triangle*)collider->m_shape);
								shape.transform(entity->getWorldPosition(), entity->getWorldScale(), entity->getWorldOrientation());
								if (Collision::collide(&cache.m_aabb, &shape, nullptr))
								{
									CollisionCache::Element element;
									element.m_entity = entity;
									element.m_shape = &cache.m_triangles[cache.m_triangles.add(shape)];
									cache.m_elements.push_back(element);
								}
							}
							break;

						default:
							break;
					}
				}
				return false;
			};
		entity->componentsVisitor(Collider::getStaticClassID(), colliderVisitor);

		TerrainAreaDrawableComponent* terrainArea = entity->getComponent<TerrainAreaDrawableComponent>();
		if (terrainArea)
			terrainArea->getArea()->getCollisionInCache(cache);
	}
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
		if (rigidbody)
			rigidbody->m_clusterIndex = -1;

		if (!rigidbody || rigidbody->getMass() == 0.f)
		{
			entity->getParentWorld()->releaseOwnership(entity);
			it = movingEntity.erase(it);
		}
		else
		{
			if (rigidbody->getType() == RigidBody::RigidBodyType::DYNAMIC)
			{
				rigidbody->m_previousPosition = rigidbody->getPosition();
				rigidbody->m_previousOrientation = rigidbody->getOrientation();

				rigidbody->m_linearAcceleration = gravity * rigidbody->m_gravityFactor;							// gravity
				rigidbody->m_linearAcceleration += rigidbody->m_inverseMass * rigidbody->m_externalForces;		// other forces
				rigidbody->m_angularAcceleration = rigidbody->m_inverseInertia * rigidbody->m_externalTorques;	// other torques

				rigidbody->m_linearVelocity += elapsedTime * rigidbody->m_linearAcceleration;
				rigidbody->m_angularVelocity += elapsedTime * rigidbody->m_angularAcceleration;

				vec4f newPosition = rigidbody->m_previousPosition + elapsedTime * rigidbody->m_linearVelocity;
				quatf dq = quatf(0.f, rigidbody->m_angularVelocity.x, rigidbody->m_angularVelocity.y, rigidbody->m_angularVelocity.z);
				quatf newOrientation = rigidbody->getOrientation() + (0.5f * elapsedTime) * dq * rigidbody->getOrientation();
				newOrientation.normalize();

				rigidbody->m_sweptBox = entity->m_worldBoundingBox;
				AxisAlignedBox end = entity->m_localBoundingBox;
				end.transform(newPosition, vec4f(entity->getWorldScale()), newOrientation);
				rigidbody->m_sweptBox.add(end);

				entity->setWorldTransformation(newPosition, entity->getWorldScale(), newOrientation);
			}
			else if (rigidbody->getType() == RigidBody::RigidBodyType::KINEMATICS)
			{
				rigidbody->m_previousPosition = rigidbody->getPosition();
				rigidbody->m_previousOrientation = rigidbody->getOrientation();
				rigidbody->m_linearAcceleration = gravity * rigidbody->m_gravityFactor;
				rigidbody->m_linearVelocity += elapsedTime * rigidbody->m_linearAcceleration;
				rigidbody->m_angularVelocity = vec4f::zero;
				rigidbody->m_angularAcceleration = vec4f::zero;

				vec4f newPosition = rigidbody->m_previousPosition + elapsedTime * rigidbody->m_linearVelocity;
				rigidbody->m_sweptBox = entity->m_worldBoundingBox;
				AxisAlignedBox end = entity->m_localBoundingBox;
				end.transform(newPosition, vec4f(entity->getWorldScale()), rigidbody->m_previousOrientation);
				rigidbody->m_sweptBox.add(end);

				entity->setWorldPosition(newPosition);
			}

			++it;
		}
	}
}

void Physics::predictTransform2(const unsigned int& clusterIndex, const float& deltaTime)
{
	SCOPED_CPU_MARKER("PredictTransform2");
	Cluster& cluster = m_clusters[clusterIndex];
	
	for (int i = 0; i < cluster.dynamicEntities.size(); i++)
	{
		RigidBody* rigidbody = cluster.dynamicEntities[i];
		if (rigidbody->getType() == RigidBody::RigidBodyType::DYNAMIC)
		{
			rigidbody->m_previousPosition = rigidbody->m_position;
			rigidbody->m_previousOrientation = rigidbody->m_orientation;

			rigidbody->m_linearAcceleration = gravity * rigidbody->m_gravityFactor;							// gravity
			rigidbody->m_linearAcceleration += rigidbody->m_inverseMass * rigidbody->m_externalForces;		// other forces
			rigidbody->m_angularAcceleration = rigidbody->m_inverseInertia * rigidbody->m_externalTorques;	// other torques

			rigidbody->m_linearVelocity *= 1.f - rigidbody->m_damping;
			rigidbody->m_angularVelocity *= 1.f - rigidbody->m_damping;
			rigidbody->m_linearVelocity += deltaTime * rigidbody->m_linearAcceleration;
			rigidbody->m_angularVelocity += deltaTime * rigidbody->m_angularAcceleration;

			rigidbody->m_position = rigidbody->m_previousPosition + deltaTime * rigidbody->m_linearVelocity;
			quatf dq = quatf(0.f, rigidbody->m_angularVelocity.x, rigidbody->m_angularVelocity.y, rigidbody->m_angularVelocity.z);
			rigidbody->m_orientation = rigidbody->m_previousOrientation + (0.5f * deltaTime) * dq * rigidbody->m_previousOrientation;
			rigidbody->m_orientation.normalize();
		}
		else if (rigidbody->getType() == RigidBody::RigidBodyType::KINEMATICS)
		{
			rigidbody->m_previousPosition = rigidbody->m_position;
			rigidbody->m_previousOrientation = rigidbody->m_orientation;
			rigidbody->m_linearAcceleration = gravity * rigidbody->m_gravityFactor;
			rigidbody->m_linearAcceleration += rigidbody->m_inverseMass * rigidbody->m_externalForces;		// other forces
			rigidbody->m_linearVelocity += deltaTime * rigidbody->m_linearAcceleration;
			rigidbody->m_angularVelocity = vec4f::zero;
			rigidbody->m_angularAcceleration = vec4f::zero;

			rigidbody->m_position = rigidbody->m_previousPosition + deltaTime * rigidbody->m_linearVelocity;
			quatf dq = quatf(0.f, rigidbody->m_angularVelocity.x, rigidbody->m_angularVelocity.y, rigidbody->m_angularVelocity.z);
			rigidbody->m_orientation = rigidbody->m_previousOrientation + (0.5f * deltaTime) * dq * rigidbody->m_previousOrientation;
			rigidbody->m_orientation.normalize();
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
		const AxisAlignedBox& box = rigidbody->m_sweptBox;

		g_proximityTest.result.clear();
		g_proximityTest.bbMin = box.min;
		g_proximityTest.bbMax = box.max;
		g_proximityList.result.clear();
		g_proximityList.m_flags = (uint64_t)Entity::Flags::Fl_Physics | (uint64_t)Entity::Flags::Fl_Collision;
		scene->getEntities(&g_proximityTest, &g_proximityList);

		bool collideOnDynamic = false;
		bool collideOnStatic = false;
		for (unsigned int i = 0; i < g_proximityList.result.size(); i++)
		{
			if (g_proximityList.result[i] == entity)
				continue;

			//	get shape of concurent entity
			Entity* other = g_proximityList.result[i];
			RigidBody* body2 = other->getComponent<RigidBody>();// (other->getFlags() & (uint64_t)Entity::Flags::Fl_Physics) ? : nullptr;
			AxisAlignedBox* box2 = nullptr;
			if (body2)
				box2 = &body2->m_sweptBox;
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
					//staticCollisions[entity].push_back(proximityList.result[i]);
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
			rigidbody->m_clusterIndex = m_clusters.add(Cluster());
			Cluster& cluster = m_clusters[rigidbody->m_clusterIndex];
			cluster.constraints.clear();
			cluster.dynamicEntities.clear();
			cluster.cache.clear();
			cluster.dynamicEntities.push_back(rigidbody);
			rigidbody->computeWorldShapes();

			cluster.cache.m_aabb = cluster.dynamicEntities[0]->m_sweptBox;
			getCollisionCache(scene, cluster.cache, (uint64_t)Entity::Flags::Fl_Collision, (uint64_t)Entity::Flags::Fl_Physics);

			/*std::map<Entity*, std::vector<Entity*>>::iterator it = staticCollisions.find(entity);
			if (it != staticCollisions.end())
			{
				for (unsigned int k = 0; k < it->second.size(); k++)
					cluster.staticEntities.push_back(it->second[k]);
			}*/
		}
	}
}
void Physics::computeDynamicClusters(SceneManager* scene)
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
		int clusterIndex = m_clusters.add(Cluster());
		Cluster& cluster = m_clusters[clusterIndex];
		std::set<Entity*> staticEntities;

		for (unsigned int j = 0; j < clusterList[i].size(); j++)
		{
			Entity* entity = clusterList[i][j];
			RigidBody* body = entity->getComponent<RigidBody>();
			cluster.dynamicEntities.emplace_back(body);
			body->computeWorldShapes();

			/*std::map<Entity*, std::vector<Entity*>>::iterator it = staticCollisions.find(entity);
			if (it != staticCollisions.end())
			{
				for (unsigned int k = 0; k < it->second.size(); k++)
					staticEntities.insert(it->second[k]);
			}*/
		}

		if (cluster.dynamicEntities.empty())
			m_clusters.remove(clusterIndex);
		/*else
		{
			for (Entity* entity : staticEntities)
				cluster.staticEntities.push_back(entity);
		}*/

		else
		{
			for (int i = 1; i < cluster.dynamicEntities.size(); i++)
				cluster.dynamicEntities[i]->m_clusterIndex = clusterIndex;

			cluster.cache.m_aabb = cluster.dynamicEntities[0]->m_sweptBox;
			for (int i = 1; i < cluster.dynamicEntities.size(); i++)
				cluster.cache.m_aabb.add(cluster.dynamicEntities[i]->m_sweptBox);

			getCollisionCache(scene, cluster.cache, (uint64_t)Entity::Flags::Fl_Collision, (uint64_t)Entity::Flags::Fl_Physics);
		}
	}
}
void Physics::computeBoundingPairsClusters2(const float& elapsedTime, SceneManager* scene)
{
	for (int i = 0; i < m_physicObjList.range(); i++)
	{
		if (!m_physicObjList.isValid(i))
			continue;

		RigidBody* rbA = m_physicObjList[i];
		rbA->computeWorldShapes();
		const AxisAlignedBox& boxA = rbA->m_sweptBox;

		for (int j = i + 1; j < m_physicObjList.range(); j++)
		{
			if (!m_physicObjList.isValid(j))
				continue;

			RigidBody* rbB = m_physicObjList[j];
			const AxisAlignedBox& boxB = rbB->m_sweptBox;

			if (!Collision::collide_AxisAlignedBoxvsAxisAlignedBox(boxA.min, boxA.max, boxB.min, boxB.max))
				continue;

			if (rbA->m_clusterIndex < 0 && rbB->m_clusterIndex < 0)
			{
				rbA->m_clusterIndex = m_clusters.add(Cluster());
				rbB->m_clusterIndex = rbA->m_clusterIndex;
				Cluster& cluster = m_clusters[rbA->m_clusterIndex];
				cluster.constraints.clear();
				cluster.dynamicEntities.clear();
				cluster.cache.clear();
				cluster.dynamicEntities.push_back(rbA);
				cluster.dynamicEntities.push_back(rbB);
				cluster.cache.m_aabb = boxA;
				cluster.cache.m_aabb.add(boxB);
			}
			else if (rbA->m_clusterIndex < 0)
			{
				rbA->m_clusterIndex = rbB->m_clusterIndex;
				Cluster& cluster = m_clusters[rbB->m_clusterIndex];
				cluster.dynamicEntities.push_back(rbA);
				cluster.cache.m_aabb.add(boxA);
			}
			else if (rbB->m_clusterIndex < 0)
			{
				rbB->m_clusterIndex = rbA->m_clusterIndex;
				Cluster& cluster = m_clusters[rbA->m_clusterIndex];
				cluster.dynamicEntities.push_back(rbB);
				cluster.cache.m_aabb.add(boxB);
			}
			else if (rbA->m_clusterIndex != rbB->m_clusterIndex)
			{
				bool targetA = rbA->m_clusterIndex < rbB->m_clusterIndex;
				int clusterTargetIndex = targetA ? rbA->m_clusterIndex : rbB->m_clusterIndex;
				int clusterOtherIndex = targetA ? rbB->m_clusterIndex : rbA->m_clusterIndex;
				Cluster& clusterTarget = m_clusters[clusterTargetIndex];
				Cluster& clusterOther = m_clusters[clusterOtherIndex];

				for (RigidBody* rb : clusterOther.dynamicEntities)
				{
					rb->m_clusterIndex = clusterTargetIndex;
					clusterTarget.dynamicEntities.push_back(rb);
				}
				clusterOther.dynamicEntities.clear();
				clusterTarget.cache.m_aabb.add(clusterOther.cache.m_aabb);
				m_clusters.remove(clusterOtherIndex);
			}
		}

		if (rbA->m_clusterIndex < 0)
		{
			rbA->m_clusterIndex = m_clusters.add(Cluster());
			Cluster& cluster = m_clusters[rbA->m_clusterIndex];
			cluster.constraints.clear();
			cluster.dynamicEntities.clear();
			cluster.cache.clear();
			cluster.dynamicEntities.push_back(rbA);
			cluster.cache.m_aabb = boxA;
		}
	}

	/*for (int i = 0; i < m_clusters.range(); i++)
	{
		if (!m_clusters.isValid(i))
			continue;

		getCollisionCache(scene, m_clusters[i].cache, (uint64_t)Entity::Flags::Fl_Collision, (uint64_t)Entity::Flags::Fl_Physics);
	}*/
}

void Physics::createConstraint(const unsigned int& clusterIndex, const float& deltaTime)
{
	SCOPED_CPU_MARKER("CreateConstraint");

	Cluster* cluster = &m_clusters[clusterIndex];

	CollisionReport report;
	report.computeManifoldContacts = true;
	std::vector<Component*> entityColliders;

	for (unsigned int i = 0; i < cluster->dynamicEntities.size(); i++)
	{
		RigidBody* body1 = cluster->dynamicEntities[i];

		for (unsigned int j = i + 1; j < cluster->dynamicEntities.size(); j++)
		{
			RigidBody* body2 = cluster->dynamicEntities[j];

			for (unsigned int k = 0; k < body1->m_worldShapes.size(); k++)
				for (unsigned int l = 0; l < body2->m_worldShapes.size(); l++)
				{
					if (Collision::collide(body1->m_worldShapes[k], body2->m_worldShapes[l], &report))
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

		/*for (unsigned int j = 0; j < cluster->staticEntities.size(); j++)
		{
			Entity* entity2 = cluster->staticEntities[j];
			//entityColliders.clear();
			//entity2->getAllComponents<Collider>(entityColliders);


			auto colliderVisitor = [&](Component* componentCollider)
			{
				const Collider* collider = static_cast<const Collider*>(componentCollider);
				if (collider && !collider->m_isTrigger)
				{
					Shape* tmp = collider->m_shape->duplicate();
					tmp->transform(entity2->getWorldPosition(), vec4f(entity2->getWorldScale()), entity2->getWorldOrientation());

					for (unsigned int k = 0; k < body1->m_worldShapes.size(); k++)
					{
						if (Collision::collide(body1->m_worldShapes[k], tmp, &report))
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


			TerrainAreaDrawableComponent* terrainArea = entity2->getComponent<TerrainAreaDrawableComponent>();
			//if (terrainArea)
			//	terrainArea->getArea()->getCollisionInCache(cache);
		}*/

		for (int i = 0; i < cluster->cache.m_elements.size(); i++)
		{
			const Shape* shape = cluster->cache.m_elements[i].m_shape;
			Entity* entity = cluster->cache.m_elements[i].m_entity;
			for (unsigned int k = 0; k < body1->m_worldShapes.size(); k++)
			{
				if (Collision::collide(body1->m_worldShapes[k], shape, &report))
				{
					report.entity1 = body1->getParentEntity();
					report.entity2 = entity;
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
		}
	}
}
void Physics::clearTempoaryStruct(SceneManager* scene)
{
	//sweptList.clear();
}
//

//  Solveurs
int g_maxIterationCount = 20;
float g_contactNormalRelaxation = 0.8f;
float g_contactTangentRelaxation = 0.7f;
void Physics::solveConstraint(const unsigned int& clusterIndex, const float& deltaTime)
{
	SCOPED_CPU_MARKER("SolveConstraint");

	Cluster* cluster = &m_clusters[clusterIndex];
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

				constraint.body1->m_linearVelocity += (impulseLength * constraint.body1->m_inverseMass) * constraint.axis[0];
				constraint.body1->m_angularVelocity += impulseLength * constraint.rotationPerUnitImpulse1[0];
				if (constraint.body2)
				{
					constraint.body2->m_linearVelocity -= (impulseLength * constraint.body2->m_inverseMass) * constraint.axis[0];
					constraint.body2->m_angularVelocity -= impulseLength * constraint.rotationPerUnitImpulse2[0];
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
				constraint.body1->m_linearVelocity += (impulseLength1 * constraint.body1->m_inverseMass) * constraint.axis[1];
				constraint.body1->m_angularVelocity += impulseLength1 * constraint.rotationPerUnitImpulse1[1];
				if (constraint.body2)
				{
					constraint.body2->m_linearVelocity -= (impulseLength1 * constraint.body2->m_inverseMass) * constraint.axis[1];
					constraint.body2->m_angularVelocity -= impulseLength1 * constraint.rotationPerUnitImpulse2[1];
				}

				maxImpulseCorrection = std::max(maxImpulseCorrection, std::abs(impulseLength1));
			}

			if (std::abs(impulseLength2) > SOLVER_ITERATION_THRESHOLD)
			{
				constraint.body1->m_linearVelocity += (impulseLength2 * constraint.body1->m_inverseMass) * constraint.axis[2];
				constraint.body1->m_angularVelocity += impulseLength2 * constraint.rotationPerUnitImpulse1[2];
				if (constraint.body2)
				{
					constraint.body2->m_linearVelocity -= (impulseLength2 * constraint.body2->m_inverseMass) * constraint.axis[2];
					constraint.body2->m_angularVelocity -= impulseLength2 * constraint.rotationPerUnitImpulse2[2];
				}

				maxImpulseCorrection = std::max(maxImpulseCorrection, std::abs(impulseLength2));
			}
		}

		if (maxImpulseCorrection < SOLVER_ITERATION_THRESHOLD)
			break;
	}

	//return;
	for (int i = 0; i < g_maxIterationCount; i++)
	{
		float maxCorrection = 0.f;
		for (unsigned int j = 0; j < cluster->constraints.size(); j++)
		{
			Constraint& constraint = cluster->constraints[j];
			vec4f p1 = constraint.body1->m_position + constraint.body1->m_orientation * constraint.localPoint1;
			vec4f p2 = constraint.worldPoint + constraint.depth * constraint.axis[0];
			if (constraint.body2)
				p2 = constraint.body2->m_position + constraint.body2->m_orientation * constraint.localPoint2;

			float error = vec4f::dot(p2 - p1, constraint.axis[0]);
			if (error < SOLVER_ITERATION_THRESHOLD)
				continue;

			float invMassSum = constraint.body1->m_inverseMass;
			if (constraint.body2)
				invMassSum += constraint.body2->m_inverseMass;
			if (invMassSum < 10E-06f)
				continue;

			float correction = error * g_contactNormalRelaxation / invMassSum;
			maxCorrection = std::max(maxCorrection, std::abs(correction));
			float slack = correction * constraint.body1->m_inverseMass;
			constraint.body1->m_position += slack * constraint.axis[0];
			if (constraint.body2)
			{
				slack = correction * constraint.body2->m_inverseMass;
				constraint.body2->m_position -= slack * constraint.axis[0];
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
		if (solver < rigidbody->m_solver)
			solver = rigidbody->m_solver;
	}
	return solver;
}


void Physics::CollisionCache::clear()
{
	m_triangles.clear();
	m_spheres.clear();
	m_boxes.clear();
	m_capsules.clear();
	m_elements.clear();
}
void Physics::CollisionCache::debugDraw(bool wireframe, vec4f baseColor) const
{
	Debug::setBlending(baseColor.w != 1.f && !wireframe);
	const auto randomColorA = [](int seed)
		{
			vec4f res(0.f);
			int n = (seed << 13) ^ seed;
			int seed2 = n;
			n = n * (n * n * 15731 + 789221) + 1376312589;
			res.x = float(n & 0x0fffffff) / float(0x0fffffff);

			n = (seed2 << 13) ^ seed2;
			int seed3 = n;
			n = n * (n * n * 15731 + 789221) + 1376312589;
			res.y = float(n & 0x0fffffff) / float(0x0fffffff);

			n = seed3;
			n = (n << 13) ^ n;
			n = n * (n * n * 15731 + 789221) + 1376312589;
			res.z = float(n & 0x0fffffff) / float(0x0fffffff);

			return res;
		};
	const auto randomColorB = [](float x, float y)
		{
			vec4f res(0.f);
			int n = (int)(x * 311 + y * 113);
			n = (n << 13) ^ n;
			n = n * (n * n * 15731 + 789221) + 1376312589;
			res.x = float(n & 0x0fffffff) / float(0x0fffffff);

			n = (int)(x * 313 + y * 109);
			n = (n << 13) ^ n;
			n = n * (n * n * 15731 + 789221) + 1376312589;
			res.y = float(n & 0x0fffffff) / float(0x0fffffff);

			n = (int)(x * 317 + y * 107);
			n = (n << 13) ^ n;
			n = n * (n * n * 15731 + 789221) + 1376312589;
			res.z = float(n & 0x0fffffff) / float(0x0fffffff);

			return res;
		};
	
	for (const Physics::CollisionCache::Element& element : m_elements)
	{
		Debug::color = 0.9f * baseColor + 0.1f * randomColorA((intptr_t)element.m_shape);
		Debug::color.w = baseColor.w;
		switch (element.m_shape->type)
		{
			case Shape::ShapeType::SPHERE:
			{
				const Sphere* sph = (const Sphere*)element.m_shape;
				if (wireframe)
					Debug::drawLineSphere(sph->center, sph->radius);
				else
					Debug::drawSphere(sph->center, sph->radius);
			}
			break;
		case Shape::ShapeType::ORIENTED_BOX:
			{
				const OrientedBox* box = (const OrientedBox*)element.m_shape;
				if (wireframe)
					Debug::drawLineCube(box->base, box->min, box->max);
				else
					Debug::drawCube(box->base, box->min, box->max);
			}
			break;
		case Shape::ShapeType::AXIS_ALIGNED_BOX:
			{
				const AxisAlignedBox* box = (const AxisAlignedBox*)element.m_shape;
				if (wireframe)
					Debug::drawLineCube(mat4f::identity, box->min, box->max);
				else
					Debug::drawCube(mat4f::identity, box->min, box->max);
			}
			break;
		case Shape::ShapeType::CAPSULE:
			{
				const Capsule* cap = (const Capsule*)element.m_shape;
				if (wireframe)
					Debug::drawLineCapsule(cap->p1, cap->p2, cap->radius);
				else
					Debug::drawCapsule(cap->p1, cap->p2, cap->radius);
			}
			break;
		default:
			break;
		}
	}

	int bufferSize = 0;
	Debug::Vertex tmpBuffer[64 * 3];
	for (int i = 0; i < m_triangles.size(); i++)
	{
		const Triangle& triangle = m_triangles[i];
		vec4f tnormal = vec4f::cross(triangle.p2 - triangle.p1, triangle.p3 - triangle.p1);
		vec4f center = 0.333f * (triangle.p2 + triangle.p1 + triangle.p3); center.w = 1.f;
		vec4f color = 0.9f * baseColor + 0.1f * randomColorB(center.x, center.z);
		color.w = baseColor.w;

		if (wireframe)
		{
			tmpBuffer[bufferSize + 0].m_position = triangle.p1;  tmpBuffer[bufferSize].m_color = color;
			tmpBuffer[bufferSize + 1].m_position = triangle.p3;  tmpBuffer[bufferSize + 1].m_color = color;
			tmpBuffer[bufferSize + 2].m_position = triangle.p1;  tmpBuffer[bufferSize + 2].m_color = color;
			tmpBuffer[bufferSize + 3].m_position = triangle.p2;  tmpBuffer[bufferSize + 3].m_color = color;
			tmpBuffer[bufferSize + 4].m_position = triangle.p2;  tmpBuffer[bufferSize + 4].m_color = color;
			tmpBuffer[bufferSize + 5].m_position = triangle.p3;  tmpBuffer[bufferSize + 5].m_color = color;
			bufferSize += 6;
		}
		else
		{
			tmpBuffer[bufferSize + 0].m_position = triangle.p1;  tmpBuffer[bufferSize].m_color = color;
			tmpBuffer[bufferSize + 1].m_position = triangle.p3;  tmpBuffer[bufferSize + 1].m_color = color;
			tmpBuffer[bufferSize + 2].m_position = triangle.p2;  tmpBuffer[bufferSize + 2].m_color = color;
			bufferSize += 3;
		}

		//Debug::color = Debug::red; Debug::drawLine(center, center + tnormal);

		if (bufferSize == 64 * 3)
		{
			Debug::drawMultiplePrimitive(tmpBuffer, bufferSize, mat4f::identity, wireframe ? GL_LINES : GL_TRIANGLES);
			bufferSize = 0;
		}
	}
	if (bufferSize)
	{
		Debug::drawMultiplePrimitive(tmpBuffer, bufferSize, mat4f::identity, wireframe ? GL_LINES : GL_TRIANGLES);
		bufferSize = 0;
	}
	Debug::setBlending(false);
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

	for (int i = 0; i < m_clusters.range(); i++)
	{
		if (!m_clusters.isValid(i))
			continue;

		const Cluster& cluster = m_clusters[i];

		if (drawClustersAABB)
		{
			Debug::color = Debug::magenta;
			//AxisAlignedBox box = cluster.dynamicEntities[0]->getParentEntity()->m_worldBoundingBox;
			//for (int j = 1; j < cluster.dynamicEntities.size(); j++)
			//	box.add(cluster.dynamicEntities[j]->getParentEntity()->m_worldBoundingBox);
			Debug::drawLineCube(mat4f::identity, cluster.cache.m_aabb.min - clustersOffset, cluster.cache.m_aabb.max + clustersOffset);
		}

		if (drawSweptBoxes)
		{
			for (const RigidBody* dynamicBody : cluster.dynamicEntities)
			{
				Debug::color = Debug::yellow;
				Debug::drawLineCube(mat4f::identity, dynamicBody->m_sweptBox.min - sweptOffset, dynamicBody->m_sweptBox.max + sweptOffset);
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
				collider->drawDebug(collider->m_isTrigger ? vec4f(0, 1, 1, 1) : vec4f(0, 1, 0, 1), m_drawCollidersWired);
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
	ImGui::Text("Cluster count : %d", m_clusters.size());
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
		stepSimulation2(0.016f, &world.getSceneManager());
		//stepSimulation(0.005f, &world.getSceneManager());
		//stepSimulation(0.005f, &world.getSceneManager());
	}

	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::TextColored(titleColor, "Object thrower");

	ImGui::Combo("Shape code", &m_shapeCode, "Sphere\0Box\0\0");
	ImGui::DragFloat("Velocity", &m_velocity, 0.01f, 0.f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat3("Size", &m_size[0], 0.01f, 0.00001f, 10.f);
	ImGui::Checkbox("Select trowed object", &m_autoSelectThrowedObject);
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
			g_GeneratedObjContainer->setLocalTransformation(vec4f::zero, vec4f::one, quatf::identity);
			world.getSceneManager().addToRootList(g_GeneratedObjContainer);
		}

		world.getEntityFactory().createObject(type, [&](Entity* object)
			{
				float radius = m_size.x;
				object->setName("Throwed " + type + " (" + std::to_string(g_GeneratedEntitiesIdCount++) + ")");
				g_GeneratedObjContainer->addChild(object);
				object->setLocalTransformation(mainCamera->getParentEntity()->getWorldPosition() + mainCamera->getForward(), vec4f(radius, radius, radius, 1.f), quatf::identity);

				RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
				rb->setMass(1000);// radius* radius* radius);
				rb->setBouncyness(0.01f);
				rb->setFriction(0.2f);
				rb->setDamping(0.001f);
				rb->setGravityFactor(1.f);
				rb->setLinearVelocity(m_velocity * mainCamera->getForward());
				object->addComponent(rb);

				if (m_autoSelectThrowedObject)
					world.getSceneManager().selectEntity(world, object);
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