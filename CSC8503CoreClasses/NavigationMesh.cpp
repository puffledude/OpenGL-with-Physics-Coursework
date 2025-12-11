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
	NavTri* start	= GetTriForPosition(from);
	NavTri* end	= GetTriForPosition(to);
	if (!start || !end) return false;
	start->f = 0;
	start->g = 0;


	std::vector<NavTri*> openList;
	std::vector <NavTri*> closedList;
	openList.emplace_back(start);
	while (!openList.empty()) {
		NavTri* currentTri = openList.back();
		openList.pop_back();
		closedList.emplace_back(currentTri);
		if (currentTri == end) {
			NavTri* pathTri = end;
			while (pathTri) {
				outPath.PushWaypoint(pathTri->centroid); //Add the position of the centre of the tri as a waypoint
				pathTri = pathTri->parent;  //While there is a parent, keep following it back to the start
			}
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
					neighbour->parent =currentTri;
					neighbour->g =(tentativeG);
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