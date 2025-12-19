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
		//Clamp to max distance and kill any accumulated momentum/forces
		Vector3 dir = Vector::Normalise(currentPos - intialPosition);
		this->GetTransform().SetPosition(intialPosition + dir * punchDistance);
		if (this->GetPhysicsObject()) {
			this->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0)); //Kill momentum
			this->GetPhysicsObject()->ClearForces();
		}

		punchDirection = -punchDirection;
		flipped = true;
	}
	else if (distanceFromInitial <= 0.1f && flipped) {
		//Return to initial position and kill momentum/forces
		this->GetTransform().SetPosition(intialPosition);
		if (this->GetPhysicsObject()) {
			this->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0)); //Kill momentum
			this->GetPhysicsObject()->ClearForces();
		}
		punchDirection = -punchDirection;
		flipped = false;
	}

	//Apply movement force for this frame
	if (this->GetPhysicsObject()) {
		this->GetPhysicsObject()->AddForce(punchDirection * punchForce * dt);
	}
}

void PunchBox::OnCollisionEnd(NCL::CSC8503::GameObject* otherObject) {
	//Reset position on collision end
	this->GetTransform().SetPosition(intialPosition);
	this->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
	this->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
	this->GetPhysicsObject()->ClearForces();
}