#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class BoundingBox
{
public:
	BoundingBox() : m_min(), m_max() {}
	BoundingBox(const glm::vec4& min, const glm::vec4& max) : m_min(min), m_max(max) {}

	void setMin(const glm::vec4& min) { m_min = min; }
	void setMax(const glm::vec4& max) { m_max = max; }
	glm::vec4 getMin() const { return m_min; }
	glm::vec4 getMax() const { return m_max; }
	glm::vec4 getSize() const { return m_max - m_min; }
	glm::vec4 getCenter() const { return (m_min + m_max) * 0.5f; }

	bool intersect(const glm::vec4& point) const;
	bool intersect(const glm::vec4& min, const glm::vec4& max) const;
	bool intersect(const glm::vec4& center, float radius) const;

private:
	glm::vec4 m_min;
	glm::vec4 m_max;
};

class BoundingSphere
{
public:
	BoundingSphere() : m_center(), m_radius(0) {}
	BoundingSphere(const glm::vec4& center, float radius) : m_center(center), m_radius(radius) {}

	void setCenter(const glm::vec4& center) { m_center = center; }
	void setRadius(float radius) { m_radius = radius; }
	glm::vec4 getCenter() const { return m_center; }
	float getRadius() const { return m_radius; }
	glm::vec4 getSize() const { return glm::vec4(m_radius, m_radius, m_radius, 0.f); }

	bool intersect(const glm::vec4& point) const;
	bool intersect(const glm::vec4& min, const glm::vec4& max) const;
	bool intersect(const glm::vec4& center, float radius) const;

private:
	glm::vec4 m_center;
	float m_radius;
};

class BoundingVolume
{
public:
	enum Type
	{
		Sphere,
		OOBox,
		AABox
	};
public:
	BoundingVolume();
	BoundingVolume(const glm::vec4& min, const glm::vec4& max);
	BoundingVolume(const glm::vec4& min, const glm::vec4& max, const glm::mat4& transform);
	BoundingVolume(const glm::vec4& center, float radius);

	Type getType() { return m_type; }

	void setAABox(const glm::vec4& min, const glm::vec4& max);
	void setOOBox(const glm::vec4& min, const glm::vec4& max, const glm::mat4& transform);
	void setSphere(const glm::vec4& center, float radius);
	glm::vec4 getLocalMin() const;
	glm::vec4 getLocalMax() const;
	glm::vec4 getGlobalMin() const;
	glm::vec4 getGlobalMax() const;
	glm::vec4 getSphereCenter() const;
	const glm::mat4* getTransformMatrix() const;

	glm::vec4 getSize() const;
	glm::vec4 getCenter() const;

	bool intersect(const BoundingVolume& other) const;
	bool intersect(const BoundingBox& bbox) const;
	bool intersect(const BoundingBox& bbox, const glm::mat4& m_transform) const;
	bool intersect(const BoundingSphere& sphere) const;
	bool intersect(const glm::vec4& point) const;
	bool intersect(const glm::vec4& min, const glm::vec4& max) const;
	bool intersect(const glm::vec4& center, float radius) const;

private:
	union {
		BoundingBox m_box;
		BoundingSphere m_sphere;
	};
	const glm::mat4* m_transform;
	Type m_type;
};