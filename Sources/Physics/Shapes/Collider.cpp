#include "Collider.h"



//	Default
Collider::Collider(Shape* _shape) : m_shape(_shape)
{}

Collider::~Collider()
{
	if (m_shape)
		delete m_shape;
}
//
