#include "BoundingVolume.hpp"
#include "Utiles\Assert.hpp"



bool BoundingBox::intersect(const glm::vec4& point) const
{
	return false; // TODO
}

bool BoundingBox::intersect(const glm::vec4& min, const glm::vec4& max) const
{
	return false; // TODO
}

bool BoundingBox::intersect(const glm::vec4& center, float radius) const
{
	return false; // TODO
}



bool BoundingSphere::intersect(const glm::vec4& point) const
{
	return false; // TODO
}

bool BoundingSphere::intersect(const glm::vec4& min, const glm::vec4& max) const
{
	return false; // TODO
}

bool BoundingSphere::intersect(const glm::vec4& center, float radius) const
{
	return false; // TODO
}



BoundingVolume::BoundingVolume()
	: m_type(Sphere)
	, m_transform(nullptr)
	, m_sphere()
{
}

BoundingVolume::BoundingVolume(const glm::vec4& min, const glm::vec4& max)
	: m_type(AABox)
	, m_transform(nullptr)
	, m_box(min, max)
{}

BoundingVolume::BoundingVolume(const glm::vec4& min, const glm::vec4& max, const glm::mat4& transform)
	: m_type(OOBox)
	, m_transform(&transform)
	, m_box(min, max)
{}

BoundingVolume::BoundingVolume(const glm::vec4& center, float radius)
	: m_type(Sphere)
	, m_transform(nullptr)
	, m_sphere(center, radius)
{}

void BoundingVolume::setAABox(const glm::vec4& min, const glm::vec4& max)
{
	m_type = AABox;
	m_transform = nullptr;
	m_box.setMin(min);
	m_box.setMax(max);
}

void BoundingVolume::setOOBox(const glm::vec4& min, const glm::vec4& max, const glm::mat4& transform)
{
	m_type = OOBox;
	m_transform = &transform;
	m_box.setMin(min);
	m_box.setMax(max);
}

void BoundingVolume::setSphere(const glm::vec4& center, float radius)
{
	m_type = Sphere;
	m_transform = nullptr;
	m_sphere.setCenter(center);
	m_sphere.setRadius(radius);
}

glm::vec4 BoundingVolume::getLocalMin() const
{
	GF_ASSERT(m_type != Sphere);
	return m_box.getMin();
}

glm::vec4 BoundingVolume::getLocalMax() const
{
	GF_ASSERT(m_type != Sphere);
	return m_box.getMax();
}

glm::vec4 BoundingVolume::getGlobalMin() const
{
	return glm::vec4(); // TODO
}

glm::vec4 BoundingVolume::getGlobalMax() const
{
	return glm::vec4(); // TODO
}

glm::vec4 BoundingVolume::getSphereCenter() const
{
	GF_ASSERT(m_type == Sphere);
	return m_sphere.getCenter();
}

const glm::mat4* BoundingVolume::getTransformMatrix() const
{
	return m_transform;
}

glm::vec4 BoundingVolume::getSize() const
{
	switch (m_type)
	{
	case BoundingVolume::Sphere:
		return m_sphere.getSize();
	case BoundingVolume::OOBox:
		return (*m_transform) * m_box.getSize();
	case BoundingVolume::AABox:
		return m_box.getSize();
	default:
		GF_ASSERT(0);
		return glm::vec4();
	}
}

glm::vec4 BoundingVolume::getCenter() const
{
	switch (m_type)
	{
	case BoundingVolume::Sphere:
		return m_sphere.getCenter();
	case BoundingVolume::OOBox:
		return glm::inverse(*m_transform) * m_box.getCenter();
	case BoundingVolume::AABox:
		return m_box.getCenter();
	default:
		GF_ASSERT(0);
		return glm::vec4();
	}
}

bool BoundingVolume::intersect(const BoundingVolume& other) const
{
	switch (m_type)
	{
	case BoundingVolume::Sphere:
		return other.intersect(m_sphere);
	case BoundingVolume::OOBox:
		return other.intersect(m_box, *m_transform);
	case BoundingVolume::AABox:
		return other.intersect(m_box);
	default:
		GF_ASSERT(0);
		return false;
	}
}

bool BoundingVolume::intersect(const BoundingBox& bbox) const
{
	return false; // TODO
}

bool BoundingVolume::intersect(const BoundingBox& bbox, const glm::mat4& m_transform) const
{
	return false; // TODO
}

bool BoundingVolume::intersect(const BoundingSphere& sphere) const
{
	return false; // TODO
}

bool BoundingVolume::intersect(const glm::vec4& point) const
{
	return false; // TODO
}

bool BoundingVolume::intersect(const glm::vec4& min, const glm::vec4& max) const
{
	return false; // TODO
}

bool BoundingVolume::intersect(const glm::vec4& center, float radius) const
{
	return false; // TODO
}

