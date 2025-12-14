#include "NavigationMesh.h"
#include "Assets.h"
#include "Maths.h"
#include <fstream>
using namespace NCL;
using namespace CSC8503;
using namespace std;

NavigationMesh::NavigationMesh()
{
}

NavigationMesh::NavigationMesh(const std::string&filename)
{
	ifstream file(Assets::DATADIR + filename);

	int numVertices = 0;
	int numIndices	= 0;

	file >> numVertices;
	file >> numIndices;

	for (int i = 0; i < numVertices; ++i) {
		Vector3 vert;
		file >> vert.x;
		file >> vert.y;
		file >> vert.z;

		allVerts.emplace_back(vert);
	}
	float navScale = 8.0f; // Scale mesh to the dimensions of this project

	for (Vector3& v : allVerts) {
		v *= navScale;
		v.y += 6.0f;
	}

	allTris.resize(numIndices / 3);

	for (int i = 0; i < allTris.size(); ++i) {
		NavTri* tri = &allTris[i];
		file >> tri->indices[0];
		file >> tri->indices[1];
		file >> tri->indices[2];

		tri->centroid = allVerts[tri->indices[0]] +
			allVerts[tri->indices[1]] +
			allVerts[tri->indices[2]];

		tri->centroid = allTris[i].centroid / 3.0f;

		tri->triPlane = Plane::PlaneFromTri(allVerts[tri->indices[0]],
			allVerts[tri->indices[1]],
			allVerts[tri->indices[2]]);

		tri->area = Maths::AreaofTri3D(allVerts[tri->indices[0]], allVerts[tri->indices[1]], allVerts[tri->indices[2]]);
	}
	for (int i = 0; i < allTris.size(); ++i) {
		NavTri* tri = &allTris[i];
		for (int j = 0; j < 3; ++j) {
			int index = 0;
			file >> index;
			if (index != -1) {
				tri->neighbours[j] = &allTris[index];
			}
		}
	}
}

NavigationMesh::~NavigationMesh()
{
}

bool NavigationMesh::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	std::vector<NavTri*> outTris;
	bool pathMade = AStarAlgorithm(from, to, outTris);
	ResetNavTris();
	if (pathMade) {
		return SmoothPath(from, to, outTris, outPath);
	}
	return false;
}

bool NavigationMesh::AStarAlgorithm(const Vector3& from, const Vector3& to, std::vector<NavTri*>& outTris) {
	NavTri* start = GetTriForPosition(from);
	NavTri* end = GetTriForPosition(to);
	if (!start || !end) return false;
	start->f = 0;
	start->g = 0;


	std::vector<NavTri*> openList;
	std::vector <NavTri*> closedList;
	openList.emplace_back(start);
	while (!openList.empty()) {
	/*	NavTri* currentTri = openList.back();
		openList.pop_back();*/
		auto bestIt = std::min_element(openList.begin(), openList.end(),
			[](NavTri* a, NavTri* b) { return a->f < b->f; });

		NavTri* currentTri = *bestIt;
		openList.erase(bestIt);
		closedList.emplace_back(currentTri);
		if (currentTri == end) {
			NavTri* pathTri = end;
			while (pathTri) {
				outTris.emplace_back(pathTri);
				pathTri = pathTri->parent;
			}
			//std::reverse(outTris.begin(), outTris.end());
			return true;

		}
		else {
			for (int i = 0; i < 3; i++) { //For each of the triangles neighbours
				NavTri* neighbour = currentTri->neighbours[i];
				if (!neighbour) {
					continue;
				}

				if (std::find(closedList.begin(), closedList.end(), neighbour) != closedList.end()) {
					continue; //Already processed
				}

				float tentativeG = neighbour->g + Herustic(currentTri->centroid, neighbour->centroid);

				bool inOpen = (std::find(openList.begin(), openList.end(), neighbour) != openList.end());
				if (tentativeG < neighbour->g || !inOpen) {
					neighbour->parent = currentTri;
					neighbour->g = (tentativeG);
					neighbour->h = Herustic(neighbour->centroid, end->centroid);
					neighbour->f = neighbour->g + neighbour->h;
					if (!inOpen) {
						openList.emplace_back(neighbour);
					}

				}
			}
		}
	}
	return false;
}

bool NavigationMesh::SmoothPath(const Vector3& from, const Vector3& to, std::vector<NavTri*>& outTris, NavigationPath& path) {
	//Without smoothing, will go jagidly from one triangle centre to the next.
	//With smoothing, make things look more natural
	//Use the funnel algorithm.

	//Taken from https://jceipek.com/Olin-Coding-Tutorials/pathing.html
	//The Simple Stupid Funnel Algorithm
	/*This algorithm is all about smoothing paths by finding corners on the minimum distance path of a navigation mesh without leaving the walkable area.The result is the same as if the A* path were a string being pulled until it was taut.

		The algorithm proceeds as follows :

	Create a list of the portals along the A* path.Make sure that the points of each portal are stored the same way relative to the character.You will need to know if a point is to the left or right of the character.
		Create a "funnel" with three points : the characters starting location(the apex), the right side of the portal, and the left side of the portal.
		Alternate updating the left and right sides of the "funnel," making it narrower each time
		When the sides of the funnel cross, make the point you didn't update the apex of the new funnel, and store it as part of the smoothed path.*/

	if (outTris.size() == 0) {
		return false;
	}

	struct Portal {
		Vector3 left;
		Vector3 right;
	};
	//Portal is a pair of edges. To make portals, need to find the shared edge edge between each triangle.
	std::vector<Portal> portals;
	for (int i = 0; i < outTris.size(); i++) 
	{
		NavTri* currentTri = outTris[i];
		if (i < outTris.size() - 1) {
			NavTri* nextTri = outTris[i + 1];
			int indexA=-1, indexB=-1;
			if (!SharedEdge(currentTri, nextTri, indexA, indexB)) {
				continue;
			}
			//Need to now determine which way round the edge goes.
			Portal p;
			Vector3 p0 = allVerts[indexA];
			Vector3 p1 = allVerts[indexB];

			// Determine left/right
			Vector3 dir = Vector::Normalise(nextTri->centroid - currentTri->centroid);
			Vector3 edge = p1 - p0;

			bool p1IsLeft = Vector::Cross(edge, dir).y > 0;

			if (p1IsLeft)
				portals.emplace_back(p1, p0);   // (left, right)
			else
				portals.emplace_back(p0, p1);


		}
		else {
			portals.emplace_back(to, to);
		}

	}


	//Now have the portals, with left and right vertices. 
	//Now for main algorithm.

	Vector3 apex = from;
	int apexIndex = 0;
	path.PushWaypoint(apex);
	Vector3 left = portals[0].left;
	int leftIndex = 0;
	Vector3 right = portals[0].right;
	int rightIndex = 0;

	//How this bit actually works. Go through each vert on the left hand side until the angle between the apex->left and apex->centre starts increasing.
	//Thee vert before the increase is now the new left vert.
	//Then travel down the right hand side the same way.

	for (int i = 1; i < portals.size(); i++) {
		Vector3 leftVert = portals[i].left;
		Vector3 rightVert = portals[i].right;

		//Left side
		if (isLeftOf(apex, left, leftVert)) {
			if (Vector::Length(apex-left)<0.000005 || !isLeftOf(apex, right, leftVert)) {
				left = leftVert;
				leftIndex = i;
			}
			else {
				apex = right;
				path.PushWaypoint(apex);

				apexIndex = rightIndex;
				i = apexIndex;

				left = apex;
				right = apex;

				leftIndex = apexIndex;
				rightIndex = apexIndex;

				continue;
			}
		}
		//Right side
		if (!isLeftOf(apex, right, rightVert)) {
			if (Vector::Length(apex-right)<0.000005 || isLeftOf(apex, left, rightVert)) {
				right = rightVert;
				rightIndex = i;
			}
			else {
				apex = left;
				path.PushWaypoint(apex);

				apexIndex = leftIndex;
				i = apexIndex;

				left = apex;
				right = apex;
				leftIndex = apexIndex;
				rightIndex = apexIndex;
				continue;
			}
		}
	}
	path.PushWaypoint(to);

	return true;
}

bool NavigationMesh::isLeftOf(const Vector3& a, const Vector3& b, const Vector3& point) {
	Vector3 ab = b - a;
	Vector3 ap = point - a;
	Vector3 cross = Vector::Cross(ab, ap);
	return cross.y > 0;
}

/// <summary>
/// Find a shared edge between two triangles
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <param name="outA"></param>
/// <param name="outB"></param>
/// <returns></returns>
bool NavigationMesh::SharedEdge(const NavigationMesh::NavTri* a, const NavigationMesh::NavTri* b, int& outA, int& outB) const {
	for (int i = 0; i < 3; i++) {
		int aIndexA = a->indices[i];
		int aIndexB = a->indices[(i + 1) % 3];
		if (aIndexA == -1 || aIndexB == -1) {
			continue;
		}
		for (int j = 0; j < 3; j++) {
			int bIndexA = b->indices[j];
			int bIndexB = b->indices[(j + 1) % 3];
			if(bIndexA == -1 || bIndexB == -1) {
				continue;
			}
			if ((aIndexA == bIndexB) && (aIndexB == bIndexA) || (aIndexA == bIndexA) && (aIndexB == bIndexB)) {
				outA = aIndexA; // actual vertex index in allVerts
				outB = aIndexB;
				return true;
			}
		}
	}
	return false;
}



/*
If you have triangles on top of triangles in a full 3D environment, you'll need to change this slightly,
as it is currently ignoring height. You might find tri/plane raycasting is handy.
*/

const NavigationMesh::NavTri* NavigationMesh::GetTriForPosition(const Vector3& pos) const {
	for (const NavTri& t : allTris) {
		Vector3 planePoint = t.triPlane.ProjectPointOntoPlane(pos);

		float ta = Maths::AreaofTri3D(allVerts[t.indices[0]], allVerts[t.indices[1]], planePoint);
		float tb = Maths::AreaofTri3D(allVerts[t.indices[1]], allVerts[t.indices[2]], planePoint);
		float tc = Maths::AreaofTri3D(allVerts[t.indices[2]], allVerts[t.indices[0]], planePoint);

		float areaSum = ta + tb + tc;

		if (abs(areaSum - t.area)  > 0.001f) { //floating points are annoying! Are we more or less inside the triangle?
			continue;
		}
		return &t;
	}
	return nullptr;
}

NavigationMesh::NavTri* NavigationMesh::GetTriForPosition(const Vector3& pos) {
	for (NavTri& t : allTris) {
		Vector3 planePoint = t.triPlane.ProjectPointOntoPlane(pos);

		float ta = Maths::AreaofTri3D(allVerts[t.indices[0]], allVerts[t.indices[1]], planePoint);
		float tb = Maths::AreaofTri3D(allVerts[t.indices[1]], allVerts[t.indices[2]], planePoint);
		float tc = Maths::AreaofTri3D(allVerts[t.indices[2]], allVerts[t.indices[0]], planePoint);

		float areaSum = ta + tb + tc;

		if (abs(areaSum - t.area)  > 0.001f) { //floating points are annoying! Are we more or less inside the triangle?
			continue;
		}
		return &t;
	}
	return nullptr;
}