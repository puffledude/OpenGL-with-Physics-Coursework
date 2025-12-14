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

	void OnCollisionEnd(NCL::CSC8503::GameObject* otherObject) override;

	Vector3 GetCheckPoint() const {
		return checkPointPos;
	}
	void SetCheckPoint(const Vector3& pos) {
		checkPointPos = pos;
	}
	void ResetPos() {
		this->GetTransform().SetPosition(checkPointPos);
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

	bool IsOutOfBounds() const {
		return outOfBounds;
	}
	bool InFreeCam() {
		return freeCamMode;
	}
	void SetFreeCam() {
		freeCamMode = !freeCamMode;
	}

protected:
	Vector3 checkPointPos = Vector3(0,0,0);
	float groundCheckTime = 6.0f;
	float jumpCooldown = 0.0f;
	bool inAir = false;
	bool freeCamMode = false;
	bool outOfBounds = false;
	bool canJump = true;
	GameObject* heldItem = nullptr;
	NCL::CSC8503::PositionConstraint* itemConstraint = nullptr;
	NCL::CSC8503::GameWorld* world = nullptr;
};