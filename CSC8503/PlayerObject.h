#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "./PositionConstraint.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameWorld; // forward declaration to avoid circular include
	}
}

class PlayerObject : public NCL::CSC8503::GameObject
{
public:
	PlayerObject(NCL::CSC8503::GameWorld* world) : GameObject("PlayerObject"),
		world(world)
	{
	}

	~PlayerObject()
	{
	}

	void Update(float dt) override;

	void OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) override;


	float GetJumpCooldown() const {
		return jumpCooldown;
	}
	void SetJumpCooldown(float cooldown) {
		jumpCooldown = cooldown;
		canJump = false;
	}
	bool CanJump() const {
		return canJump&&(jumpCooldown<=0.0f);
	}

protected:
	float jumpCooldown = 0.0f;
	bool canJump = true;
	GameObject* heldItem = nullptr;
	NCL::CSC8503::PositionConstraint* itemConstraint = nullptr;
	NCL::CSC8503::GameWorld* world = nullptr;
};