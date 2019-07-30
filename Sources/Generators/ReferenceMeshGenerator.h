#pragma once

#include "Resources/ResourceManager.h"
#include "Resources/Mesh.h"


class ReferenceMeshGenerator
{
	public:
		//	Public functions
		/*!
		 *	\brief Construct and initialize mesh for the reference capsule.
		 *
		 *	The reference capsule is a capsule : 
		 *    - p1 = vec3(0, 0,  1)
		 *    - p2 = vec3(0, 0, -1)
		 *    - radius = 1.f
		 *
		 *  \param quadrature : approximate the circle perimeter by a regular polygon of <B>quadrature</B> sides
		 *	\return a pointer on the generated mesh
		 */
		static Mesh* getReferenceCapsule(const unsigned int& quadrature);
		//
};
