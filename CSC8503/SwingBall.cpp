#include "SwingBall.h"
#include "PhysicsObject.h"

//using namespace NCL;
//using namespace CSC8503;

//Better thought. Apply the push every frame and when heeight = anchor height, reverse direction
void SwingBall::Update(float dt) {
	NCL::Vector3 ballPos = ball->GetTransform().GetPosition();
	NCL::Vector3 anchorPos = anchor->GetTransform().GetPosition();
	if (ballPos.y >= anchorPos.y) {
		direction = -direction;
	}
	ball->GetPhysicsObject()->AddForce(direction * pushForce);	
	
}