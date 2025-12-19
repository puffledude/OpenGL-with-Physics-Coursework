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

	//Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	//Matrix4 camWorld = Matrix::Inverse(view);
	//Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!


	//Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	//fwdAxis.y = 0.0f;
	//fwdAxis = Vector::Normalise(fwdAxis);
	//Vector3 lookDir = fwdAxis;
	//fwdAxis *= 15.0f;
	//rightAxis *= 15.0f;
	////bool moved = false;
	//if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
	//	Vector3 playerPos = this->GetTransform().GetPosition();
	//	Matrix4 temp = Matrix::View(playerPos + lookDir, playerPos, Vector3(0, 1, 0));  //Make a view matrix looking in the move direction
	//	Matrix4 modelMat = Matrix::Inverse(temp);  //Make the model matrix in model space
	//	Quaternion q(modelMat);   //Extract the rotation
	//	this->GetTransform().SetOrientation(q.Normalised());
	//	/*if (Vector::Length(this->GetPhysicsObject()->GetLinearVelocity()) == 0) {
	//		this->GetPhysicsObject()->ApplyLinearImpulse(fwdAxis);
	//	}*/
	//}
	//	this->GetPhysicsObject()->AddForce(fwdAxis);
	//	moved = true;
	//}

	//if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
	//	this->GetPhysicsObject()->AddForce(-fwdAxis);
	//	moved = true;
	//}

	//if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
	//	this->GetPhysicsObject()->AddForce(-rightAxis);
	//	moved = true;
	//}
	//if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
	//	this->GetPhysicsObject()->AddForce(rightAxis);
	//	moved = true;
	//}
	if (!moved && !inAir) {
				//Dampen player movement when no keys are pressed
		Vector3 vel = this->GetPhysicsObject()->GetLinearVelocity();
		vel.x *= (1-5*dt);
		vel.z *= (1-5*dt);
		this->GetPhysicsObject()->SetLinearVelocity(vel);
	}

	//if (Window::GetKeyboard()->KeyDown(KeyCodes::E)) {
	//			//Drop held item
	//	if (heldItem) {
	//		world->RemoveConstraint(itemConstraint);
	//		delete itemConstraint;
	//		itemConstraint = nullptr;
	//		heldItem->GetTransform().SetPosition(this->GetTransform().GetPosition() + Vector3(0, 0, 5));
	//		heldItem = nullptr;
	//	}
	//}

	//if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE) && this->CanJump()) {
	//	float inverseMass = this->GetPhysicsObject()->GetInverseMass();
	//	this->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, 25, 0));
	//	this->SetJumpCooldown(0.1f);
	//}



	Vector3 down = Vector3(0, -1, 0);
	Ray rayDown = Ray(this->GetTransform().GetPosition(), down);
	RayCollision rayCollision;
	if (world->Raycast(rayDown, rayCollision, true, this)) {
		if (rayCollision.rayDistance < 1) {
			canJump=true;
			
			inAir = false;
		}
		else {
			inAir = true;
		}
		if (rayCollision.rayDistance > 40.0f) {
			//Just in case of weird physics glitches
			groundCheckTime -= dt;
			//outOfBounds = true;
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

void PlayerObject::CreateButtonStates(char(&buttonStates)[8]) {
	memset(buttonStates, 0, 8);
	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		buttonStates[0] = 1;
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		buttonStates[1] = 1;
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		buttonStates[2] = 1;
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		buttonStates[3] = 1;
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::E)) {
		buttonStates[4] = 1;
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE)) {
		buttonStates[5] = 1;
	}

}

void PlayerObject::SyncCamera(NCL::PerspectiveCamera cam) {
	//Sync player to camera orientation
	float yaw = cam.GetYaw();
	Quaternion q = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), yaw+180);

	this->GetTransform().SetOrientation(q.Normalised());
}

void PlayerObject::ApplyButtonStates(const char(&buttonStates)[8], float dt) {

	moved = false;
	float speedMultiplier = 2500.0f;
	Quaternion currentOr = this->GetTransform().GetOrientation();
	Vector3 forward = Vector::Normalise(currentOr * Vector3(0, 0, 1));
	Vector3 right = Vector::Normalise(currentOr * Vector3(1, 0, 0));
	if (buttonStates[0]) {  //W key
		if (Vector::Length(this->GetPhysicsObject()->GetLinearVelocity()) == 0) {
			this->GetPhysicsObject()->ApplyLinearImpulse(forward);
		}
		this->GetPhysicsObject()->AddForce(forward * speedMultiplier * dt);
		moved = true;

	}
	if (buttonStates[1]) { //A key
		this->GetPhysicsObject()->AddForce(right * speedMultiplier * dt);
		moved = true;

	}
	if (buttonStates[2]) { //S key
		this->GetPhysicsObject()->AddForce(-forward * speedMultiplier * dt);
		moved = true;
	}
	if (buttonStates[3]) {//D key
		this->GetPhysicsObject()->AddForce(-right * speedMultiplier * dt);
		moved = true;
	}
	if (buttonStates[4]) {
		//Drop held item
		if (heldItem) {
			world->RemoveConstraint(itemConstraint);
			delete itemConstraint;
			itemConstraint = nullptr;
			heldItem->GetTransform().SetPosition(this->GetTransform().GetPosition() + Vector3(0, 0, 5));
			heldItem = nullptr;
		}
	}
	if (buttonStates[5] && this->CanJump()) {
		float inverseMass = this->GetPhysicsObject()->GetInverseMass();
		this->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, 40, 0));
		this->SetJumpCooldown(0.1f);
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
