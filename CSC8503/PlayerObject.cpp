#include "PlayerObject.h"
#include "Vector.h"
#include "PhysicsObject.h"
#include "GameWorld.h"
#include "Window.h"

using namespace NCL;
using namespace CSC8503;

void PlayerObject::Update(float dt) {
	if (freeCamMode)
	{
		return;
	}

	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = Matrix::Inverse(view);
	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!


	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);
	Vector3 lookDir = fwdAxis;
	fwdAxis *= 10.0f;
	rightAxis *= 10.0f;

	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		Vector3 playerPos = this->GetTransform().GetPosition();
		Matrix4 temp = Matrix::View(playerPos + lookDir, playerPos, Vector3(0, 1, 0));  //Make a view matrix looking in the move direction
		Matrix4 modelMat = Matrix::Inverse(temp);  //Make the model matrix in model space
		Quaternion q(modelMat);   //Extract the rotation
		this->GetTransform().SetOrientation(q.Normalised());
		this->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		this->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		this->GetPhysicsObject()->AddForce(-rightAxis);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		this->GetPhysicsObject()->AddForce(rightAxis);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::E)) {
				//Drop held item
		if (heldItem) {
			world->RemoveConstraint(itemConstraint);
			delete itemConstraint;
			itemConstraint = nullptr;
			heldItem->GetTransform().SetPosition(this->GetTransform().GetPosition() + Vector3(0, 0, 5));
			heldItem = nullptr;
		}
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE) && this->CanJump()) {
		float inverseMass = this->GetPhysicsObject()->GetInverseMass();
		this->GetPhysicsObject()->AddForce(Vector3(0, 1750, 0));
		this->SetJumpCooldown(0.2f);
	}
	//Player wil probably be launched if groundCheckTime runs out
	/*if (inAir) {
		groundCheckTime -= dt;
		if (groundCheckTime < 0.0f) {
			outOfBounds = true;
		}
	}*/


	Vector3 down = Vector3(0, -1, 0);
	Ray rayDown = Ray(this->GetTransform().GetPosition(), down);
	RayCollision rayCollision;
	if (world->Raycast(rayDown, rayCollision, true, this)) {
		if (rayCollision.rayDistance < this->GetTransform().GetScale().y) {
			canJump=true;
			
			inAir = false;
		}
		if (rayCollision.rayDistance > 40.0f) {
			//Just in case of weird physics glitches
			outOfBounds = true;
		}
	}

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
	//canJump = true;
	//inAir = false;
	//If we hit an item, pick it up
	if ((otherObject->GetName() == "Item" || otherObject->GetName() == "Glass") && heldItem == nullptr) {
		heldItem = otherObject;
		itemConstraint = new PositionConstraint(this, heldItem, 5.0f);
		world->AddConstraint(itemConstraint);
	}
}

void PlayerObject::OnCollisionEnd(NCL::CSC8503::GameObject* otherObject) {
	inAir = true;
	groundCheckTime = 6.0f;
}
