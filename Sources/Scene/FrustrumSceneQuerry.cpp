#include "FrustrumSceneQuerry.h"
#include "Physics/Collision.h"

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
	float maxAbsoluteDimension = (std::max)(size.x, (std::max)(size.y, size.z)) / 2.f;
	float maxTangentDimension = abs(size.x * cameraLeftAxis.x) / 2.f + abs(size.y * cameraLeftAxis.y) / 2.f + abs(size.z * cameraLeftAxis.z) / 2.f;
	if (abs(glm::dot(p, cameraLeftAxis)) - maxTangentDimension > std::abs(forwardFloat) * tan(glm::radians(cameraHorizontalAngle)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
		return VirtualSceneQuerry::NONE;

	//	out of vertical range
	maxTangentDimension = abs(size.x * cameraVerticalAxis.x) / 2.f + abs(size.y * cameraVerticalAxis.y) / 2.f + abs(size.z * cameraVerticalAxis.z) / 2.f;
	if (abs(glm::dot(p, cameraVerticalAxis)) - maxTangentDimension > abs(forwardFloat) * tan(glm::radians(cameraVerticalAngle)) + FRUSTRUM_COEFF * maxAbsoluteDimension)
		return VirtualSceneQuerry::NONE;

	result.push_back(node);
	return VirtualSceneQuerry::OVERLAP;

}
std::vector<const NodeVirtual*>& FrustrumSceneQuerry::getResult() { return result; }