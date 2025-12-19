#include "FloatingBox.h"
#include"PhysicsObject.h"

//FloatingBox::FloatingBox(GameObject* object) {
//	this->box = object;
//	this->physicsObject = box->GetPhysicsObject();
//
//}

FloatingBox::~FloatingBox() {
}

void FloatingBox::SetResetPos() {
	initialPosition = this->GetTransform().GetPosition();
}

void FloatingBox::Update(float dt) {
	if (collided) {
		//this->GetPhysicsObject()->SetInverseMass(1.0f);
		
		resetTimer -= dt;
		if (resetTimer < 5.0f) {
			//this->GetPhysicsObject()->SetIgnoreGravity(false);
			this->GetPhysicsObject()->SetInverseMass(actualMass);
		}
		//NCL::Vector3 boxPos = box->GetTransform().GetPosition();
		//boxPos.y -= dt*0.5; // Fall down when collided
		//box->GetTransform().SetPosition(boxPos);
	}
	if (resetTimer <= 0.0f && collided) {
		// Reset position
		this->GetTransform().SetPosition(initialPosition);
		this->GetPhysicsObject()->SetLinearVelocity(NCL::Maths::Vector3(0, 0, 0));
		this->GetPhysicsObject()->SetAngularVelocity(NCL::Maths::Vector3(0, 0, 0));
		this->GetPhysicsObject()->SetInverseMass(0);
		//this->GetPhysicsObject()->SetIgnoreGravity(true);
		collided = false;
		resetTimer = 0.0f;
	}
}

void FloatingBox::OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) {
	if (otherObject->GetName() == "PlayerObject" && collided ==false) {
		resetTimer = 6.0f;

		collided = true;
	}
	
}