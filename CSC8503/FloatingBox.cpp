#include "FloatingBox.h"
#include"PhysicsObject.h"

//FloatingBox::FloatingBox(GameObject* object) {
//	this->box = object;
//	this->physicsObject = box->GetPhysicsObject();
//
//}

FloatingBox::~FloatingBox() {
}

void FloatingBox::Update(float dt) {
	if (collided) {
		//this->GetPhysicsObject()->SetInverseMass(1.0f);
		this->GetPhysicsObject()->SetIgnoreGravity(false);
		//NCL::Vector3 boxPos = box->GetTransform().GetPosition();
		//boxPos.y -= dt*0.5; // Fall down when collided
		//box->GetTransform().SetPosition(boxPos);
	}
}

void FloatingBox::OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) {
	collided = true;
}