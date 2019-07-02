#include "RayEntityCollector.h"
#include "Physics/Collision.h"
#include "Renderer/DrawableComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Resources/Mesh.h"

#include <glm/gtx/component_wise.hpp>


RayEntityCollector::RayEntityCollector(const glm::vec3& pos, const glm::vec3& dir, float maxDist) : position(pos), direction(dir), distance(maxDist)
{}
bool RayEntityCollector::operator() (Entity* entity)
{
	DrawableComponent* drawableComp = entity->getComponent<DrawableComponent>();
	if (!drawableComp || !drawableComp->isValid()) return false;

	const SkeletonComponent* skeletonComp = entity->getComponent<SkeletonComponent>();
	bool animatable = drawableComp->hasSkeleton() && skeletonComp && skeletonComp->isValid();
	Mesh* mesh = drawableComp->getMesh();

	//	first pass test -> test ray vs object OBB or capsules
	if (animatable)
	{
		glm::mat4 model = entity->getMatrix();
		float scale = glm::compMax(entity->getScale());
		const std::vector<glm::mat4> pose = entity->getComponent<SkeletonComponent>()->getPose();
		const std::vector<glm::ivec2>& segments = entity->getComponent<SkeletonComponent>()->getSegmentsIndex();
		const std::vector<float>& radius = entity->getComponent<SkeletonComponent>()->getSegmentsRadius();

		bool collision = false;
		for (unsigned int i = 0; i < segments.size(); i++)
		{
			glm::vec3 a(model * pose[(const int)segments[i].x][3]);
			glm::vec3 b(model * pose[(const int)segments[i].y][3]);
			if (Collision::collide_SegmentvsCapsule(position, position + distance * direction, a, b, scale*radius[i]))
			{
				collision = true;
				break;
			}
		}
		if (!collision) return false;
	}
	else
	{
		if (!Collision::collide(Segment(position, position + distance * direction), *entity->getGlobalBoundingShape()))
			return false;
	}

	//	second test -> test ray vs all object triangles
	const std::vector<glm::vec3>& vertices = *mesh->getVertices();
	const std::vector<unsigned short>& faces = *mesh->getFaces();
	glm::mat4 model = entity->getMatrix();
	float collisionDistance = std::numeric_limits<float>::max();

	if (animatable)
	{
		const std::vector<glm::mat4> ibind = skeletonComp->getInverseBindPose();
		const std::vector<glm::mat4> pose = entity->getComponent<SkeletonComponent>()->getPose();
		const std::vector<glm::ivec3>* bones = mesh->getBones();
		const std::vector<glm::vec3>* weights = mesh->getWeights();
		if (ibind.empty() || pose.empty() || !bones || !weights) return false;

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

				m1 += pose[(*bones)[faces[i]][j]] * ibind[(*bones)[faces[i]][j]] * (*weights)[(*bones)[faces[i]][j]][j];
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
		if (collisionDistance == std::numeric_limits<float>::max()) return false;
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
