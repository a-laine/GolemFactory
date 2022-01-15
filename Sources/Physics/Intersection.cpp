#include "Intersection.h"
#include "GJK.h"
#include <Resources/Mesh.h>

#include <iostream>
#include <string>





/*std::string shapeTypeToString(const Shape::ShapeType& type)
{
	switch (type)
	{
		case Shape::ShapeType::POINT:				return "point";
		case Shape::ShapeType::SEGMENT:			return "segment";
		case Shape::ShapeType::TRIANGLE:			return "triangle";
		case Shape::ShapeType::ORIENTED_BOX:		return "oriented box";
		case Shape::ShapeType::AXIS_ALIGNED_BOX:	return "axis aligned box";
		case Shape::ShapeType::SPHERE:				return "sphere";
		case Shape::ShapeType::CAPSULE:			return "capsule";
		case Shape::ShapeType::HULL:				return "hull";
		default:						return "unknown";
	}
}

//	Public field
Intersection::Contact Intersection::intersect(const Shape& a, const Shape& b)
{
	//	order objects
	bool swaped = a.type > b.type;
	Shape& Shape1 = swaped ? (Shape&)b : (Shape&)a;
	Shape& Shape2 = swaped ? (Shape&)a : (Shape&)b;
	
	switch (Shape1.type)
	{
		case Shape::ShapeType::POINT:
			if(swaped) return intersect_PointvsShape(Shape1, Shape2).swap();
			else return intersect_PointvsShape(Shape1, Shape2);
		case Shape::ShapeType::SEGMENT:
			if (swaped) return intersect_SegmentvsShape(Shape1, Shape2).swap();
			else return intersect_SegmentvsShape(Shape1, Shape2);
		case Shape::ShapeType::TRIANGLE:
			if (swaped) return intersect_TrianglevsShape(Shape1, Shape2).swap();
			else return intersect_TrianglevsShape(Shape1, Shape2);
		case Shape::ShapeType::ORIENTED_BOX:
			if (swaped) return intersect_OrientedBoxvsShape(Shape1, Shape2).swap();
			else return intersect_OrientedBoxvsShape(Shape1, Shape2);
		case Shape::ShapeType::AXIS_ALIGNED_BOX:
			if (swaped) return intersect_AxisAlignedBoxvsShape(Shape1, Shape2).swap();
			else return intersect_AxisAlignedBoxvsShape(Shape1, Shape2);
		case Shape::ShapeType::SPHERE:
			if (swaped) return intersect_SpherevsShape(Shape1, Shape2).swap();
			else return intersect_SpherevsShape(Shape1, Shape2);
		case Shape::ShapeType::CAPSULE:
			if (swaped) return intersect_CapsulevsShape(Shape1, Shape2).swap();
			else return intersect_CapsulevsShape(Shape1, Shape2);
		default:						
			return Intersection::Contact(); //GJK::intersect(a, b);
	}
}
//*/


//	Unitary tests
int Intersection::debugUnitaryTest(const int& verboseLevel, const Hull* testHull)
{
	int errorCount = 0;

	//


	return errorCount;
}
//