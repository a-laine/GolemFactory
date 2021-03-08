#include "IncrementalHull.h"
#include <Utiles/ToolBox.h>

#define EPSILON 0.00001f

//  Default
IncrementalHull::IncrementalHull() : degenerated(true) {};
//

//	Public functions
Mesh* IncrementalHull::getConvexHull(Mesh* m)
{
	/*	the goal is to begin with a good convex hull and expand it, just by adding one point at a time
		to recompute the new hull we need to introduce the notion of horizon
		this algorithm is the naive and non optimize way for quickhull

		to optimize it and get the quickhull algorithm :
		1. we need to speedup the first loop (for all point in cloud)
		2. we need to compute the horizon thingy just for the most revelant point
		   > to do so we sort all point outside the current hull in different lists (associated to a face)
		   > and for the eye computation we choose the farrest point in the "ouside" list of a face
		3. to compute the horizon we need to loop (for all faces)
		   > to do so each faces is associated to neighbours faces
		   > and we navigate on the "faces graph" starting from a visible face
		   > by doing this we can compute horizon just by iterating on visible faces (more or less)

		first initialize the hull with 4 point -> tetrahedron
		for all point in cloud
			if point is inside hull (of far enough [EPSILON])
				point became an eye and we compute the horizon
				for each edge in horizon
					we add a new triangle constituted by edge and eye
					we add the two new edges if needed
			before iterating we remove all unecessary edges and faces from hull

		final complexity is 0(N²)
		because computing the horizon is dependant on N in worse case
		in actual implementation we implement some helper function that have an unnecessary linear complexity (aka checkFaceNormal and existingEdge)
	*/

	//	initialization
	std::cout << "Hull creation from mesh : " << m->name << std::endl;
	const std::vector<glm::vec3>& pointCloud = *m->getVertices();
	initializeHull(pointCloud);
	unsigned int maxIteration = 10000;
	bool stop = false;

	//	iterate
	for(unsigned int i=0; i < pointCloud.size() && maxIteration && !stop; i++)
	{
		//	test if point inside current hull
		//Face* f = nullptr;
		bool inside = true;
		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
		{
			if (glm::dot(it->n, pointCloud[i] - it->p1) > EPSILON)
			{
				inside = false;
				//f = &(*it);
				break;
			}
		}
		if (inside)// || !f)
			continue;

		//	compute horizon
		for (auto it = hullEdges.begin(); it != hullEdges.end(); it++)
			it->horizonCheck = 2;
		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
			it->onHull = true;
		std::list<Edge*> horizon = computeHorizon(pointCloud[i]);

		//	compute cone faces
		for (auto it = horizon.begin(); it != horizon.end(); it++)
		{
			//  create tmp face
			Face tmp((*it)->p1, (*it)->p2, pointCloud[i], glm::normalize(glm::cross((*it)->p2 - (*it)->p1, pointCloud[i] - (*it)->p1)));
			if (checkFaceNormal(tmp)) tmp.n *= -1.f;

			Face* otherFace = nullptr;
			if ((*it)->f1) otherFace = (*it)->f1;
			else if ((*it)->f2) otherFace = (*it)->f2;
			else 
			{
				stop = true;
				std::cout << "IncrementalHull : Fatal error in horizon cone : both faces of an horizon edge are not on hull" << std::endl;
			}
			if (!otherFace) continue;

			//	create cone face and add it to hull
			hullFaces.insert(hullFaces.end(), Face((*it)->p1, (*it)->p2, pointCloud[i], tmp.n));
			Face* face = &hullFaces.back();

			//  create new horizon edge
			if (!(*it)->f1) (*it)->f1 = face;
			else if(!(*it)->f2) (*it)->f2 = face;
			else 
			{
				stop = true;
				std::cout << "IncrementalHull : Fatal error in horizon cone : both faces of an horizon edge are already set" << std::endl;
			}
			face->e1 = (*it);

			// create others edges
			Edge* e2 = existingEdge((*it)->p1, pointCloud[i]);
			if (!e2)
			{
				hullEdges.insert(hullEdges.end(), Edge((*it)->p1, pointCloud[i]));
				e2 = &hullEdges.back();
			}
			if (!e2->f1) e2->f1 = face;
			else if (!e2->f2) e2->f2 = face;
			else 
			{
				stop = true;
				std::cout << "IncrementalHull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			}
			face->e2 = e2;

			Edge* e3 = existingEdge((*it)->p2, pointCloud[i]);
			if (!e3)
			{
				hullEdges.insert(hullEdges.end(), Edge((*it)->p2, pointCloud[i]));
				e3 = &hullEdges.back();
			}
			if (!e3->f1) e3->f1 = face;
			else if (!e3->f2) e3->f2 = face;
			else
			{
				stop = true;
				std::cout << "IncrementalHull : Fatal error in horizon cone : a new edge already have 2 faces" << std::endl;
			}
			face->e3 = e3;
		}



		//	clear hull from dead entities
		for (auto it = hullEdges.begin(); it != hullEdges.end();)
		{
			if (it->horizonCheck == 0) it = hullEdges.erase(it);
			else it++;
		}
		for (auto it = hullFaces.begin(); it != hullFaces.end();)
		{
			if (it->onHull) it++;
			else it = hullFaces.erase(it);
		}
		maxIteration--;
	}


	//	generate a drawable mesh from hull
	Mesh* mesh = new Mesh("");
	if (degenerated)
	{
		std::cout << "  degenerated mesh !" << std::endl;
		mesh->initialize(std::vector<glm::vec3>(), std::vector<glm::vec3>(), std::vector<glm::vec3>(), std::vector<unsigned short>(), std::vector<glm::ivec3>(), std::vector<glm::vec3>());
	}
	else
	{
		glm::vec3 hullColor = glm::vec3(0.5f, 0.f, 1.f);
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> colors;
		std::vector<unsigned short> faces;

		for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
		{
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p1);
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p2);
			faces.push_back((unsigned short)vertices.size());  vertices.push_back(it->p3);

			colors.push_back(hullColor);
			colors.push_back(hullColor);
			colors.push_back(hullColor);

			normales.push_back(glm::normalize(it->n));
			normales.push_back(normales.back());
			normales.push_back(normales.back());
		}
		ToolBox::optimizeStaticMesh(vertices, normales, colors, faces);
		mesh->initialize(vertices, normales, colors, faces, std::vector<glm::ivec3>(), std::vector<glm::vec3>());
	}
	return mesh;
}
//

//	Protected functions
void IncrementalHull::initializeHull(const std::vector<glm::vec3>& pointCloud)
{
	/*	the goal of the initialisation is to get the biggest non degenerated tetrahedron composed by 4 point of the mesh vertices cloud (more or less)
		to do so
		1. we search for the giggest segment on one axis
		   (actually it should be better if we have the longest segment, regardless of any axis, but complexity is n² to do that)
		2. we search for the farrest point from this segment to get a triangle
		3. we search for the farrest point from this triangle to get tetrahedron
	
		initial segment pseudo-code
		for each point 
			for each searching axis
				if point is better for one point slot (point min on x axis, point max on x axis, point min on y axis, etc ...)
					replace current slot
		test computed slot against each other and choose the best axis
		the initial wanted segment is now contructed by the two winning slot points

		final complexity is linear of number of vertices in point cloud
		O(3N) = O(N)
	*/

	//	compute initial segment :
	degenerated = false;

	glm::vec3 p1, p2;
	float minx = std::numeric_limits<float>::max();     glm::vec3 x;
	float maxx = std::numeric_limits<float>::min();     glm::vec3 X;
	float miny = std::numeric_limits<float>::max();		glm::vec3 y;
	float maxy = std::numeric_limits<float>::min();		glm::vec3 Y;
	float minz = std::numeric_limits<float>::max();		glm::vec3 z;
	float maxz = std::numeric_limits<float>::min();		glm::vec3 Z;

	for (unsigned int j = 0; j < pointCloud.size(); j++)
	{
		if (pointCloud[j].x < minx && x != pointCloud[j])
		{
			minx = pointCloud[j].x;
			x = pointCloud[j];
		}
		if (pointCloud[j].x > maxx && X != pointCloud[j])
		{
			maxx = pointCloud[j].x;
			X = pointCloud[j];
		}

		if (pointCloud[j].y < miny && y != pointCloud[j])
		{
			miny = pointCloud[j].y;
			y = pointCloud[j];
		}
		if (pointCloud[j].y > maxy && Y != pointCloud[j])
		{
			maxy = pointCloud[j].y;
			Y = pointCloud[j];
		}

		if (pointCloud[j].z < minz && z != pointCloud[j])
		{
			minz = pointCloud[j].z;
			z = pointCloud[j];
		}
		if (pointCloud[j].z > maxz && Z != pointCloud[j])
		{
			maxz = pointCloud[j].z;
			Z = pointCloud[j];
		}
	}

	if (x == X) degenerated = true;
	else if (y == Y) degenerated = true;
	else if (z == Z) degenerated = true;
	if (degenerated) return;

	float d1 = maxx - minx;
	float d2 = maxy - miny;
	float d3 = maxz - minz;
	if (d1 > d2 && d1 > d3) { p1 = x, p2 = X; }
	else if (d2 > d1 && d2 > d3) { p1 = y, p2 = Y; }
	else { p1 = z, p2 = Z; };

	// search maximum distant point from initial segment
	float maxd = std::numeric_limits<float>::min();
	glm::vec3 T;
	glm::vec3 u = p2 - p1;
	for (unsigned int j = 0; j < pointCloud.size(); j++)
	{
		float d = glm::length(glm::cross(pointCloud[j] - p1, u));
		if (d > maxd)
		{
			maxd = d;
			T = pointCloud[j];
		}
	}
	if (maxd == 0.f) // mesh is 1D
	{
		degenerated = true;
		return;
	}

	// search maximum distant point from triangle
	maxd = std::numeric_limits<float>::min();
	glm::vec3 P;
	glm::vec3 n = glm::cross(T - p1, u);
	for (unsigned int j = 0; j < pointCloud.size(); j++)
	{
		float d = std::abs(glm::dot(pointCloud[j], n));
		if (d > maxd)
		{
			maxd = d;
			P = pointCloud[j];
		}
	}
	if (maxd == 0.f) // mesh is 2D
	{
		degenerated = true;
		return;
	}


	// construct tetrahedron
	hullEdges.push_back(Edge(p1, p2));
	hullEdges.push_back(Edge(p1, T));
	hullEdges.push_back(Edge(p1, P));
	hullEdges.push_back(Edge(p2, T));
	hullEdges.push_back(Edge(p2, P));
	hullEdges.push_back(Edge(T, P));

	Face f1(p1, p2, T, glm::normalize(glm::cross(u, T - p1)));
	if (glm::dot(f1.n, P) > 0) f1.n *= -1.f;
	hullFaces.push_back(f1);

	Face f2(p1, p2, P, glm::normalize(glm::cross(u, P - p1)));
	if (glm::dot(f2.n, T) > 0) f2.n *= -1.f;
	hullFaces.push_back(f2);

	Face f3(p1, T, P, glm::normalize(glm::cross(T - p1, P - p1)));
	if (glm::dot(f3.n, p2) > 0) f3.n *= -1.f;
	hullFaces.push_back(f3);

	Face f4(p2, T, P, glm::normalize(glm::cross(T - p2, P - p2)));
	if (glm::dot(f4.n, p1) > 0) f4.n *= -1.f;
	hullFaces.push_back(f4);

	// assign pointers
	for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
	{
		for (auto it2 = hullEdges.begin(); it2 != hullEdges.end(); it2++)
		{
			if (isFaceEdge(*it, *it2))
			{
				if (it->e1 == nullptr) it->e1 = &(*it2);
				else if (it->e2 == nullptr) it->e2 = &(*it2);
				else if (it->e3 == nullptr) it->e3 = &(*it2);
				else std::cout << "IncrementalHull : Fatal error ! : a face (triangle) has more than 3 edges" << std::endl;

				if (it2->f1 == nullptr) it2->f1 = &(*it);
				else if (it2->f2 == nullptr) it2->f2 = &(*it);
				else std::cout << "IncrementalHull : Fatal error ! : an edge has more than 2 faces" << std::endl;
			}
			if (it->e3 != nullptr)
				break;
		}
	}

}
std::list<IncrementalHull::Edge*> IncrementalHull::computeHorizon(const glm::vec3& eye)
{
	/*	the goal of this function is to get all the edges constituting the horizon from an eye point
		
		for all faces of current hull
			if face is visible from eye
				push all edges of face into an horizon list
				mark these edges as seen once (decrement a int starting from 2)
				unassign the edge face reference corresponding to current face (prepare for face deletion)
		for all edges in horizon list
			if edge is seen twice (both face of the edge are seen from eye)
				so remove current edge from horizon list
		return horizon

		final complexity is linear in current hull faces
	*/


	std::list<Edge*> horizon;
	for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
	{
		if (glm::dot(it->n, eye - it->p1) > 0)
		{
			it->onHull = false;

			it->e1->horizonCheck--;
			it->e2->horizonCheck--;
			it->e3->horizonCheck--;

			horizon.insert(horizon.end(), it->e1);
			horizon.insert(horizon.end(), it->e2);
			horizon.insert(horizon.end(), it->e3);

			if (it->e1->f1 == &(*it)) it->e1->f1 = nullptr;
			else if (it->e1->f2 == &(*it)) it->e1->f2 = nullptr;
			if (it->e2->f1 == &(*it)) it->e2->f1 = nullptr;
			else if (it->e2->f2 == &(*it)) it->e2->f2 = nullptr;
			if (it->e3->f1 == &(*it)) it->e3->f1 = nullptr;
			else if (it->e3->f2 == &(*it)) it->e3->f2 = nullptr;
		}
	}

	for (auto it = horizon.begin(); it != horizon.end(); )
	{
		if ((*it)->horizonCheck == 0) it = horizon.erase(it);
		else it++;
	}
	return horizon;
}
bool IncrementalHull::isFaceEdge(const Face& f, const Edge& e)
{
	if (f.p1 == e.p1 && f.p2 == e.p2) return true;			// just f.p3 is not in edge
	else if (f.p2 == e.p1 && f.p3 == e.p2) return true;		// just f.p1 is not in edge
	else if (f.p1 == e.p1 && f.p3 == e.p2) return true;		// just f.p2 is not in edge
	else if (f.p1 == e.p2 && f.p2 == e.p1) return true;		// just f.p3 is not in edge
	else if (f.p2 == e.p2 && f.p3 == e.p1) return true;		// just f.p1 is not in edge
	else if (f.p1 == e.p2 && f.p3 == e.p1) return true;		// just f.p2 is not in edge
	else return false;
}
IncrementalHull::Edge* IncrementalHull::existingEdge(const glm::vec3& p1, const glm::vec3& p2)
{
	for (auto it = hullEdges.begin(); it != hullEdges.end(); it++)
	{
		if (it->p1 == p1 && it->p2 == p2)
			return &(*it);
		else if (it->p1 == p2 && it->p2 == p1)
			return &(*it);
	}
	return nullptr;
}
bool IncrementalHull::checkFaceNormal(const Face& f)
{
	for (auto it = hullFaces.begin(); it != hullFaces.end(); it++)
	{
		if (glm::dot(f.n, it->p1 - f.p1) > EPSILON) return true;
		else if (glm::dot(f.n, it->p2 - f.p1) > EPSILON) return true;
		else if (glm::dot(f.n, it->p3 - f.p1) > EPSILON) return true;
	}
	return false;
}
//