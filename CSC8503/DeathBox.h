#pragma once
#include "GameObject.h"
#include <string>

namespace NCL {
	namespace CSC8503 {
		class GameWorld; // forward declaration to avoid circular include
	}
}


namespace NCL {
	namespace CSC8503 {
		class DeathBox : public GameObject {
		public:
			DeathBox()
				: GameObject("DeathBox") {
			}


			~DeathBox(){}

			void OnCollisionBegin(GameObject* otherObject) override;

		};
	}
}