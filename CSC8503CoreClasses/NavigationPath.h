#pragma once
#include <vector>

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		class NavigationPath		{
		public:
			NavigationPath() {}
			~NavigationPath() {}

			void	Clear() {
				waypoints.clear();
			}
			void	PushWaypoint(const Vector3& wp) {
				waypoints.emplace_back(wp);
			}
			bool	PopWaypoint(Vector3& waypoint) {
				std::cout << "Is waypoints empty? " << waypoints.empty() << std::endl;
				if (waypoints.empty()) {
					return false;
				}
				waypoint = waypoints.back();
				waypoints.pop_back();
				return true;
			}

			void Reverse() {
				std::reverse(waypoints.begin(), waypoints.end());
			}

		protected:

			std::vector <Vector3> waypoints;
		};
	}
}

