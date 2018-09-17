#include "SceneQueryTests.h"


void DefaultRayPickingCollector::operator() (NodeVirtual* node, InstanceVirtual* object)
{
	//	first pass test
	if (object->getType() != InstanceVirtual::ANIMATABLE)
	{
		if (!Collision::collide_SegmentvsOrientedBox(position, position + distance*direction, object->getModelMatrix(), object->getBBMin(), object->getBBMax()))
			return;
	}
	else
	{
		return;
		//test on all model capsule;
		/*glm::mat4 model = object->getModelMatrix();
		if (!Collision::collide_SegmentvsCapsule(position, position + distance*direction, ))
			return;*/
	}

	//	second test
	/*Mesh* m = object->getMesh();
	if (!m) return;
	const std::vector<glm::vec3>& vertices = *m->getVertices();
	const std::vector<unsigned short>& faces = *m->getFaces();
	glm::mat4 model = object->getModelMatrix();
	float collisionDistance = std::numeric_limits<float>::max();

	if (object->getType() == InstanceVirtual::ANIMATABLE)
	{
		return;
	}
	else
	{
		unsigned int i = 0;
		for (i = 0; i < faces.size(); i += 3)
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
		if (i >= faces.size()) return;
	}*/

	//		Mesh* m = it->second->getMesh();
	//		if (!m) continue;
	//		const std::vector<glm::vec3>& vertices = *m->getVertices();
	//		const std::vector<unsigned short>& faces = *m->getFaces();
	//		const glm::mat4 model = it->second->getModelMatrix();
	//
	//		if (it->second->getType() == InstanceVirtual::ANIMATABLE)
	//		{
	//			const std::vector<glm::mat4> ibind = it->second->getSkeleton()->getInverseBindPose();
	//			const std::vector<glm::mat4> pose = it->second->getPose();
	//			const std::vector<glm::ivec3>* bones = m->getBones();
	//			const std::vector<glm::vec3>* weights = m->getWeights();
	//			if (ibind.empty() || pose.empty() || !bones || !weights) continue;
	//
	//			for (unsigned int i = 0; i < faces.size(); i += 3)
	//			{
	//				glm::mat4 m1(0.f); glm::mat4 m2(0.f); glm::mat4 m3(0.f);
	//				for (int j = 0; j < 3; j++)
	//				{
	//					/*
	//						WARNNING : CRYPTIC PART INCOMING, READ EXPLANATION BEFORE CRYING
	//
	//						faces[i + lambda] -> define index for vertex lambda of triangle
	//						(*bones)[alpha][j] -> define composante j (x, y, or z), of bone vector definition for vertex alpha
	//						pose[beta] -> define pose matrix for bone of index beta (idem for ibind)
	//						(*weights)[alpha][j] -> define composante j (x, y, or z), of weight vector definition for vertex alpha
	//					*/
	//
	//					m1 += pose[(*bones)[faces[i    ]][j]] * ibind[(*bones)[faces[i    ]][j]] * (*weights)[(*bones)[faces[i    ]][j]][j];
	//					m2 += pose[(*bones)[faces[i + 1]][j]] * ibind[(*bones)[faces[i + 1]][j]] * (*weights)[(*bones)[faces[i + 1]][j]][j];
	//					m3 += pose[(*bones)[faces[i + 2]][j]] * ibind[(*bones)[faces[i + 2]][j]] * (*weights)[(*bones)[faces[i + 2]][j]][j];
	//				}
	//
	//				glm::vec3 p1 = glm::vec3(model * m1 * glm::vec4(vertices[faces[i]], 1.f));			//	vertex 1 of triangle
	//				glm::vec3 v1 = glm::vec3(model * m2 * glm::vec4(vertices[faces[i + 1]], 1.f)) - p1;	//	vertex 2 of triangle - vertex 1 of triangle
	//				glm::vec3 v2 = glm::vec3(model * m3 * glm::vec4(vertices[faces[i + 2]], 1.f)) - p1;	//	vertex 3 of triangle - vertex 1 of triangle
	//				glm::vec3 normal = glm::cross(v1, v2);
	//				if (normal == glm::vec3(0.f)) continue;
	//				glm::normalize(normal);
	//
	//				//	compute intersection point
	//				float proj = glm::dot(normal, camDirection);
	//				if (proj == 0.f) continue;
	//				if (proj > 0.f) normal *= -1.f;
	//				float depth = glm::dot(normal, p1 - camPosition) / glm::dot(normal, camDirection);
	//				if (depth > maxDistance || depth < 0.f) continue;
	//				glm::vec3 intersection = camPosition + depth * camDirection - p1;
	//
	//				//	check if point is inside triangle
	//				float dotv1v2 = glm::dot(v1, v2);
	//				float magnitute = glm::dot(v2, v2) * glm::dot(v1, v1) - dotv1v2 * dotv1v2;
	//				glm::vec2 barry;
	//				barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - dotv1v2 * glm::dot(intersection, v2)) / magnitute;
	//				barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - dotv1v2 * glm::dot(intersection, v1)) / magnitute;
	//				if (barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f) continue;
	//				else refinedList.push_back(std::pair<float, InstanceVirtual*>(depth, it->second));
	//				break;
	//			}
	//		}
	//		else
	//		{
	//			for (unsigned int i = 0; i < faces.size(); i += 3)
	//			{
	//				//	compute local base
	//				glm::vec3 p1 = glm::vec3(model * glm::vec4(vertices[faces[i]], 1.f));				//	vertex 1 of triangle
	//				glm::vec3 v1 = glm::vec3(model * glm::vec4(vertices[faces[i + 1]], 1.f)) - p1;		//	vertex 2 of triangle - vertex 1 of triangle
	//				glm::vec3 v2 = glm::vec3(model * glm::vec4(vertices[faces[i + 2]], 1.f)) - p1;		//	vertex 3 of triangle - vertex 1 of triangle
	//				glm::vec3 normal = glm::cross(v1, v2);
	//				if (normal == glm::vec3(0.f)) continue;
	//				glm::normalize(normal);
	//
	//				//	compute intersection point
	//				float proj = glm::dot(normal, camDirection);
	//				if (proj == 0.f) continue;
	//				if (proj > 0.f) normal *= -1.f;
	//				float depth = glm::dot(normal, p1 - camPosition) / glm::dot(normal, camDirection);
	//				if (depth > maxDistance || depth < 0.f) continue;
	//				glm::vec3 intersection = camPosition + depth * camDirection - p1;
	//
	//				//	check if point is inside triangle
	//				float dotv1v2 = glm::dot(v1, v2);
	//				float magnitute = glm::dot(v2, v2) * glm::dot(v1, v1) - dotv1v2 * dotv1v2;
	//				glm::vec2 barry;
	//				barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - dotv1v2 * glm::dot(intersection, v2)) / magnitute;
	//				barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - dotv1v2 * glm::dot(intersection, v1)) / magnitute;
	//				if (barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f) continue;
	//				else refinedList.push_back(std::pair<float, InstanceVirtual*>(depth, it->second));
	//				break;
	//			}
	//		}

	//	pass all collision test
	objectOnRay[std::numeric_limits<float>::max()] = object;
}

std::map<float, InstanceVirtual*>& DefaultRayPickingCollector::getObjects()
{
	return objectOnRay;
}
InstanceVirtual* DefaultRayPickingCollector::getNearestObject() const
{
	if (objectOnRay.empty()) return nullptr;
	else return objectOnRay.begin()->second;
}
float DefaultRayPickingCollector::getNearestDistance() const
{
	if (objectOnRay.empty()) return std::numeric_limits<float>::max();
	else return objectOnRay.begin()->first;
}



