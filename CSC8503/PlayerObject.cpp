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

void PlayerObject::OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) {
	//Allow jumping again when we hit the ground
	canJump = true;
	//If we hit an item, pick it up
	if ((otherObject->GetName() == "Item" || otherObject->GetName() == "Glass") && heldItem == nullptr) {
		heldItem = otherObject;
	}
}