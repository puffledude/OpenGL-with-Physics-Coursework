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

				int f = INT16_MAX;
				int g = INT16_MAX;
				int h = INT16_MAX;
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
				float setF(int updated) {
					f = updated;
				}
				float setG(int updated) {
					g = updated;
				}
				NavTri* setParent(NavTri* updated) {
					parent = updated;
				}

			};

			const NavTri* GetTriForPosition(const Vector3& pos) const;
			NavTri* GetTriForPosition(const Vector3& pos);

			std::vector<NavTri>		allTris;
			std::vector<Vector3>	allVerts;
		};
	}
}