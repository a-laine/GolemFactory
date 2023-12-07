#include "CollisionUtils.h"

#define quickClamp(x, y) (((x) <= 0.f) ? 0.f : ((x) >= (y) ? 1.f : ((x) / (y))))


/*vec4f CollisionUtils::getSegmentClosestPoint(const vec4f& segment1, const vec4f& segment2, const vec4f& point)
{
	if (segment2 == segment1)
		return segment1;

	const vec4f s = segment2 - segment1;
	const float ss = glm::length2(s);

	return segment1 + glm::clamp(glm::dot(point - segment1, s) / ss, 0.f, 1.f) * s;
}*/
vec4f CollisionUtils::getSegmentClosestPoint(const vec4f& segment1, const vec4f& segment2, const vec4f& point)
{
	if (segment2 == segment1)
		return segment1;

	const vec4f s = segment2 - segment1;
	const float ss = s.getNorm2();

	return segment1 + clamp(vec4f::dot(point - segment1, s) / ss, 0.f, 1.f) * s;
}

std::pair<vec4f, vec4f> CollisionUtils::getSegmentsClosestSegment(const vec4f& segment1a, const vec4f& segment1b, const vec4f& segment2a, const vec4f& segment2b)
{
	//http://geomalgorithms.com/a07-_distance.html
	vec4f u = segment1b - segment1a; u.w = 0.f;
	vec4f v = segment2b - segment2a; v.w = 0.f;
	vec4f w = segment1a - segment2a; w.w = 0.f;

	float a = u.getNorm2();
	float b = vec4f::dot(u, v);
	float c = v.getNorm2();
	float d = vec4f::dot(u, w);
	float e = vec4f::dot(v, w);
	float D = a * c - b * b;

	float t1, t2;
	if (D < COLLISION_EPSILON * COLLISION_EPSILON)
	{
		t1 = 0.0;
		t2 = (b > c ? d / b : e / c);
	}
	else
	{
		t1 = (b * e - c * d) / D;
		t2 = (a * e - b * d) / D;
	}

	if (t1 > 1.f || t2 > 1.f || t1 < 0.f || t2 < 0.f)
	{
		float t[2][2];
		int edge = 0;

		if (t1 < 0.f)
		{
			t[edge][0] = 0.f;
			t[edge][1] = quickClamp(e, c);  // this is a clamp(e/c, 0, 1) but we compute the division just if needed
			edge++;
		}
		else if (t1 > 1.f)
		{
			t[edge][0] = 1.f;
			t[edge][1] = quickClamp(e + b, c);
			edge++;
		}
		if (t2 < 0.f)
		{
			t[edge][0] = quickClamp(-d, a);
			t[edge][1] = 0.f;
			edge++;
		}
		else if (t2 > 1.f)
		{
			t[edge][0] = quickClamp(b - d, a);
			t[edge][1] = 1.f;
			edge++;
		}

		if (edge == 1) // only one edge of the limit square [0,1]x[0,1] is visible from (t1, t2) -> it's candidate
		{
			t1 = t[0][0];
			t2 = t[0][1];
		}
		else if ((w + u * t[0][0] - v * t[0][1]).getNorm2() <= (w + u * t[1][0] - v * t[1][1]).getNorm2()) // we choose the closer candidate (t1,t2)
		{
			t1 = t[0][0];
			t2 = t[0][1];
		}
		else
		{
			t1 = t[1][0];
			t2 = t[1][1];
		}
	}

	return std::pair<vec4f, vec4f>(segment1a + u * t1, segment2a + v * t2);
}

/*vec4f CollisionUtils::getTriangleClosestPoint(const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, const vec4f& point)
{
	return point;
}*/

vec2f CollisionUtils::getBarycentricCoordinates(const vec4f& v1, const vec4f& v2, const vec4f& point, const bool& clamped)
{
	float crossDot = vec4f::dot(v1, v2);
	float magnitute = vec4f::dot(v1, v1) * vec4f::dot(v2, v2) - crossDot * crossDot;
	if (std::abs(magnitute) < 10E-07f)
		return vec2f::zero;
	vec2f barry;

	barry.x = (vec4f::dot(v2, v2) * vec4f::dot(point, v1) - crossDot * vec4f::dot(point, v2)) / magnitute;
	barry.y = (vec4f::dot(v1, v1) * vec4f::dot(point, v2) - crossDot * vec4f::dot(point, v1)) / magnitute;

	if (!clamped)
		return barry;

	vec2f::clamp(barry, vec2f::zero, vec2f::one);
	float length = barry.x + barry.y;
	if (length > 1.f)
		barry /= length;
	return barry;
}
/*vec2f CollisionUtils::getBarycentricCoordinates(const vec4f& v1, const vec4f& v2, const vec4f& point, const bool& clamped)
{
	float crossDot = glm::dot(v1, v2);
	float magnitute = glm::dot(v1, v1) * glm::dot(v2, v2) - crossDot * crossDot;
	vec2f barry;

	barry.x = (glm::dot(v2, v2) * glm::dot(point, v1) - crossDot * glm::dot(point, v2)) / magnitute;
	barry.y = (glm::dot(v1, v1) * glm::dot(point, v2) - crossDot * glm::dot(point, v1)) / magnitute;

	if (!clamped)
		return barry;

	glm::clamp(barry, vec2f(0.f), vec2f(1.f));
	float length = barry.x + barry.y;
	if (length > 1.f)
		barry /= length;
	return barry;
}*/
