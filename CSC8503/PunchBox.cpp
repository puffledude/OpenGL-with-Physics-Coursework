#include "PunchBox.h"
#include "PhysicsObject.h"

void PunchBox::Update(float dt) {
	/*Punches in direction with force each frame.
	* When at max distance, invert direction and reverse.
	* Maybe kill momentum when changing direction?
	*/

	Vector3 currentPos = this->GetTransform().GetPosition();
	float distanceFromInitial = Vector::Length(currentPos - intialPosition);
	if (distanceFromInitial >= punchDistance && !flipped) {
		this->physicsObject->SetLinearVelocity(Vector3(0, 0, 0)); //Kill momentum

		punchDirection = -punchDirection;
		flipped = true;
	}
	else if (distanceFromInitial <= 0.1f && flipped) {
		this->physicsObject->SetLinearVelocity(Vector3(0, 0, 0)); //Kill momentum
		punchDirection = -punchDirection;
		flipped = false;
	}
	this->physicsObject->AddForce(punchDirection * punchForce);
}

void PunchBox::OnCollisionEnd(NCL::CSC8503::GameObject* otherObject) {
	//Reset position on collision end
	this->GetTransform().SetPosition(intialPosition);
	this->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
	this->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
	this->GetPhysicsObject()->ClearForces();
}