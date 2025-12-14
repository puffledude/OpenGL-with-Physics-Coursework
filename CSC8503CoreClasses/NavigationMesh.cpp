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
			std::reverse(outTris.begin(), outTris.end());
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

				float tentativeG = currentTri->g +
					Herustic(currentTri->centroid, neighbour->centroid);

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
	if (outTris.empty()) return false;

	struct Portal { Vector3 left; Vector3 right; };
	std::vector<Portal> portals;
	portals.reserve(outTris.size());

	// Build portals between triangle i and i+1
	for (size_t i = 0; i + 1 < outTris.size(); ++i) {
		NavTri* a = outTris[i];
		NavTri* b = outTris[i + 1];

		int ia = -1, ib = -1;
		if (!SharedEdge(a, b, ia, ib)) return false;

		Vector3 p0 = allVerts[ia];
		Vector3 p1 = allVerts[ib];

		// Orient portal consistently using direction between triangle centroids
		Vector3 dir = Vector::Normalise(b->centroid - a->centroid);
		Vector3 edge = p1 - p0;
		bool p1IsLeft = Vector::Cross(edge, dir).y > 0.0f;

		if (p1IsLeft) portals.push_back({ p1, p0 }); // left, right
		else          portals.push_back({ p0, p1 });
	}
	// final portal is the goal (degenerate)
	portals.push_back({ to, to });

	// Funnel initialization
	const float EPS_PUSH = 0.0005f;
	Vector3 apex = from;
	int apexIndex = 0;
	path.PushWaypoint(apex);

	Vector3 left = portals[0].left;
	int leftIndex = 0;
	Vector3 right = portals[0].right;
	int rightIndex = 0;

	// Iterate with explicit control to avoid bouncing
	size_t i = 1;
	int iter = 0;
	const int maxIter = (int)portals.size() * 10 + 100; // safety cap
	while (i < portals.size() && iter++ < maxIter) {
		Vector3 leftVert  = portals[i].left;
		Vector3 rightVert = portals[i].right;

		// Left update
		if (isLeftOf(apex, left, leftVert)) {
			// left moved inward
			if (Vector::Length(apex - left) < 1e-6f || !isLeftOf(apex, right, leftVert)) {
				left = leftVert;
				leftIndex = (int)i;
			} else {
				// right crosses left -> push right, make right the new apex
				if (Vector::Length(right - apex) > EPS_PUSH) {
					path.PushWaypoint(right);
				}
				apex = right;
				apexIndex = rightIndex;
				// reset funnel
				left = apex;
				right = apex;
				leftIndex = apexIndex;
				rightIndex = apexIndex;
				i = (size_t)apexIndex + 1;
				continue;
			}
		}

		// Right update
		if (!isLeftOf(apex, right, rightVert)) {
			// right moved inward
			if (Vector::Length(apex - right) < 1e-6f || isLeftOf(apex, left, rightVert)) {
				right = rightVert;
				rightIndex = (int)i;
			} else {
				// left crosses right -> push left, make left the new apex
				if (Vector::Length(left - apex) > EPS_PUSH) {
					path.PushWaypoint(left);
				}
				apex = left;
				apexIndex = leftIndex;
				// reset funnel
				left = apex;
				right = apex;
				leftIndex = apexIndex;
				rightIndex = apexIndex;
				i = (size_t)apexIndex + 1;
				continue;
			}
		}

		++i;
	}

	if (iter >= maxIter) {
		// Safety: bail out to avoid an infinite loop
		return false;
	}

	// Add goal and reverse for NavigationPath::PopWaypoint semantics
	path.PushWaypoint(to);
	path.Reverse();
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