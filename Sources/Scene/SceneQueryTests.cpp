#include "SceneQueryTests.h"

#include <Physics/Collision.h>
#include <EntityComponent/Entity.hpp>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Resources/Mesh.h>


//	coefficient for intersection test computation (to avoid artefacts)
#define RAY_COEFF				0.2f
#define FRUSTRUM_COEFF			2.f


DefaultSceneManagerBoxTest::DefaultSceneManagerBoxTest(const glm::vec3& cornerMin, const glm::vec3& cornerMax) : bbMin(cornerMin), bbMax(cornerMax)
{}
SceneManager::CollisionType DefaultSceneManagerBoxTest::operator() (const NodeVirtual* node) const
{
	//	AABB/AABB collision test with special case if node is completely inside query box
	const glm::vec3 nodeHalfSize = node->getSize() * 0.5f;
	const glm::vec3 halfSize = 0.5f * (bbMax - bbMin);
	const glm::vec3 allowance(node->allowanceSize);
	const glm::vec3 p = glm::max(glm::abs(node->getCenter() - 0.5f * (bbMax + bbMin)) - allowance, 0.f);

	if (p.x > halfSize.x + nodeHalfSize.x || p.y > halfSize.y + nodeHalfSize.y || p.z > halfSize.z + nodeHalfSize.z) return SceneManager::NONE;
	if (p.x <= halfSize.x - nodeHalfSize.x && p.y <= halfSize.y - nodeHalfSize.y && p.z <= halfSize.z - nodeHalfSize.z) return SceneManager::INSIDE;
	else return SceneManager::OVERLAP;
}
void DefaultSceneManagerBoxTest::getChildren(NodeVirtual* node, std::vector<NodeVirtual::NodeRange>& path) const
{
	node->getChildrenInBox(path, bbMin, bbMax);
}


DefaultSceneManagerRayTest::DefaultSceneManagerRayTest(const glm::vec3& pos, const glm::vec3& dir, float maxDist) : position(pos), direction(dir), distance(maxDist)
{}
SceneManager::CollisionType DefaultSceneManagerRayTest::operator() (const NodeVirtual* node) const
{
	//	Segment/AABB collision test
	if (Collision::collide_SegmentvsAxisAlignedBox(position, position + distance*direction, node->getBBMin(), node->getBBMax()))
		return SceneManager::OVERLAP;
	else return SceneManager::NONE;
}


DefaultSceneManagerFrustrumTest::DefaultSceneManagerFrustrumTest(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& verticalDir, const glm::vec3& leftDir, float verticalAngle, float horizontalAngle)
	: camP(position) , camD(direction) , camV(verticalDir) , camL(leftDir) , camVa(verticalAngle) , camHa(horizontalAngle)
{}
SceneManager::CollisionType DefaultSceneManagerFrustrumTest::operator() (const NodeVirtual* node) const
{
	//	test if in front of camera
	const glm::vec3 p = node->getCenter() - camP;
	const glm::vec3 size = node->getSize();
	float forwardFloat = glm::dot(p, camD) + FRUSTRUM_COEFF * (abs(size.x * camD.x) + abs(size.y * camD.y) + abs(size.z * camD.z));
	if (forwardFloat < 0.f)
		return SceneManager::NONE;

	//	out of horizontal range
	float maxAbsoluteDimension = (std::max)(size.x, (std::max)(size.y, size.z)) / 2.f;
	float maxTangentDimension = abs(size.x * camL.x) / 2.f + abs(size.y * camL.y) / 2.f + abs(size.z * camL.z) / 2.f;
	if (abs(glm::dot(p, camL)) - maxTangentDimension > std::abs(forwardFloat) * tan(glm::radians(camHa)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
		return SceneManager::NONE;

	//	out of vertical range
	maxTangentDimension = abs(size.x * camV.x) / 2.f + abs(size.y * camV.y) / 2.f + abs(size.z * camV.z) / 2.f;
	if (abs(glm::dot(p, camV)) - maxTangentDimension > abs(forwardFloat) * tan(glm::radians(camVa)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
		return SceneManager::NONE;

	//	return distance to camera in int
	return SceneManager::OVERLAP;
}


DefaultRayPickingCollector::DefaultRayPickingCollector(const glm::vec3& pos, const glm::vec3& dir, float maxDist) : position(pos), direction(dir), distance(maxDist)
{}


void DefaultRayPickingCollector::operator() (NodeVirtual* node, Entity* object)
{
    DrawableComponent* drawableComp = object->getComponent<DrawableComponent>();
	if (!drawableComp || !drawableComp->isValid()) return;

    const SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
	bool animatable = drawableComp->hasSkeleton() && skeletonComp && skeletonComp->isValid();

    Mesh* mesh = drawableComp->getMesh();

	//	first pass test -> test ray vs object OBB or capsules
	if (animatable)
	{
		//	TODO collision on capsules
	}
	else
	{
		AxisAlignedBox aabb = mesh->getBoundingBox();
		OrientedBox box(glm::translate(object->getPosition()) * glm::toMat4(object->getOrientation()), aabb.min, aabb.max);
		if (!Collision::collide_SegmentvsOrientedBox( position, position + distance*direction, box.transform, box.min, box.max))
			return;
		/*OrientedBox box = object->getBoundingVolume();
		if (!Collision::collide_SegmentvsOrientedBox(position, position + distance*direction, box.transform, box.min, box.max))
			return;*/
	}

	//	second test -> test ray vs all object triangles
	const std::vector<glm::vec3>& vertices = *mesh->getVertices();
	const std::vector<unsigned short>& faces = *mesh->getFaces();
	glm::mat4 model = object->getMatrix();
	float collisionDistance = std::numeric_limits<float>::max();

	if (animatable)
	{
		const std::vector<glm::mat4> ibind = skeletonComp->getInverseBindPose();
		const std::vector<glm::mat4> pose = object->getComponent<SkeletonComponent>()->getPose();
		const std::vector<glm::ivec3>* bones = mesh->getBones();
		const std::vector<glm::vec3>* weights = mesh->getWeights();
		if (ibind.empty() || pose.empty() || !bones || !weights) return;

		for (unsigned int i = 0; i < faces.size(); i += 3)
		{
			//	compute pose bones contribution matrix
			glm::mat4 m1(0.f); glm::mat4 m2(0.f); glm::mat4 m3(0.f);
			for (int j = 0; j < 3; j++)
			{
				/*
					WARNNING : CRYPTIC PART INCOMING, READ EXPLANATION BEFORE CRYING
				
					faces[i + lambda] -> define index for vertex lambda of triangle
					(*bones)[alpha][j] -> define composante j (x, y, or z), of bone vector definition for vertex alpha
					pose[beta] -> define pose matrix for bone of index beta (idem for ibind)
					(*weights)[alpha][j] -> define composante j (x, y, or z), of weight vector definition for vertex alpha
				*/
				
				m1 += pose[(*bones)[faces[i    ]][j]] * ibind[(*bones)[faces[i    ]][j]] * (*weights)[(*bones)[faces[i    ]][j]][j];
				m2 += pose[(*bones)[faces[i + 1]][j]] * ibind[(*bones)[faces[i + 1]][j]] * (*weights)[(*bones)[faces[i + 1]][j]][j];
				m3 += pose[(*bones)[faces[i + 2]][j]] * ibind[(*bones)[faces[i + 2]][j]] * (*weights)[(*bones)[faces[i + 2]][j]][j];
			}
			
			//	compute triangle position
			glm::vec3 p1 = glm::vec3(model * m1 * glm::vec4(vertices[faces[i]], 1.f));
			glm::vec3 p2 = glm::vec3(model * m2 * glm::vec4(vertices[faces[i + 1]], 1.f));
			glm::vec3 p3 = glm::vec3(model * m3 * glm::vec4(vertices[faces[i + 2]], 1.f));

			//	collision detection
			if (Collision::collide_SegmentvsTriangle(position, position + distance*direction, p1, p2, p3))
			{
				glm::vec3 normal = glm::cross(p2 - p1, p3 - p1);
				glm::normalize(normal);
				collisionDistance = std::min(collisionDistance, glm::dot(normal, p1 - position) / glm::dot(normal, direction));
			}
		}
		if (collisionDistance == std::numeric_limits<float>::max()) return;
	}
	else
	{
		for (unsigned int i = 0; i < faces.size(); i += 3)
		{
			glm::vec3 p1 = glm::vec3(model * glm::vec4(vertices[faces[i]], 1.f));
			glm::vec3 p2 = glm::vec3(model * glm::vec4(vertices[faces[i + 1]], 1.f));
			glm::vec3 p3 = glm::vec3(model * glm::vec4(vertices[faces[i + 2]], 1.f));

			if (Collision::collide_SegmentvsTriangle(position, position + distance*direction, p1, p2, p3))
			{
				glm::vec3 normal = glm::cross(p2 - p1, p3 - p1);
				glm::normalize(normal);
				collisionDistance = std::min(collisionDistance, glm::dot(normal, p1 - position) / glm::dot(normal, direction));
			} 
		}
		if(collisionDistance == std::numeric_limits<float>::max()) return;
	}

	//	pass all collision test
	objectOnRay[collisionDistance] = object;
}
std::map<float, Entity*>& DefaultRayPickingCollector::getObjects() { return objectOnRay; }
Entity* DefaultRayPickingCollector::getNearestObject() const
{
	if (objectOnRay.empty()) return nullptr;
	else return objectOnRay.begin()->second;
}
float DefaultRayPickingCollector::getNearestDistance() const
{
	if (objectOnRay.empty()) return std::numeric_limits<float>::max();
	else return objectOnRay.begin()->first;
}
