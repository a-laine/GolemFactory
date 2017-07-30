#include "Camera.h"


//  Default
Camera::Camera(const float& screenRatio) : 
	configuration(FREEFLY), position(0.f, -10.f, 5.f), teta(0.f), phi(0.f), sensivity(0.2f), speedMag(0.003f),
	radius(3.f), radiusMin(1.f), radiusMax(10.f), frustrumAngleVertical(45.f)
{
    forward  = glm::vec3(0,0,1)- position;
	frustrumAngleHorizontal = screenRatio*frustrumAngleVertical;
	
    anglesFromVectors();
    vectorsFromAngles();
}
Camera::~Camera() {}
//

//  Public functions
void Camera::animate(float elapseTime, bool goForw, bool goBack, bool goLeft, bool goRight, bool option1, bool option2)
{
	//	begin
    mutex.lock();
    float tmpSpeed = speedMag;
	glm::vec3 direction(0.,0.,0.);
	float tmpSensitivity = sensivity*frustrumAngleVertical / 45.f;

	//	move camera
    switch(configuration&MODE_MASK)
    {
        case FREEFLY:
            if(!EventHandler::getInstance()->getCursorMode())
            {
                phi  -= tmpSensitivity*EventHandler::getInstance()->getCursorPositionRelative().x;
                teta -= tmpSensitivity*EventHandler::getInstance()->getCursorPositionRelative().y;
                vectorsFromAngles();
            }
			if (goForw) direction += forward;
			if (goBack) direction -= forward;
			if (goLeft) direction += left;
            if (goRight) direction -= left;

			if (option2) tmpSpeed /= 10.f;
			if (option1) tmpSpeed *= 10.f;

			if (direction.x || direction.y || direction.z)
				position += glm::normalize(direction)*elapseTime*tmpSpeed;
            break;

        case TRACKBALL:
            if(!EventHandler::getInstance()->getCursorMode())
            {
                phi  -= tmpSensitivity*EventHandler::getInstance()->getCursorPositionRelative().x;
                teta -= tmpSensitivity*EventHandler::getInstance()->getCursorPositionRelative().y;
                vectorsFromAngles(position + radius * forward);
            }
            break;

        case ISOMETRIC:
            if(option2)
            {
                phi  -= tmpSensitivity*EventHandler::getInstance()->getCursorPositionRelative().x;
                teta -= tmpSensitivity*EventHandler::getInstance()->getCursorPositionRelative().y;
                vectorsFromAngles();
            }

			if (option2) tmpSpeed /= 10.f;
			if (option1) tmpSpeed *= 10.f;

			if (goForw) direction += glm::normalize(glm::dvec3(forward.x, forward.y, 0));
			if (goBack) direction -= glm::normalize(glm::dvec3(forward.x, forward.y, 0));
			if (goLeft) direction += left;
			if (goRight) direction -= left;

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
	m = glm::lookAt(position, position + forward, glm::vec3(0, 0, 1));
    mutex.unlock();
	return m;
}
void Camera::translate(glm::vec3 v)
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
void Camera::setPosition(glm::vec3 pos)
{
    mutex.lock();
	if ((configuration & MODE_MASK) == TRACKBALL)
    {
		glm::vec3 target = position + radius*forward;
        position += pos;
        forward = target - position;
        radius = (float)forward.length();
		forward = glm::normalize(forward);

        boundingRadius();
        anglesFromVectors();
        vectorsFromAngles(target);
    }
    else position = pos;
    mutex.unlock();
}
void Camera::setOrientation(glm::vec3 orientation)
{
    mutex.lock();
    forward = glm::normalize(orientation);
    anglesFromVectors();
    vectorsFromAngles();
    mutex.unlock();
}
void Camera::setTarget(glm::vec3 target)
{
    mutex.lock();
    boundingRadius();
    vectorsFromAngles(target);
    mutex.unlock();
}
void Camera::setFrustrumAngleVertical(float angle)
{
    mutex.lock();
    frustrumAngleVertical = angle;
    mutex.unlock();
}
void Camera::setFrustrumAngleHorizontal(float angle)
{
    mutex.lock();
    frustrumAngleHorizontal = angle;
    mutex.unlock();
}
void Camera::setFrustrumAngleHorizontalFromScreenRatio(float screenratio)
{
	mutex.lock();
	frustrumAngleHorizontal = screenratio*frustrumAngleVertical;
	mutex.unlock();
}
void Camera::setSensitivity(float sens)
{
    mutex.lock();
    sensivity = sens;
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
    if((configuration&MODE_MASK) == TRACKBALL) v = position + radius*forward;
    else v = position + forward;
    mutex.unlock();
    return v;
}
glm::vec3 Camera::getLeft()
{
	glm::vec3 l;
    mutex.lock();
    l = left;
    mutex.unlock();
    return l;
}
glm::vec3 Camera::getForward()
{
	glm::vec3 f;
    mutex.lock();
    f = forward;
    mutex.unlock();
    return f;
}
glm::vec3 Camera::getVertical()
{
	glm::vec3 v;
    mutex.lock();
    v = vertical;
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
float Camera::getFrustrumAngleVertical()
{
    float a;
    mutex.lock();
    a = frustrumAngleVertical;
    mutex.unlock();
    return a;
}
float Camera::getFrustrumAngleHorizontal()
{
    float a;
    mutex.lock();
    a = frustrumAngleHorizontal;
    mutex.unlock();
    return a;
}
float Camera::getSensitivity()
{
    float s;
    mutex.lock();
    s = sensivity;
    mutex.unlock();
    return s;
}
//

//  Private functions
void Camera::vectorsFromAngles(glm::vec3 target)
{
	//	initialization pass
    if(phi>180.f) phi -= 360.f;
    else if(phi < -180.f) phi += 360.f;
    if(teta>89.f) teta = 89.f;
    else if(teta < -89.f) teta = -89.f;

	float radTeta = glm::radians(teta);
	float radPhi = glm::radians(phi);

	//	compute vector
    switch(configuration&MODE_MASK)
    {
        case FREEFLY: case ISOMETRIC:
			forward.x = cos(radTeta)*cos(radPhi);
			forward.y = cos(radTeta)*sin(radPhi);
			forward.z = sin(radTeta);
            break;

        case TRACKBALL:
			forward.x = cos(radTeta)*cos(radPhi);
			forward.y = cos(radTeta)*sin(radPhi);
			forward.z = sin(radTeta);
            position = target - radius*forward;
            break;

        default: break;
    }

	left = glm::normalize(glm::cross(glm::vec3(0, 0, 1), forward));
	vertical = glm::normalize(glm::cross(forward, left));
}
void Camera::anglesFromVectors()
{
	teta = (float) (atan2(forward.z, sqrt(forward.x*forward.x + forward.y*forward.y))*180. / glm::pi<double>());
	phi = (float) (atan2(forward.y, forward.x)*180. / glm::pi<double>());
}
void Camera::boundingRadius()
{
	radius = std::max(std::min(radius, radiusMax), radiusMin);
}
//
