#include "CollisionUtils.h"

#include <Physics/Collision.h>

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

vec4f CollisionUtils::getTriangleClosestPoint(const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, const vec4f& point, ClosestPointCategory* optionnal0, vec3f* optionnal1)
{
	constexpr float eps = 1E-06f;
	vec4f t1t2 = triangle2 - triangle1; t1t2.w = 0.f;
	vec4f t1t3 = triangle3 - triangle1; t1t3.w = 0.f;
	vec4f pt1 = point - triangle1;

	float d1 = vec4f::dot(t1t2, pt1);
	float d2 = vec4f::dot(t1t3, pt1);
	if (d1 <= 0.f && d2 <= 0.f)
	{
		if (optionnal0) *optionnal0 = ClosestPointCategory::eVertex1;
		if (optionnal1) *optionnal1 = vec3f(1, 0, 0);
		return triangle1;
	}

	vec4f pt2 = point - triangle2;
	float d3 = vec4f::dot(t1t2, pt2);
	float d4 = vec4f::dot(t1t3, pt2);
	if (d3 >= 0.f && d4 <= d3)
	{
		if (optionnal0) *optionnal0 = ClosestPointCategory::eVertex2;
		if (optionnal1) *optionnal1 = vec3f(0, 1, 0);
		return triangle2;
	}

	vec4f pt3 = point - triangle3;
	float d5 = vec4f::dot(t1t2, pt3);
	float d6 = vec4f::dot(t1t3, pt3);
	if (d6 >= 0.f && d5 <= d6)
	{
		if (optionnal0) *optionnal0 = ClosestPointCategory::eVertex3;
		if (optionnal1) *optionnal1 = vec3f(0, 0, 1);
		return triangle3;
	}

	float vp2 = d1 * d4 - d2 * d3;
	if (vp2 <= 0.f && d1 >= 0.f && d3 <= 0.f)
	{
		float f = d1 / std::max(eps, d1 - d3);
		if (optionnal0) *optionnal0 = ClosestPointCategory::eSegment12;
		if (optionnal1) *optionnal1 = vec3f(1 - f, f, 0);
		return triangle1 + f * t1t2;
	}

	float vp1 = d2 * d5 - d1 * d6;
	if (vp1 <= 0.f && d2 >= 0.f && d6 <= 0.f)
	{
		float f = d2 / std::max(eps, d2 - d6);
		if (optionnal0) *optionnal0 = ClosestPointCategory::eSegment13;
		if (optionnal1) *optionnal1 = vec3f(1 - f, 0, f);
		return triangle1 + f * t1t3;
	}

	float vp0 = d3 * d6 - d4 * d5;
	float d4d3 = d4 - d3;
	float d5d6 = d5 - d6;
	if (vp0 <= 0.f && d4d3 >= 0.f && d5d6 >= 0.f)
	{
		float f = d4d3 / std::max(eps, d4d3 + d5d6);
		vec4f t2t3 = triangle3 - triangle2; t2t3.w = 0.f;
		if (optionnal0) *optionnal0 = ClosestPointCategory::eSegment23;
		if (optionnal1) *optionnal1 = vec3f(0, 1 - f, f);
		return triangle2 + f * t2t3;
	}

	float denom = std::max(eps, vp0 + vp1 + vp2);
	float u = vp1 / denom;
	float v = vp2 / denom;
	if (optionnal0) *optionnal0 = ClosestPointCategory::eInside;
	if (optionnal1) *optionnal1 = vec3f(1 - u - v, u, v);
	return triangle1 + v * t1t3 + u * t1t2;
}

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


std::pair<vec4f, vec4f> CollisionUtils::getClosestPair(const vec4f& segment1, const vec4f& segment2, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3)
{
	GJK::MinkowskiPoint pointCloud[6];
	vec4f barycenter = vec4f::zero;
	pointCloud[0] = GJK::MinkowskiPoint(segment1, triangle1); barycenter += pointCloud[0].p;
	pointCloud[1] = GJK::MinkowskiPoint(segment1, triangle2); barycenter += pointCloud[1].p;
	pointCloud[2] = GJK::MinkowskiPoint(segment1, triangle3); barycenter += pointCloud[2].p;
	pointCloud[3] = GJK::MinkowskiPoint(segment2, triangle1); barycenter += pointCloud[3].p;
	pointCloud[4] = GJK::MinkowskiPoint(segment2, triangle2); barycenter += pointCloud[4].p;
	pointCloud[5] = GJK::MinkowskiPoint(segment2, triangle3); barycenter += pointCloud[5].p;
	barycenter /= 6.f;

	// search closest minkowski vertex to 0
	int closestVertex0 = 0;
	float dmin = pointCloud[0].p.getNorm2();
	for (int i = 1; i < 6; i++)
	{
		float d = pointCloud[i].p.getNorm2();
		if (d < dmin)
		{
			dmin = d;
			closestVertex0 = i;
		}
	}

	// search closest face containing closestVertex
	int closestVertex1 = 0;
	int closestVertex2 = 0;
	bool flip12 = false;
	ClosestPointCategory category = ClosestPointCategory::eVertex1;
	vec3f barycentric = vec3f(1, 0, 0);
	vec4f minClosest;
	float eps = COLLISION_EPSILON * COLLISION_EPSILON;
	dmin = 1E12f;
	for (int i = 0; i < 6; i++)
	{
		if (i == closestVertex0)
			continue;
		for (int j = 0; j < 6; j++)
		{
			if (j == closestVertex0 || j == i)
				continue;

			vec4f e0 = pointCloud[i].p - pointCloud[closestVertex0].p;
			vec4f e1 = pointCloud[j].p - pointCloud[closestVertex0].p;
			vec4f n = vec4f::cross(e0, e1);
			if (std::abs(n.x) < eps && std::abs(n.y) < eps && std::abs(n.z) < eps)
				continue;

			n.normalize();
			bool flip = vec4f::dot(n, barycenter - pointCloud[closestVertex0].p) > 0.f;
			if (flip)
				n *= -1.f;

			bool isHullFace = true;
			for (int k = 0; k < 6; k++)
			{
				if (k == i || k == j || k == closestVertex0)
					continue;

				if (vec4f::dot(n, pointCloud[k].p - pointCloud[closestVertex0].p) > 0.f)
				{
					isHullFace = false;
					break;
				}
			}
			if (!isHullFace)
				continue;

			vec3f b;
			vec4f closest = CollisionUtils::getTriangleClosestPoint(pointCloud[closestVertex0].p, pointCloud[i].p, pointCloud[j].p, vec4f::zero, &category, &b);
			float d = closest.getNorm2();
			if (d < dmin)
			{
				closestVertex1 = i;
				closestVertex2 = j;
				barycentric = b;
				minClosest = closest;
				flip12 = flip;
				dmin = d;
			}
		}
	}

	if (flip12)
	{
		int tmp = closestVertex1;
		closestVertex1 = closestVertex2;
		closestVertex2 = tmp;

		float tmpf = barycentric.y;
		barycentric.y = barycentric.z;
		barycentric.z = tmpf;
	}

	const auto& p0 = pointCloud[closestVertex0];
	const auto& p1 = pointCloud[closestVertex1];
	const auto& p2 = pointCloud[closestVertex2];

	//barycentric = vec3f(getBarycentricCoordinates(p1.p - p0.p, p2.p - p0.p, minClosest - p0.p, true), 0);
	//vec4f pairSegment  = p0.a + barycentric.x * (p1.a - p0.a) + barycentric.y * (p2.a - p0.a);
	//vec4f pairTriangle = p0.b + barycentric.x * (p1.b - p0.b) + barycentric.y * (p2.b - p0.b);

	barycentric = vec3f::clamp(barycentric, vec3f::zero, vec3f(1.f));
	vec4f pairSegment  = barycentric.x * p0.a + barycentric.y * p1.a + barycentric.z * p2.a;
	vec4f pairTriangle = barycentric.x * p0.b + barycentric.y * p1.b + barycentric.z * p2.b;
	return { pairSegment, pairTriangle };
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
