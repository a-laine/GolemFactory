#include "Camera.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/constants.hpp>

#include "../Events/EventHandler.h"


//  Default
Camera::Camera() :
	configuration(FREEFLY), position(0.f, -10.f, 5.f), orientation(1.f, 0.f, 0.f, 0.f),
	sensitivity(0.2f), speedMag(0.003f), radius(3.f), radiusMin(1.f), radiusMax(10.f), fov(90)
{
	setDirection_internal(glm::vec3(0, 0, 1) - position);
}
Camera::~Camera() {}
Camera& Camera::operator=(Camera& c)
{
	mutex.lock();

	configuration = c.configuration;
	position = c.position;
	orientation = c.orientation;

	radius = c.radius;
	radiusMin = c.radiusMin;
	radiusMax = c.radiusMax;

	sensitivity = c.sensitivity;
	speedMag = c.speedMag;
	fov = c.fov;

	mutex.unlock();
	return *this;
}
//

//  Public functions
void Camera::animate(float elapseTime, bool goForw, bool goBack, bool goLeft, bool goRight, bool option1, bool option2)
{
	//	begin
	mutex.lock();
	float tmpSpeed = speedMag;
	glm::vec3 direction(0., 0., 0.);
	glm::vec3 forward, right;

	//	move camera
	switch (configuration&MODE_MASK)
	{
	case FREEFLY:
		if (!EventHandler::getInstance()->getCursorMode())
		{
			float yaw = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x);
			float pitch = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y);
			rotate_internal(pitch, yaw);
		}
		forward = getForward_internal();
		right = getRight_internal();

		if (goForw) direction += forward;
		if (goBack) direction -= forward;
		if (goLeft) direction -= right;
		if (goRight) direction += right;

		if (option2) tmpSpeed /= 10.f;
		if (option1) tmpSpeed *= 10.f;

		if (direction.x || direction.y || direction.z)
			position += glm::normalize(direction)*elapseTime*tmpSpeed;
		break;

	case TRACKBALL:
		if (!EventHandler::getInstance()->getCursorMode())
		{
			float yaw = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x);
			float pitch = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y);
			forward = getForward_internal();
			rotateAround_internal(position + radius * forward, pitch, yaw);
		}
		break;

	case ISOMETRIC:
		if (option2)
		{
			float yaw = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x);
			float pitch = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y);
			rotate_internal(pitch, yaw);
		}
		forward = getForward_internal();
		right = getRight_internal();

		if (option2) tmpSpeed /= 10.f;
		if (option1) tmpSpeed *= 10.f;

		if (goForw) direction += glm::normalize(glm::dvec3(forward.x, forward.y, 0));
		if (goBack) direction -= glm::normalize(glm::dvec3(forward.x, forward.y, 0));
		if (goLeft) direction -= right;
		if (goRight) direction += right;

		if (direction.x || direction.y || direction.z)
			position += glm::normalize(direction)*elapseTime*tmpSpeed;
		break;

	default: break;
	}

	//	end
	mutex.unlock();
}
glm::mat4 Camera::getViewMatrix()
{
	glm::mat4 m;
	mutex.lock();
	glm::quat rot = glm::inverse(orientation);
	m = glm::toMat4(rot);
	m[3] = glm::rotate(rot, glm::vec4(-position, 1.f));
	mutex.unlock();
	return m;
}
glm::mat4 Camera::getModelMatrix()
{
	glm::mat4 m;
	mutex.lock();
	m = glm::toMat4(orientation);
	m[3] = glm::vec4(position, 1.f);
	mutex.unlock();
	return m;
}
glm::mat4 Camera::getOrientationMatrix()
{
	mutex.lock();
	glm::quat q = orientation;
	mutex.unlock();
	return glm::toMat4(q);
}
void Camera::translate(const glm::vec3& v)
{
	glm::vec3 p = v + getPosition();
	setPosition(p);
}
//

//  Set/get functions
void Camera::setMode(CameraMode mode)
{
	mutex.lock();
	configuration &= ~MODE_MASK;
	configuration |= mode;
	mutex.unlock();
}
void Camera::setSpeed(float s)
{
	mutex.lock();
	speedMag = s;
	mutex.unlock();
}
void Camera::setRadiusMin(float r)
{
	mutex.lock();
	radiusMin = r;
	boundingRadius();
	mutex.unlock();
}
void Camera::setRadiusMax(float r)
{
	mutex.lock();
	radiusMax = r;
	boundingRadius();
	mutex.unlock();
}
void Camera::setRadius(float r)
{
	mutex.lock();
	radius = r;
	boundingRadius();
	mutex.unlock();
}
void Camera::setAllRadius(float r, float rmin, float rmax)
{
	mutex.lock();
	radius = r;
	radiusMax = rmax;
	radiusMin = rmin;
	boundingRadius();
	mutex.unlock();
}
void Camera::setPosition(const glm::vec3& pos)
{
	mutex.lock();
	if ((configuration & MODE_MASK) == TRACKBALL)
	{
		glm::vec3 target = position + radius * getForward_internal();
		position = pos;
		glm::vec3 forward = target - position;

		radius = (float)forward.length();
		boundingRadius();

		forward = glm::normalize(forward);
		setDirection_internal(forward);
		position = target - radius * forward;

	}
	else position = pos;
	mutex.unlock();
}

void Camera::setOrientation(const glm::vec3& dirVector)
{
	mutex.lock();
	setDirection_internal(glm::normalize(dirVector));
	mutex.unlock();
}
void Camera::setTarget(const glm::vec3& target)
{
	mutex.lock();
	boundingRadius();
	position = target - radius * getForward_internal();
	mutex.unlock();
}
void Camera::setFieldOfView(float angle)
{
	mutex.lock();
	fov = angle;
	mutex.unlock();
}
void Camera::setSensitivity(float sens)
{
	mutex.lock();
	sensitivity = sens;
	mutex.unlock();
}


Camera::CameraMode Camera::getMode()
{
	CameraMode m;
	mutex.lock();
	m = (CameraMode)(configuration&MODE_MASK);
	mutex.unlock();
	return m;
}
glm::vec3 Camera::getTarget()
{
	glm::vec3 v;
	mutex.lock();
	float r = ((configuration&MODE_MASK) == TRACKBALL) ? radius : 1.f;
	v = position + r * getForward_internal();
	mutex.unlock();
	return v;
}
glm::vec3 Camera::getRight()
{
	glm::vec3 r;
	mutex.lock();
	r = getRight_internal();
	mutex.unlock();
	return r;
}
glm::vec3 Camera::getForward()
{
	glm::vec3 f;
	mutex.lock();
	f = getForward_internal();
	mutex.unlock();
	return f;
}
glm::vec3 Camera::getVertical()
{
	glm::vec3 v;
	mutex.lock();
	v = getVertical_internal();
	mutex.unlock();
	return v;
}
glm::vec3 Camera::getPosition()
{
	glm::vec3 p;
	mutex.lock();
	p = position;
	mutex.unlock();
	return p;
}
void Camera::getFrustrum(glm::vec3& pos, glm::vec3& forward, glm::vec3& up, glm::vec3& right)
{
	mutex.lock();
	pos = position;
	forward = getForward_internal();
	up = getVertical_internal();
	right = getRight_internal();
	mutex.unlock();
}
float Camera::getSpeed()
{
	float s;
	mutex.lock();
	s = speedMag;
	mutex.unlock();
	return s;
}
float Camera::getRadiusMin()
{
	float r;
	mutex.lock();
	r = radiusMin;
	mutex.unlock();
	return r;
}
float Camera::getRadiusMax()
{
	float r;
	mutex.lock();
	r = radiusMax;
	mutex.unlock();
	return r;
}
float Camera::getRadius()
{
	float r;
	mutex.lock();
	r = radius;
	mutex.unlock();
	return r;
}
float Camera::getVerticalFieldOfView(float aspectRatio)
{
	float a;
	mutex.lock();
	a = fov / aspectRatio;
	mutex.unlock();
	return a;
}
float Camera::getFieldOfView()
{
	float a;
	mutex.lock();
	a = fov;
	mutex.unlock();
	return a;
}
float Camera::getSensitivity()
{
	float s;
	mutex.lock();
	s = sensitivity;
	mutex.unlock();
	return s;
}
//

//  Private functions
void Camera::rotate_internal(float p, float y)
{
	glm::vec3 euler = glm::eulerAngles(orientation) + glm::vec3(p, 0.f, y);

	float _Pi = glm::pi<float>();
	float twoPi = 2.f*glm::pi<float>();

	if (euler.x > _Pi - 0.01f) euler.x = _Pi - 0.01f;
	else if (euler.x < 0.01f) euler.x = 0.01f;
	if (euler.z > _Pi) euler.z -= twoPi;
	else if (euler.z < -_Pi) euler.z += twoPi;

	orientation = glm::quat(euler);
}
void Camera::rotateAround_internal(const glm::vec3& target, float p, float y)
{
	rotate_internal(p, y);

	float r = glm::distance(position, target);
	position = target - r * getForward_internal();
}
void Camera::setDirection_internal(const glm::vec3& direction)
{
	glm::vec3 euler = glm::eulerAngles(orientation);

	if (!(direction.x == 0 && direction.y == 0 && direction.z == 0))
		euler.x = atan2(sqrt(direction.x*direction.x + direction.y*direction.y), -direction.z);

	if (direction.x != 0 || direction.y != 0)
		euler.z = -atan2(direction.x, direction.y);

	orientation = glm::quat(euler);
}
glm::vec3 Camera::getForward_internal() const
{
	return glm::rotate(orientation, glm::vec3(0, 0, -1));
}
glm::vec3 Camera::getRight_internal() const
{
	return glm::rotate(orientation, glm::vec3(1, 0, 0));
}
glm::vec3 Camera::getVertical_internal() const
{
	return glm::rotate(orientation, glm::vec3(0, 1, 0));
}
void Camera::boundingRadius()
{
	radius = std::max(std::min(radius, radiusMax), radiusMin);
}
//
