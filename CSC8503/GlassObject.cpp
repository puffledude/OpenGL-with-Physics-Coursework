//#include "GlassObject.h"
//#include "PhysicsObject.h"
//
//void GlassObject::OnCollisionBegin(NCL::CSC8503::GameObject* otherObject)
//{
//	//Might be redundant. May be better in the collision resolution phase
//	if (!otherObject->GetPhysicsObject()) {
//		return;
//	}
//	if (otherObject->GetName() == "PlayerObject") {
//		return;
//	}
//	Vector3 collision = otherObject->GetPhysicsObject()->GetForce();
//	float collisionForce = Vector::Length(collision);
//	if (collisionForce > forceResistance) {
//		delete(this);
//	}
//}