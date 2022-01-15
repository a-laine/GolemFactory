#include "FrustrumSceneQuerry.h"
#include <Physics/Collision.h>

#include <iostream>

#define FRUSTRUM_COEFF	2.f

FrustrumSceneQuerry::FrustrumSceneQuerry(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& verticalDir, const glm::vec3& leftDir, float verticalAngle, float horizontalAngle) :
	cameraPosition(position), cameraDirection(direction), cameraVerticalAxis(verticalDir), cameraLeftAxis(leftDir), cameraVerticalAngle(verticalAngle), cameraHorizontalAngle(horizontalAngle)
{}

VirtualSceneQuerry::CollisionType FrustrumSceneQuerry::operator() (const NodeVirtual* node)
{
	//	test if in front of camera
	const glm::vec3 p = node->getCenter() - cameraPosition;
	const glm::vec3 size = node->getSize();
	float forwardFloat = glm::dot(p, cameraDirection) + FRUSTRUM_COEFF * (abs(size.x * cameraDirection.x) + abs(size.y * cameraDirection.y) + abs(size.z * cameraDirection.z));
	if (forwardFloat < 0.f)
		return VirtualSceneQuerry::NONE;

	//	out of horizontal range
	float maxAbsoluteDimension = 0.5f * std::max(size.x, std::max(size.y, size.z));
	float maxTangentDimension = 0.5f * (abs(size.x * cameraLeftAxis.x) + abs(size.y * cameraLeftAxis.y) + abs(size.z * cameraLeftAxis.z));
	if (abs(glm::dot(p, cameraLeftAxis)) - maxTangentDimension > forwardFloat * tan(glm::radians(cameraHorizontalAngle)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
		return VirtualSceneQuerry::NONE;

	//	out of vertical range
	maxTangentDimension = 0.5f * (abs(size.x * cameraVerticalAxis.x) + abs(size.y * cameraVerticalAxis.y) + abs(size.z * cameraVerticalAxis.z));
	if (abs(glm::dot(p, cameraVerticalAxis)) - maxTangentDimension > forwardFloat * tan(glm::radians(cameraVerticalAngle)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
		return VirtualSceneQuerry::NONE;

	result.push_back(node);
	return VirtualSceneQuerry::OVERLAP;

}
std::vector<const NodeVirtual*>& FrustrumSceneQuerry::getResult() { return result; }