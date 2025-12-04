#include "PlayerObject.h"


void PlayerObject::Update(float dt) {
	if (jumpCooldown > 0.0f) {
		jumpCooldown -= dt;
		if (jumpCooldown < 0.0f) {
			jumpCooldown = 0.0f;
		}
	}
}