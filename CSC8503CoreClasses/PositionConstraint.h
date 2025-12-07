#pragma once
#include "Constraint.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class PositionConstraint : public Constraint	{
		public:
			PositionConstraint(GameObject* a, GameObject* b, float d);
			~PositionConstraint() = default;

			void UpdateConstraint(float dt) override;
			float GetDistance() const { return distance; }
		protected:
			GameObject* objectA;
			GameObject* objectB;

			float distance;
		};
	}
}