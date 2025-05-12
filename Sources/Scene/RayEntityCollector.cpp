#include "RayEntityCollector.h"
#include <Physics/Collision.h>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Resources/Mesh.h>

//#include <glm/gtx/component_wise.hpp>
#include <Physics/Shapes/Collider.h>


RayEntityCollector::RayEntityCollector(const vec4f& pos, const vec4f& dir, float maxDist) : position(pos), direction(dir), distance(maxDist)
{
	position.w = 1.f;
	direction.w = 0.f;
}
bool RayEntityCollector::operator() (Entity* entity)
{
	DrawableComponent* drawableComp = entity->getComponent<DrawableComponent>();
	if (!drawableComp || !drawableComp->isValid() || !drawableComp->visible()) 
		return false;

	const SkeletonComponent* skeletonComp = entity->getComponent<SkeletonComponent>();
	bool animatable = drawableComp->hasSkeleton() && skeletonComp && skeletonComp->isValid();
	Mesh* mesh = drawableComp->getMesh();

	//	first pass test -> test ray vs object OBB or capsules
	if (animatable)
	{
		mat4f model = entity->getWorldTransformMatrix();
		vec4f scale(entity->getWorldScale());
		float smax = std::max(scale.x, std::max(scale.y, scale.z));
		const std::vector<mat4f> pose = entity->getComponent<SkeletonComponent>()->getPose();
		/*const std::vector<vec2i>& segments = entity->getComponent<SkeletonComponent>()->getSegmentsIndex();
		const std::vector<float>& radius = entity->getComponent<SkeletonComponent>()->getSegmentsRadius();

		bool collision = false;
		for (unsigned int i = 0; i < segments.size(); i++)
		{
			vec4f a = model * pose[(const int)segments[i].x][3];
			vec4f b = model * pose[(const int)segments[i].y][3];
			if (Collision::collide_SegmentvsCapsule(position, position + distance * direction, a, b, smax * radius[i]))
			{
				collision = true;
				break;
			}
		}
		if (!collision) */return false;
	}
	else
	{
		bool collision = false;
		Segment ray(position, position + distance * direction);
		AxisAlignedBox box = entity->getBoundingBox();
		if (!Collision::collide(&ray, &box))
			return false;
	}

	//	second test -> test ray vs all object triangles
	const std::vector<vec4f>& vertices = *mesh->getVertices();
	unsigned int indiceCount = mesh->getNumberIndices();
	mat4f model = entity->getWorldTransformMatrix();
	float collisionDistance = std::numeric_limits<float>::max();

	if (animatable)
	{
		const std::vector<mat4f> ibind = skeletonComp->getInverseBindPose();
		const std::vector<mat4f> pose = entity->getComponent<SkeletonComponent>()->getPose();
		const std::vector<vec4i>* bones = mesh->getBones();
		const std::vector<vec4f>* weights = mesh->getWeights();
		if (ibind.empty() || pose.empty() || !bones || !weights) return false;

		for (unsigned int i = 0; i < indiceCount; i += 3)
		{
			// indices
			unsigned int i0 = mesh->getFaceIndiceAt(i);
			unsigned int i1 = mesh->getFaceIndiceAt(i + 1);
			unsigned int i2 = mesh->getFaceIndiceAt(i + 2);

			//	compute pose bones contribution matrix
			mat4f m1(0.f);
			mat4f m2(0.f);
			mat4f m3(0.f);
			for (int j = 0; j < 3; j++)
			{
				/*
				WARNNING : CRYPTIC PART INCOMING, READ EXPLANATION BEFORE CRYING

				faces[i + lambda] -> define index for vertex lambda of triangle
				(*bones)[alpha][j] -> define composante j (x, y, or z), of bone vector definition for vertex alpha
				pose[beta] -> define pose matrix for bone of index beta (idem for ibind)
				(*weights)[alpha][j] -> define composante j (x, y, or z), of weight vector definition for vertex alpha
				*/

				m1 += pose[(*bones)[i0][j]] * ibind[(*bones)[i0][j]] * (*weights)[(*bones)[i0][j]][j];
				m2 += pose[(*bones)[i1][j]] * ibind[(*bones)[i1][j]] * (*weights)[(*bones)[i1][j]][j];
				m3 += pose[(*bones)[i2][j]] * ibind[(*bones)[i2][j]] * (*weights)[(*bones)[i2][j]][j];
			}

			//	compute triangle position
			vec4f p1 = model * m1 * vertices[i0];
			vec4f p2 = model * m2 * vertices[i1];
			vec4f p3 = model * m3 * vertices[i2];

			//	collision detection
			if (Collision::collide_SegmentvsTriangle(position, position + distance*direction, p1, p2, p3))
			{
				vec4f normal = vec4f::cross(p2 - p1, p3 - p1);
				normal.normalize();
				collisionDistance = std::min(collisionDistance, vec4f::dot(normal, p1 - position) / vec4f::dot(normal, direction));
			}
		}
		if (collisionDistance == std::numeric_limits<float>::max()) return false;
	}
	else
	{
		CollisionReport report;
		vec4f rayEnd = position + distance * direction;
		for (unsigned int i = 0; i < indiceCount; i += 3)
		{
			// indices
			unsigned int i0 = mesh->getFaceIndiceAt(i);
			unsigned int i1 = mesh->getFaceIndiceAt(i + 1);
			unsigned int i2 = mesh->getFaceIndiceAt(i + 2);

			vec4f p1 = model * vertices[i0];
			vec4f p2 = model * vertices[i1];
			vec4f p3 = model * vertices[i2];

			if (Collision::collide_SegmentvsTriangle(position, rayEnd, p1, p2, p3, &report))
			{
				collisionDistance = std::min(collisionDistance, (report.points[0] - position).getNorm());
				//vec4f normal = vec4f::cross(p2 - p1, p3 - p1);
				//normal.normalize();
				//collisionDistance = std::min(collisionDistance, vec4f::dot(normal, p1 - position) / vec4f::dot(normal, direction));
				report.clear();
			}
		}
		if (collisionDistance == std::numeric_limits<float>::max()) return false;
	}

	//	pass all collision test
	sortedIndicies.push_back(std::pair<float, unsigned int>(collisionDistance, (unsigned int) result.size()));
	result.push_back(entity);
	return true;
}
std::vector<std::pair<float, unsigned int> >& RayEntityCollector::getSortedResult()
{
	return sortedIndicies;
}
