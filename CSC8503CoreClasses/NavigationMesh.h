#pragma once
#include "NavigationMap.h"
#include "Plane.h"
#include <string>
#include <vector>
namespace NCL {
	namespace CSC8503 {
		class NavigationMesh : public NavigationMap	{
		public:
			NavigationMesh();
			NavigationMesh(const std::string&filename);
			~NavigationMesh();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
		
		protected:
			struct NavTri {
				Plane   triPlane;
				Vector3 centroid;
				float	area;
				NavTri* neighbours[3];

				float f = FLT_MAX;
				float g = FLT_MAX;
				float h = FLT_MAX;
				NavTri* parent = nullptr;

				int indices[3];

				NavTri() {
					area = 0.0f;
					neighbours[0] = nullptr;
					neighbours[1] = nullptr;
					neighbours[2] = nullptr;

					indices[0] = -1;
					indices[1] = -1;
					indices[2] = -1;
				}

			};

			bool AStarAlgorithm(const Vector3& from, const Vector3& to, std::vector<NavTri*>& outTris);

			bool sharedEdge(const NavTri* a, const NavTri* b, int& outA, int& outB);


			void ResetNavTris() {
				for (NavTri& t : allTris) {
					t.f = FLT_MAX;
					t.g = FLT_MAX;
					t.h = FLT_MAX;
					t.parent = nullptr;
				}
			}

			bool SmoothPath(const Vector3& from, const Vector3& to, std::vector<NavTri*>& outTris, NavigationPath& path);

			bool SharedEdge(const NavTri* a, const NavTri* b, int& outA, int& outB) const;

			const NavTri* GetTriForPosition(const Vector3& pos) const;
			NavTri* GetTriForPosition(const Vector3& pos);

			float Herustic(const Vector3& a, const Vector3& b) {
				Vector3 distance = b - a;
				return Vector::Length(distance);
			}
			std::vector<NavTri>		allTris;
			std::vector<Vector3>	allVerts;
		};
	}
}