#include "PlayerObject.h"


void PlayerObject::Update(float dt) {
	if (jumpCooldown > 0.0f) {
		jumpCooldown -= dt;
		if (jumpCooldown < 0.0f) {
			jumpCooldown = 0.0f;
		}
	}
	//Hold item above the player
	if (heldItem) {
		heldItem->GetTransform().SetPosition(this->GetTransform().GetPosition() + Vector3(0, 5, 0));
	}
}