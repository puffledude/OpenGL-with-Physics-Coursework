#pragma once
#include "GameObject.h"
#include "Camera.h"

using namespace NCL::Maths;

class PlayerObject : public NCL::CSC8503::GameObject
{
public:
	PlayerObject() : GameObject("PlayerObject")
	{
	}

	~PlayerObject()
	{
	}

	void Update(float dt) override;

	void OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) override {
		canJump = true;
	}

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
};