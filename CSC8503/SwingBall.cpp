#include "SwingBall.h"
#include "PhysicsObject.h"

//using namespace NCL;
//using namespace CSC8503;

//Better thought. Apply the push every frame and when heeight = anchor height, reverse direction
void SwingBall::Update(float dt) {
    NCL::Vector3 ballPos = ball->GetTransform().GetPosition();
    NCL::Vector3 anchorPos = anchor->GetTransform().GetPosition();

    // Rope direction
    NCL::Vector3 ropeDir = Vector::Normalise(ballPos - anchorPos);

    // Tangent in XY plane (this locks the motion to 2D!)
    NCL::Vector3 tangent = Vector::Normalise(NCL::Vector3(-ropeDir.y, ropeDir.x, 0));

    // Apply force only along the swing direction
    ball->GetPhysicsObject()->AddForce(tangent * pushForce * direction);

    // Optional: auto-reverse at peak of swing
    float speed = Vector::Length(ball->GetPhysicsObject()->GetLinearVelocity());
    if (speed < 0.05f && !flipped) {
        direction *= -1.0f;
		flipped = true;
    }
    else {
		flipped = false;
    }
	
}