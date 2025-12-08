#include "PunchBox.h"
#include "PhysicsObject.h"

void PunchBox::Update(float dt) {
	/*Punches in direction with force each frame.
	* When at max distance, invert direction and reverse.
	* Maybe kill momentum when changing direction?
	*/

	Vector3 currentPos = this->GetTransform().GetPosition();
	float distanceFromInitial = Vector::Length(currentPos - intialPosition);
	if (distanceFromInitial >= punchDistance) {
		this->physicsObject->SetLinearVelocity(Vector3(0, 0, 0)); //Kill momentum
		punchDirection = -punchDirection;
	}
	this->physicsObject->AddForce(punchDirection * punchForce);
}