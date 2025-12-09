#include "DeathBox.h"
#include "GameWorld.h"
#include "PlayerObject.h"
using namespace NCL::CSC8503;

void DeathBox::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName() == "PlayerObject") {
		PlayerObject* player = static_cast<PlayerObject*>(otherObject);
		player->ResetPos();
	}
}